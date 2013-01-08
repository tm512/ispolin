/*
   ispolin
   Copyright [c] 2011-2013 tm512 (Kyle Davis), All Rights Reserved.

   Ispolin is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 3, as
   published by the Free Software Foundation.

   Ispolin is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Ispolin.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/time.h>
#ifndef linux
#include <sys/select.h>
#endif // linux

#include "prints.h"
#include "net.h"

struct timeval conn_timeout = { 10, 0 };

int net_recvd = 0, net_sent = 0;
int highsock = -1;

fd_set *fds = NULL;

int net_connect (const char *host, const char *port)
{
	int sock;
	struct addrinfo hints, *res, *r;
	fd_set sel; // fdset for select(), since we need a timeout for connect()...

	iprint ("Connecting to %s:%s", host, port);

	memset (&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo (host, port, &hints, &res) != 0)
	{
		eprint (0, "Couldn't resolve %s:%s", host, port);
		return -1;
	}

	// Loop until we get a good connection or run out of options
	for (r = res; r; r = r->ai_next)
	{
		if ((sock = socket (r->ai_family, r->ai_socktype, r->ai_protocol)) < 0)
			continue;

		FD_ZERO (&sel);
		FD_SET (sock, &sel);
		fcntl (sock, F_SETFL, O_NONBLOCK);

		if (connect (sock, r->ai_addr, r->ai_addrlen) < 0)
			if (errno != EWOULDBLOCK && errno != EINPROGRESS && errno != EAGAIN)
				continue; // An error is to be expected, but it's not always an issue

		// Wait for server response, check if we're connected or not
		if (select (sock + 1, NULL, &sel, NULL, &conn_timeout) == 1)
		{
			int serror = 0;
			socklen_t slen = sizeof (int);

			getsockopt (sock, SOL_SOCKET, SO_ERROR, &serror, &slen);

			if (!serror)
				break; // Success!
		}

		close (sock);
	}

	freeaddrinfo (res);

	if (!r) // We never got a good connection from that loop
	{
		eprint (0, "Could not connect to %s:%s", host, port);
		return -1;
	}

	return sock;
}

// Attempts to send len bytes from buf to sock
// Returns the number of bytes that failed to send, or -1 with invalid args or failure to send
int net_send (int sock, char *buf, unsigned int len)
{
	int totalSent = 0, tempSent = 0, sendTries = 0;

	if (sock < 0 || !buf)
		return -1;

	while (totalSent < len)
	{
		if ((tempSent = send (sock, buf + totalSent, len - totalSent, 0)) < 0)
		{
			if (errno != EWOULDBLOCK && errno != EINPROGRESS && errno != EAGAIN)
				return -1;
		}
		else
		{
			totalSent += tempSent;
			net_sent += tempSent;
		}
	}

	return len - totalSent;
}

// Attempts to receive len bytes from sock to buf
// Returns the amount of data read, or -1 with invalid args or failure to receive
int net_recv (int sock, char *buf, unsigned int len)
{
	int totalRecv = 0, tempRecv = 0, recvTries = 0;

	if (sock < 0 || !buf)
		return -1;

	while (totalRecv < len)
	{
		if ((tempRecv = recv (sock, buf + totalRecv, len - totalRecv, 0)) <= 0)
		{
			if ((errno != EWOULDBLOCK && errno != EAGAIN) || !tempRecv)
				return -1;
			else
				break;
		}
		else
		{
			totalRecv += tempRecv;
			net_recvd += tempRecv;
		}
	}

	return totalRecv;
}

// Wrapper for close()
int net_close (int sock)
{
	if (sock >= 0)
	{
		FD_CLR (sock, fds);
		return close (sock);
	}
	else
		return -1;
}

// Add a socket to the reading fdset
void net_addsock (int sock)
{
	if (!fds)
	{
		fds = malloc (sizeof (fd_set));
		FD_ZERO (fds);
	}

	highsock = (sock > highsock) ? sock : highsock;
	FD_SET (sock, fds);
	return;
}

// See if the socket is set in the fdset (wrapper for FD_ISSET)
// returns -1 if the socket is set but has no pending data to read (disconnected)
int net_isset (int sock)
{
	if (FD_ISSET (sock, fds))
		return 1;
	else
		return 0;
}

// wrapper for select ()
int net_select (void)
{
	struct timeval sel_timeout = { 1, 0 };
	int ret = select (highsock + 1, fds, NULL, NULL, &sel_timeout);

	highsock = -1; // reset for next loop
	return ret;
}
