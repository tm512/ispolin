/*
   ispolin
   Copyright [c] 2011-2012 tm512 (Kyle Davis), All Rights Reserved.

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
#include <stdarg.h>
#include <string.h>

#include "prints.h"
#include "net.h"
#include "irc.h"

#define MAXBUF 512 // 512B of buffer for irc_getln / irc_sendln
#define VERSIONTEST "0.0.1"

// Initializes an ircclient_t, then starts its loop
void irc_init (ircclient_t *cl, const char *host, const char *port)
{
	int running = 1;
	int conn_attempts = 0;

	cl->s = -1;
	cl->host = host;
	cl->port = port;

	cl->rbuf = (char *) malloc (MAXBUF);

	if (!cl->rbuf)
		eprint (1, "malloc failed for cl->buf, aborting!");

	while (running && conn_attempts <= 5)
	{
		net_close (cl->s);

		if((cl->s = net_connect (cl->host, cl->port)) < 0)
		{
			conn_attempts ++;
			continue;
		}

		// Success, reset connection attempt count
		conn_attempts = 0;

		// Login:
		if (irc_login (cl, "ispolin") != 0)
		{
			conn_attempts ++;
			continue;
		}

		for (;;)
		{
			char buf [MAXBUF] = { 0 };
			irc_getln (cl, buf);
			usleep (10000);
		}
	}

	return;
}

int irc_login (ircclient_t *cl, char *nick)
{
	return irc_sendln (cl, "NICK %s", nick) || irc_sendln (cl, "USER %s 8 * :ispolin", nick);
}

int irc_getln (ircclient_t *cl, char *buf)
{
	char *pos = NULL;
	char tmpbuf [MAXBUF] = { 0 };

	while ((pos = strstr (cl->rbuf, "\r\n")) == NULL)
	{
		memset (tmpbuf, 0, MAXBUF);

		if (net_recv (cl->s, tmpbuf, MAXBUF - strlen (cl->rbuf)) <= 0) // failed to recieve anything
			return 1;

		// Copy tmpbuf into the recvbuf
		memcpy (cl->rbuf + strlen (cl->rbuf), tmpbuf, MAXBUF - strlen (cl->rbuf));
	}

	// Copy recvbuf into buf
	memcpy (buf, cl->rbuf, pos + 2 - cl->rbuf);

	// shift the recvbuf back
	memset (tmpbuf, 0, MAXBUF);
	memcpy (tmpbuf, pos + 2, MAXBUF - (pos + 2 - cl->rbuf));
	memcpy (cl->rbuf, tmpbuf, MAXBUF);

	return 0;
}

int irc_sendln (ircclient_t *cl, char *fmt, ...)
{
	char buf [MAXBUF + 3]; // + 3 for \r\n\0 :|
	va_list va;

	va_start (va, fmt);
	vsnprintf (buf, MAXBUF, fmt, va);
	va_end (va);

	int len = strlen (buf);

	buf [len] = '\r';
	buf [len + 1] = '\n';
	buf [len + 2] = '\0';

	return net_send (cl->s, buf, strlen (buf));
}
