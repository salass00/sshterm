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

#include <proto/bsdsocket.h>
#include <sys/filio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>

int socket(int domain, int type, int protocol)
{
	return ISocket->socket(domain, type, protocol);
}

int connect(int sock, const struct sockaddr *addr, socklen_t addrlen)
{
	return ISocket->connect(sock, (struct sockaddr *)addr, addrlen);
}

ssize_t send(int sock, const void *buf, size_t len, int flags)
{
	return ISocket->send(sock, (void *)buf, len, flags);
}

ssize_t recv(int sock, void *buf, size_t len, int flags)
{
	return ISocket->recv(sock, buf, len, flags);
}

int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen)
{
	return ISocket->setsockopt(sock, level, optname, (void *)optval, optlen);
}

int getsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen)
{
	return ISocket->getsockopt(sock, level, optname, optval, optlen);
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	struct timeval *timeout)
{
	return ISocket->WaitSelect(nfds, readfds, writefds, exceptfds, timeout, NULL);
}

#ifdef __CLIB2__
int waitselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	struct timeval *timeout, ULONG *signals)
#else
int waitselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	struct timeval *timeout, unsigned int *signals)
#endif
{
	return ISocket->WaitSelect(nfds, readfds, writefds, exceptfds, timeout, (ULONG *)signals);
}

in_addr_t inet_addr(const char *cp)
{
	return ISocket->inet_addr((STRPTR)cp);
}

struct hostent *gethostbyname(const char *name)
{
	return ISocket->gethostbyname((STRPTR)name);
}

int fcntl(int sock, int cmd, ...)
{
	va_list ap;
	int arg, nonblock;

	va_start(ap, cmd);

	switch (cmd)
	{
		case F_GETFL:
			return 0;

		case F_SETFL:
			arg = va_arg(ap, int);
			nonblock = (arg & O_NONBLOCK) ? 1 : 0;
			return ISocket->IoctlSocket(sock, FIONBIO, &nonblock);
	}

	va_end(ap);

	errno = ENOSYS;
	return -1;
}

int close(int sock)
{
	return ISocket->CloseSocket(sock);
}

