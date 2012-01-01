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
#include <stdarg.h>
#include <string.h>

#include "prints.h"
#include "net.h"
#include "irc.h"

#define RECVBUF 512 // 512B of buffer for irc_getline
#define VERSIONTEST "0.0.1"

// Initializes an ircclient_t, then starts its loop
void irc_init (ircclient_t *cl, const char *host, const char *port)
{
	int running = 1;
	int conn_attempts = 0;

	cl->s = -1;
	cl->host = host;
	cl->port = port;

	cl->buf = (char *) malloc (RECVBUF);

	if (!cl->buf)
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

		irc_sendln (cl, "NICK ispolin");
		irc_sendln (cl, "USER ispolin 8 *: ispolin %s", VERSIONTEST);
		sleep (3);
		irc_sendln (cl, "JOIN #test");
		for (;;);
	}

	return;
}

int irc_sendln (ircclient_t *cl, char *fmt, ...)
{
	char buf [512]; // Oh no, magic numbers!
	va_list va;

	va_start (va, fmt);
	vsnprintf (buf, 509, fmt, va);
	va_end (va);

	int len = strlen (buf);

	buf [len] = '\r';
	buf [len + 1] = '\n';
	buf [len + 2] = '\0';

	dprint ("Sending: %s", buf);
	return net_send (cl->s, buf, strlen (buf));
}
