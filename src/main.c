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

static void kbd_callback(const char *name, int name_len, const char *instruction,
	int instruction_len, int num_prompts, const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
	LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses, void **abstract)
{
	const char *password = *abstract;

	if (num_prompts == 1)
	{
		responses[0].text   = strdup(password);
		responses[0].length = strlen(password);
	}
}

int sshterm(int argc, char **argv)
{
	LONG args[NUM_ARGS];
	struct RDArgs *rda = NULL;
	const char *hostname;
	const char *username;
	const char *password;
	LONG sb_size = 2000;
	struct Screen *screen = NULL;
	struct TermWindow *termwin = NULL;
	int sock = -1;
	const struct hostent *hostent;
	struct in_addr hostaddr;
	int port;
	struct sockaddr_in sin;
	LIBSSH2_SESSION *session = NULL;
	int rc;
	char homedir[1024];
	const char *userauthlist;
	unsigned int auth_pw;
	LIBSSH2_AGENT *agent = NULL;
	LIBSSH2_CHANNEL *channel = NULL;
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
	password = (const char *)args[ARG_PASSWORD];

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

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
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

	if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) != 0)
	{
		fprintf(stderr, "Failed to connect - %s (%d)\n", hstrerror(h_errno), h_errno);
		goto out;
	}

	session = libssh2_session_init_ex(NULL, NULL, NULL, (void *)password);
	if (session == NULL)
	{
		fprintf(stderr, "Failed to create SSH session\n");
		goto out;
	}

	rc = libssh2_session_handshake(session, sock);
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

	userauthlist = libssh2_userauth_list(session, username, strlen(username));
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

	if (auth_pw == 0)
	{
		fprintf(stderr, "No supported authentication methods\n");
		goto out;
	}

	if (auth_pw & AUTH_PASSWORD)
	{
		if (password == NULL)
		{
			password = request_password(AUTH_PASSWORD, username, hostname);
			if (password == NULL)
				goto out;
		}

		rc = libssh2_userauth_password(session, username, password);
	}
	else if (auth_pw & AUTH_KEYBOARD_INTERACTIVE)
	{
		if (password == NULL)
		{
			password = request_password(AUTH_KEYBOARD_INTERACTIVE, username, hostname);
			if (password == NULL)
				goto out;
		}

		rc = libssh2_userauth_keyboard_interactive(session, username, &kbd_callback);
	}
	else
	{
		BOOL authenticated = FALSE;

		if (!args[ARG_NOSSHAGENT])
		{
			agent = libssh2_agent_init(session);
			if (agent == NULL)
			{
				fprintf(stderr, "Failed to init ssh-agent support\n");
			}
			else
			{
				rc = libssh2_agent_connect(agent);
				if (rc < 0)
				{
					fprintf(stderr, "Failed to connect to ssh-agent\n");
				}
				else
				{
					struct libssh2_agent_publickey *identity;
					struct libssh2_agent_publickey *prev_identity = NULL;

					if (libssh2_agent_list_identities(agent))
					{
						fprintf(stderr, "Failure requesting identities from ssh-agent\n");
						goto out;
					}
					while (TRUE)
					{
						rc = libssh2_agent_get_identity(agent, &identity, prev_identity);
						if (rc == 1)
							break;
						if (rc < 0)
						{
							fprintf(stderr, "Failure obtaining identity from ssh-agent\n");
							goto out;
						}

						if (libssh2_agent_userauth(agent, username, identity))
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

			if (agent != NULL)
			{
				libssh2_agent_disconnect(agent);
				libssh2_agent_free(agent);
				agent = NULL;
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

			if (password == NULL)
			{
				password = request_password(AUTH_PUBLICKEY, keyfile2);
				if (password == NULL)
					goto out;
			}

			rc = libssh2_userauth_publickey_fromfile(session, username, keyfile1, keyfile2, password);
		}
	}
	if (rc < 0)
	{
		fprintf(stderr, "Authentication failed\n");
		goto out;
	}

	channel = libssh2_channel_open_session(session);
	if (channel == NULL)
	{
		fprintf(stderr, "Unable to open a session\n");
		goto out;
	}

	termwin_get_size(termwin, &columns, &rows);

	if (libssh2_channel_request_pty_ex(channel, "xterm-256color", 14, NULL, 0, columns, rows, 0, 0))
	{
		fprintf(stderr, "Failed requesting pty\n");
		goto out;
	}

	if (libssh2_channel_shell(channel))
	{
		fprintf(stderr, "Unable to request shell on allocated pty\n");
		goto out;
	}

	libssh2_session_set_blocking(session, 0);

	done = FALSE;

	while (!done)
	{
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);

		FD_SET(sock, &rfds);

		if (termwin_poll_new_size(termwin) || termwin_poll(termwin))
		{
			FD_SET(sock, &wfds);
		}

		signals = termwin_get_signals(termwin);

		#ifdef __CLIB2__
		rc = waitselect(sock + 1, &rfds, &wfds, NULL, NULL, &signals);
		#else
		rc = waitselect(sock + 1, &rfds, &wfds, NULL, NULL, (unsigned int *)&signals);
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

		if (FD_ISSET(sock, &rfds))
		{
			char buffer[256];
			ssize_t n;

			do {
				n = libssh2_channel_read(channel, buffer, sizeof(buffer));

				if (n > 0)
				{
					termwin_write(termwin, buffer, n);
				}
				else if (n < 0 && n != LIBSSH2_ERROR_EAGAIN)
				{
					IExec->DebugPrintF("libssh2_channel_read: %d\n", n);
					goto out;
				}
			} while (n > 0);
		}

		if (FD_ISSET(sock, &wfds))
		{
			if (termwin_poll_new_size(termwin))
			{
				termwin_get_size(termwin, &columns, &rows);

				rc = libssh2_channel_request_pty_size(channel, columns, rows);
				if (rc < 0)
				{
					IExec->DebugPrintF("libssh2_channel_request_pty_size: %d\n", rc);
					goto out;
				}
			}

			if (termwin_poll(termwin))
			{
				char buffer[256], *p;
				ssize_t size, n;

				do {
					size = termwin_read(termwin, buffer, sizeof(buffer));

					p = buffer;
					while (size > 0)
					{
						n = libssh2_channel_write(channel, p, size);
						if (n > 0)
						{
							p += n;
							size -= n;
						}
						else if (n < 0 && n != LIBSSH2_ERROR_EAGAIN)
						{
							IExec->DebugPrintF("libssh2_channel_write: %d\n", n);
							goto out;
						}
					}
				} while (size > 0);
			}
		}

		if (libssh2_channel_eof(channel))
		{
			done = TRUE;
		}
	}

	if (AboutWindowPID != 0)
		aboutwin_close();

	retval = RETURN_OK;

out:
	if (channel != NULL)
	{
		libssh2_channel_close(channel);
		libssh2_channel_wait_closed(channel);
		libssh2_channel_free(channel);
		channel = NULL;
	}

	if (agent != NULL)
	{
		libssh2_agent_disconnect(agent);
		libssh2_agent_free(agent);
		agent = NULL;
	}

	if (session != NULL)
	{
		libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
		libssh2_session_free(session);
		session = NULL;
	}

	if (sock != -1)
	{
		close(sock);
		sock = -1;
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

