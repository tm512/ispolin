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
#include <alloca.h>

#include "prints.h"
#include "net.h"
#include "irc.h"
#include "irchandler.h"

#define MAXBUF 512 // 512B of buffer for irc_getln / irc_sendln

// Initializes an ircclient_t, then starts its loop
void irc_init (ircclient_t *cl, const char *host, const char *port)
{
	int running = 1;
	int conn_attempts = 0;
	char buf [MAXBUF] = { 0 };

	cl->s = -1;
	cl->host = host;
	cl->port = port;

	cl->rbuf = (char *) malloc (MAXBUF);

	if (!cl->rbuf)
		eprint (1, "malloc failed for cl->buf, aborting!");

	memset (cl->rbuf, 0, MAXBUF);

	while (running && conn_attempts <= 5)
	{
		net_close (cl->s);

		if ((cl->s = net_connect (cl->host, cl->port)) < 0)
		{
			conn_attempts ++;
			continue;
		}

		// Login:
		if (irc_login (cl, "ispolin") != 0)
		{
			conn_attempts ++;
			continue;
		}

		// Success, reset connection attempt count
		conn_attempts = 0;

		while (irc_getln (cl, buf) >= 0)
		{
			printf ("%s", buf);
			irc_parse (cl, buf, &running);
			usleep (10000);
		}

		eprint (0, "Connection to %s:%s lost.", cl->host, cl->port);
		conn_attempts ++;
	}

	return;
}

// Sends NICK and USER
int irc_login (ircclient_t *cl, char *nick)
{
	return irc_sendln (cl, "NICK %s", nick) || irc_sendln (cl, "USER %s 8 * :ispolin", nick);
}

// Sends JOIN
int irc_join (ircclient_t *cl, char *chan)
{
	return irc_sendln (cl, "JOIN %s", chan);
}

// Sends PRIVMSG
int irc_privmsg (ircclient_t *cl, char *target, char *message, ...)
{
	char buf [MAXBUF] = { 0 };
	va_list va;

	va_start (va, message);
	vsnprintf (buf, MAXBUF, message, va);
	va_end (va);

	return irc_sendln (cl, buf);
}

// Parses a line of text from IRC
void irc_parse (ircclient_t *cl, char *buf, int *running)
{
	int i;
	char *tokbuf = alloca (strlen (buf)); // For strtok_r
	char *targ = NULL, *nick = NULL, *host = NULL, *cmd = NULL, *args = NULL;

	// Strip newlines and any whitespace at the end of the buffer:
	while (strlen (buf) > 0 && isspace (buf [strlen (buf) - 1]))
		buf [strlen (buf) - 1] = '\0';

	if (buf [0] != ':') // PING
	{
		if (!strncmp (buf, "PING", 4))
		{
			dprint ("<< PONG");
			irc_sendln (cl, "PONG %s", &buf [5]);
		}

		return;
	}

	buf ++; // remove the leading ':'

	targ = strtok_r (buf, " ", &tokbuf);

	// Is this a server message, or a message from a user?
	if (!strstr (targ, "!"))
		nick = targ; // This is a server message (host remains NULL)
	else
	{
		char *targbuf = alloca (strlen (targ));
		nick = strtok_r (targ, "!", &targbuf);
		host = strtok_r (NULL, "!", &targbuf);
	}

	cmd = strtok_r (NULL, " ", &tokbuf);
	args = cmd + strlen (cmd) + 1;

	// Find the function for this command (there might not be one)
	for (i = 0; i < (sizeof (irchandlers) / sizeof (irchandlers [0])); i++)
	{
		int cmp = strncmp (cmd, irchandlers [i].command, strlen (irchandlers [i].command));

		if (!cmp)
			irchandlers [i].func (nick, host, args);
		else if (cmp < 0)
			break;
	}

	return;
}	

// fills up buf with a whole line, if possible
// returns length of the line on success, 0 or -1 on failure
int irc_getln (ircclient_t *cl, char *buf)
{
	int ret = 0;
	char *pos = NULL;
	char tmpbuf [MAXBUF] = { 0 };

	// Clear the buffer we're sent
	memset (buf, 0, MAXBUF);

	while ((pos = strstr (cl->rbuf, "\r\n")) == NULL)
	{
		memset (tmpbuf, 0, MAXBUF);

		ret = net_recv (cl->s, tmpbuf, MAXBUF - strlen (cl->rbuf));

		if (ret <= 0)
			return ret;

		// Copy tmpbuf into the recvbuf
		memcpy (cl->rbuf + strlen (cl->rbuf), tmpbuf, MAXBUF - strlen (cl->rbuf));
	}

	// Copy line to buf, add \0
	memcpy (buf, cl->rbuf, pos + 2 - cl->rbuf);
	strstr (buf, "\r\n") [2] = '\0';

	// shift the recvbuf back
	memset (tmpbuf, 0, MAXBUF);
	memcpy (tmpbuf, pos + 2, MAXBUF - (pos + 2 - cl->rbuf));
	memcpy (cl->rbuf, tmpbuf, MAXBUF);

	return strlen (buf);
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
