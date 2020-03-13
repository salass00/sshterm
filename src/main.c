/*
 * SSHTerm - SSH2 shell client
 *
 * Copyright (C) 2019-2020 Fredrik Wikstrom <fredrik@a500.org>
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the SSHTerm
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sshterm.h"

#include <proto/intuition.h>
#include <classes/requester.h>

#include <unistd.h>

#include "SSHTerm_rev.h"

static const char template[] =
	"HOSTADDR/A,"
	"PORT/N/K,"
	"USER/A,"
	"PASSWORD,"
	"NOSSHAGENT/S,"
	"KEYFILE/K,"
	"MAXSB/N/K";

enum {
	ARG_HOSTADDR,
	ARG_PORT,
	ARG_USER,
	ARG_PASSWORD,
	ARG_NOSSHAGENT,
	ARG_KEYFILE,
	ARG_MAXSB,
	NUM_ARGS
};

#define AUTH_PASSWORD             (1 << 0)
#define AUTH_KEYBOARD_INTERACTIVE (1 << 1)
#define AUTH_PUBLICKEY            (1 << 2)

static char *request_password(unsigned int auth_pw, ...)
{
	struct ClassLibrary *RequesterBase;
	Class               *RequesterClass;
	char                *password = NULL;
	va_list              ap;

	va_start(ap, auth_pw);

	RequesterBase = IIntuition->OpenClass("requester.class", 53, &RequesterClass);
	if (RequesterBase != NULL)
	{
		struct Screen *screen;

		screen = IIntuition->LockPubScreen(NULL);
		if (screen != NULL)
		{
			TEXT    bodytext[256];
			TEXT    buffer[256];
			Object *reqobj;

			buffer[0] = '\0';

			if (auth_pw == AUTH_PUBLICKEY)
				vsnprintf(bodytext, sizeof(bodytext), "Enter passphrase for key file '%s'", ap);
			else
				vsnprintf(bodytext, sizeof(bodytext), "Enter password for %s@%s", ap);

			reqobj = IIntuition->NewObject(RequesterClass, NULL,
				REQ_Type,        REQTYPE_STRING,
				REQ_Image,       REQIMAGE_QUESTION,
				REQ_TitleText,   VERS,
				REQ_BodyText,    bodytext,
				REQ_GadgetText,  "_Ok|_Cancel",
				REQS_AllowEmpty, FALSE,
				REQS_Invisible,  TRUE,
				REQS_Buffer,     buffer,
				REQS_MaxChars,   sizeof(buffer),
				REQS_ReturnEnds, TRUE,
				TAG_END);

			if (reqobj != NULL)
			{
				struct orRequest reqmsg;
				LONG             result;

				reqmsg.MethodID  = RM_OPENREQ;
				reqmsg.or_Attrs  = NULL;
				reqmsg.or_Window = NULL;
				reqmsg.or_Screen = screen;

				result = IIntuition->IDoMethodA(reqobj, (Msg)&reqmsg);

				if (result && buffer[0] != '\0')
				{
					password = strdup(buffer);
				}

				IIntuition->DisposeObject(reqobj);
			}

			IIntuition->UnlockPubScreen(NULL, screen);
		}

		IIntuition->CloseClass(RequesterBase);
	}

	va_end(ap);

	return password;
}

struct ssh_session {
	int              socket;
	LIBSSH2_SESSION *session;
	LIBSSH2_CHANNEL *channel;
	char            *password;
    LIBSSH2_AGENT   *agent;
	const char      *keyfile;
	char             iobuf[32768];
};

static void kbd_callback(const char *name, int name_len, const char *instruction,
	int instruction_len, int num_prompts, const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
	LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses, void **abstract)
{
	struct ssh_session *ss = IExec->FindTask(NULL)->tc_UserData;

	if (num_prompts == 1)
	{
		responses[0].text   = strdup(ss->password);
		responses[0].length = strlen(ss->password);
	}
}

static int passphrase_callback(char *buf, int size, int rwflag, void *userdata)
{
	struct ssh_session *ss = IExec->FindTask(NULL)->tc_UserData;

	if (ss->password == NULL)
	{
		ss->password = request_password(AUTH_PUBLICKEY, ss->keyfile);
		if (ss->password == NULL)
		{
			if (size)
				*buf = '\0';
			return 0;
		}
	}

	strlcpy(buf, ss->password, size);

	return strlen(buf);
}

int sshterm(int argc, char **argv)
{
	LONG args[NUM_ARGS];
	struct RDArgs *rda = NULL;
	const char *hostname;
	const char *username;
	LONG sb_size = 2000;
	struct Screen *screen = NULL;
	struct TermWindow *termwin = NULL;
	struct ssh_session *ss = NULL;
	const struct hostent *hostent;
	struct in_addr hostaddr;
	int port;
	struct sockaddr_in sin;
	int rc;
	char homedir[1024];
	const char *userauthlist;
	unsigned int auth_pw;
	UWORD columns, rows;
	BOOL done;
	ULONG signals;
	fd_set rfds, wfds;
	int retval = RETURN_ERROR;

	memset(args, 0, sizeof(args));

	rda = IDOS->ReadArgs(template, args, NULL);
	if (rda == NULL)
	{
		IDOS->PrintFault(IDOS->IoErr(), NULL);
		goto out;
	}

	hostname = (const char *)args[ARG_HOSTADDR];
	username = (const char *)args[ARG_USER];

	screen = IIntuition->LockPubScreen(NULL);
	if (screen == NULL)
	{
		fprintf(stderr, "Failed to lock default public screen\n");
		goto out;
	}

	if (args[ARG_MAXSB])
	{
		sb_size = *(LONG *)args[ARG_MAXSB];
		if (sb_size < 0)
			sb_size = 0;
		else if (sb_size > 64000)
			sb_size = 64000;
	}

	termwin = termwin_open(screen, sb_size);
	if (termwin == NULL)
	{
		fprintf(stderr, "Failed to create terminal\n");
		goto out;
	}

	ss = malloc(sizeof(*ss));
	if (ss == NULL)
	{
		goto out;
	}

	memset(ss, 0, sizeof(*ss));
	ss->socket = -1;

	if (args[ARG_PASSWORD])
	{
		ss->password = strdup((const char *)args[ARG_PASSWORD]);
	}

	ss->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (ss->socket == -1)
	{
		fprintf(stderr, "Failed to create socket\n");
		goto out;
	}

	hostent = gethostbyname(hostname);
	if (hostent == NULL)
	{
		fprintf(stderr, "Failed to resolve host name\n");
		goto out;
	}

	memcpy(&hostaddr, hostent->h_addr_list[0], sizeof(struct in_addr));

	port = 22;

	if (args[ARG_PORT])
	{
		port = *(LONG *)args[ARG_PORT];
	}

	memset(&sin, 0, sizeof(sin));

	sin.sin_family = AF_INET;
	sin.sin_port   = htons(port);
	sin.sin_addr   = hostaddr;

	if (connect(ss->socket, (struct sockaddr *)&sin, sizeof(sin)) != 0)
	{
		fprintf(stderr, "Failed to connect - %s (%d)\n", hstrerror(h_errno), h_errno);
		goto out;
	}

	IExec->FindTask(NULL)->tc_UserData = ss;

	ss->session = libssh2_session_init();
	if (ss->session == NULL)
	{
		fprintf(stderr, "Failed to create SSH session\n");
		goto out;
	}

	rc = libssh2_session_handshake(ss->session, ss->socket);
	if (rc < 0)
	{
		fprintf(stderr, "Failed to establish SSH session - %d\n", rc);
		goto out;
	}

	if (IDOS->GetVar("HOME", homedir, sizeof(homedir), 0) <= 0)
	{
		strcpy(homedir, "HOME:");
	}

	auth_pw = 0;

	userauthlist = libssh2_userauth_list(ss->session, username, strlen(username));
	if (strstr(userauthlist, "password") != NULL)
	{
		auth_pw |= AUTH_PASSWORD;
	}
	if (strstr(userauthlist, "keyboard-interactive") != NULL)
	{
		auth_pw |= AUTH_KEYBOARD_INTERACTIVE;
	}
	if (strstr(userauthlist, "publickey") != NULL)
	{
		auth_pw |= AUTH_PUBLICKEY;
	}

	if (args[ARG_KEYFILE])
	{
		/* Force public key authentication if the KEYFILE argument is specified
		 * and this method is supported by the SSH server.
		 */
		if (auth_pw & AUTH_PUBLICKEY)
		{
			auth_pw = AUTH_PUBLICKEY;
		}
	}

	if (auth_pw == 0)
	{
		fprintf(stderr, "No supported authentication methods\n");
		goto out;
	}

	if (auth_pw & AUTH_PASSWORD)
	{
		if (ss->password == NULL)
		{
			ss->password = request_password(AUTH_PASSWORD, username, hostname);
			if (ss->password == NULL)
				goto out;
		}

		rc = libssh2_userauth_password(ss->session, username, ss->password);
		if (rc < 0)
		{
			fprintf(stderr, "Authentication by password failed\n");
			goto out;
		}
	}
	else if (auth_pw & AUTH_KEYBOARD_INTERACTIVE)
	{
		if (ss->password == NULL)
		{
			ss->password = request_password(AUTH_KEYBOARD_INTERACTIVE, username, hostname);
			if (ss->password == NULL)
				goto out;
		}

		rc = libssh2_userauth_keyboard_interactive(ss->session, username, &kbd_callback);
		if (rc < 0)
		{
			fprintf(stderr, "Authentication by keyboard-interactive failed\n");
			goto out;
		}
	}
	else
	{
		BOOL authenticated = FALSE;

		if (!args[ARG_NOSSHAGENT])
		{
			ss->agent = libssh2_agent_init(ss->session);
			if (ss->agent == NULL)
			{
				fprintf(stderr, "Failed to init ssh-agent support\n");
			}
			else
			{
				rc = libssh2_agent_connect(ss->agent);
				if (rc < 0)
				{
					fprintf(stderr, "Failed to connect to ssh-agent\n");
				}
				else
				{
					struct libssh2_agent_publickey *identity;
					struct libssh2_agent_publickey *prev_identity = NULL;

					if (libssh2_agent_list_identities(ss->agent))
					{
						fprintf(stderr, "Failure requesting identities from ssh-agent\n");
						goto out;
					}
					while (TRUE)
					{
						rc = libssh2_agent_get_identity(ss->agent, &identity, prev_identity);
						if (rc == 1)
							break;
						if (rc < 0)
						{
							fprintf(stderr, "Failure obtaining identity from ssh-agent\n");
							goto out;
						}

						if (libssh2_agent_userauth(ss->agent, username, identity))
						{
							fprintf(stderr, "Authentication with username %s and public key %s failed\n",
								username, identity->comment);
						}
						else
						{
							printf("Authentication with username %s and public key %s succeeded\n",
								username, identity->comment);
							authenticated = TRUE;
							break;
						}

						prev_identity = identity;
					}
				}
			}
		}

		if (!authenticated)
		{
			char keyfile1[1024];
			char keyfile2[1024];

			if (ss->agent != NULL)
			{
				libssh2_agent_disconnect(ss->agent);
				libssh2_agent_free(ss->agent);
				ss->agent = NULL;
			}

			if (args[ARG_KEYFILE])
			{
				strlcpy(keyfile1, (const char *)args[ARG_KEYFILE], sizeof(keyfile1));
				strlcat(keyfile1, ".pub", sizeof(keyfile1));
				strlcpy(keyfile2, (const char *)args[ARG_KEYFILE], sizeof(keyfile2));
			}
			else
			{
				strlcpy(keyfile1, homedir, sizeof(keyfile1));
				strlcpy(keyfile2, homedir, sizeof(keyfile2));

				IDOS->AddPart(keyfile1, ".ssh/id_rsa.pub", sizeof(keyfile1));
				IDOS->AddPart(keyfile2, ".ssh/id_rsa", sizeof(keyfile2));
			}

			ss->keyfile = keyfile2;
			libssh2_passphrase_callback_set(ss->session, &passphrase_callback);

			rc = libssh2_userauth_publickey_fromfile(ss->session, username, keyfile1, keyfile2, NULL/*ss->password*/);
			if (rc < 0)
			{
				fprintf(stderr, "Authentication by public key failed\n");
				goto out;
			}
		}
	}

	ss->channel = libssh2_channel_open_session(ss->session);
	if (ss->channel == NULL)
	{
		fprintf(stderr, "Unable to open a session\n");
		goto out;
	}

	termwin_get_size(termwin, &columns, &rows);

	if (libssh2_channel_request_pty_ex(ss->channel, "xterm-256color", 14, NULL, 0, columns, rows, 0, 0))
	{
		fprintf(stderr, "Failed requesting pty\n");
		goto out;
	}

	if (libssh2_channel_shell(ss->channel))
	{
		fprintf(stderr, "Unable to request shell on allocated pty\n");
		goto out;
	}

	libssh2_session_set_blocking(ss->session, 0);

	done = FALSE;

	while (!done)
	{
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);

		FD_SET(ss->socket, &rfds);

		if (termwin_poll_new_size(termwin) || termwin_poll(termwin))
		{
			FD_SET(ss->socket, &wfds);
		}

		signals = termwin_get_signals(termwin);

		#ifdef __CLIB2__
		rc = waitselect(ss->socket + 1, &rfds, &wfds, NULL, NULL, &signals);
		#else
		rc = waitselect(ss->socket + 1, &rfds, &wfds, NULL, NULL, (unsigned int *)&signals);
		#endif
		if (rc < 0)
		{
			if (errno != EINTR)
			{
				IExec->DebugPrintF("waitselect: %d\n", errno);
				goto out;
			}

			done = TRUE;
		}

		if (termwin_handle_input(termwin))
			done = TRUE;

		if (FD_ISSET(ss->socket, &rfds))
		{
			ssize_t rs;

			do
			{
				rs = libssh2_channel_read(ss->channel, ss->iobuf, sizeof(ss->iobuf));

				if (rs > 0)
				{
					termwin_write(termwin, ss->iobuf, rs);
				}
				else if (rs < 0 && rs != LIBSSH2_ERROR_EAGAIN)
				{
					IExec->DebugPrintF("libssh2_channel_read: %d\n", rs);
					goto out;
				}
			}
			while (rs > 0 && libssh2_poll_channel_read(ss->channel, 0));

			termwin_refresh(termwin);
		}

		if (FD_ISSET(ss->socket, &wfds))
		{
			if (termwin_poll_new_size(termwin))
			{
				termwin_get_size(termwin, &columns, &rows);

				rc = libssh2_channel_request_pty_size(ss->channel, columns, rows);
				if (rc < 0)
				{
					IExec->DebugPrintF("libssh2_channel_request_pty_size: %d\n", rc);
					goto out;
				}
			}

			if (termwin_poll(termwin))
			{
				char *buffer;
				ssize_t input_size, remain, ws;

				do
				{
					input_size = termwin_read(termwin, ss->iobuf, sizeof(ss->iobuf));

					buffer = ss->iobuf;
					remain = input_size;
					while (remain > 0)
					{
						ws = libssh2_channel_write(ss->channel, buffer, remain);
						if (ws > 0)
						{
							buffer += ws;
							remain -= ws;
						}
						else if (ws < 0 && ws != LIBSSH2_ERROR_EAGAIN)
						{
							IExec->DebugPrintF("libssh2_channel_write: %d\n", ws);
							goto out;
						}
					}
				}
				while (input_size > 0);
			}
		}

		if (libssh2_channel_eof(ss->channel))
		{
			done = TRUE;
		}
	}

	if (AboutWindowPID != 0)
		aboutwin_close();

	retval = RETURN_OK;

out:
	if (ss != NULL)
	{
		if (ss->channel != NULL)
		{
			libssh2_channel_close(ss->channel);
			libssh2_channel_wait_closed(ss->channel);
			libssh2_channel_free(ss->channel);
			ss->channel = NULL;
		}

		if (ss->agent != NULL)
		{
			libssh2_agent_disconnect(ss->agent);
			libssh2_agent_free(ss->agent);
			ss->agent = NULL;
		}

		if (ss->session != NULL)
		{
			libssh2_session_disconnect(ss->session, "Normal Shutdown, Thank you for playing");
			libssh2_session_free(ss->session);
			ss->session = NULL;
		}

		if (ss->password != NULL)
		{
			memset(ss->password, 0xff, strlen(ss->password) + 1);
			free(ss->password);
			ss->password = NULL;
		}

		if (ss->socket != -1)
		{
			close(ss->socket);
			ss->socket = -1;
		}

		free(ss);
		ss = NULL;
	}

	if (termwin != NULL)
	{
		termwin_close(termwin);
		termwin = NULL;
	}

	if (AboutWindowPID != 0)
	{
		while (find_pid(AboutWindowPID))
		{
			IExec->Wait(SIGF_CHILD);
		}
		AboutWindowPID = 0;
	}

	if (screen != NULL)
	{
		IIntuition->UnlockPubScreen(NULL, screen);
		screen = NULL;
	}

	if (rda != NULL)
	{
		IDOS->FreeArgs(rda);
		rda = NULL;
	}

	return retval;
}

