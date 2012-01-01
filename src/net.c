/*
   ispolin
   Copyright [c] 2011 tm512 (Kyle Davis), All Rights Reserved.

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

#include "prints.h"
#include "net.h"

#define MAXTRIES 5 // Max tries for send/recv

static struct timeval conn_timeout = { 10, 0 };

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
			if (errno != EWOULDBLOCK && errno != EINPROGRESS)
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
// Returns the number of bytes that failed to send, or -1 with invalid args
int net_send (int sock, char *buf, unsigned int len)
{
	int totalSent = 0, tempSent = 0, sendTries = 0;

	if (sock < 0 || !buf)
		return -1;

	while (totalSent < len)
	{
		if ((tempSent = send (sock, buf + totalSent, len - totalSent, 0)) <= 0)
		{
			if (sendTries++ < MAXTRIES)
			{
				dprint ("send failed (attempt %i, returned %i)", sendTries, tempSent);
				continue;
			}
			else
			{
				dprint ("send failed too many times, aborting! :O");
				break;
			}
		}
		totalSent += tempSent;
	}

	return len - totalSent;
}

// Attempts to receive len bytes from sock to buf
// Returns the amount of data read, or -1 with invalid args
int net_recv (int sock, char *buf, unsigned int len)
{
	int totalRecv = 0, tempRecv = 0, recvTries = 0;

	if (sock < 0 || !buf)
		return -1;

	while (totalRecv < len)
	{
		if ((tempRecv = recv (sock, buf + totalRecv, len - totalRecv, 0)) <= 0)
		{
			if (recvTries++ < MAXTRIES)
			{
				dprint ("recv failed (attempt %i, returned %i)", recvTries, tempRecv);
				continue;
			}
			else
			{
				dprint ("recv failed too many times, aborting! :O");
				break;
			}
		}
		totalRecv += tempRecv;
	}

	return totalRecv;
}

// Wrapper for close()
int net_close (int sock)
{
	return sock >= 0 ? close (sock) : -1;
}
