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
#ifdef linux
#include <alloca.h>
#endif // linux

#include "prints.h"
#include "net.h"
#include "irc.h"
#include "irchandler.h"

#undef DEBUG

#define MAXBUF 512 // 512B of buffer for irc_getln / irc_sendln
#define connsleep(time) \
	iprint ("Sleeping for %i seconds before reconnecting.", time); \
	sleep (time);
#define stripw(str) \
	while (strlen (str) > 0 && isspace (str [strlen (str) - 1])) str [strlen (str) - 1] = '\0';

int numclients = 0;

// Initializes an ircclient_t
// Returns 0 on success, nonzero on failure
int irc_init (ircclient_t *cl)
{
	int conn_attempts = 0;

	cl->s = -1;
	cl->run = 1;

	if (!cl->rbuf)
		cl->rbuf = malloc (MAXBUF);

	if (!cl->rbuf)
		eprint (1, "malloc failed for cl->buf, aborting!");

	memset (cl->rbuf, 0, MAXBUF);

	while (conn_attempts <= 5)
	{
		net_close (cl->s);

		if ((cl->s = net_connect (cl->host, cl->port)) < 0)
		{
			conn_attempts ++;
			connsleep (1 << conn_attempts);
			continue;
		}

		// Login:
		if (irc_login (cl) != 0)
		{
			conn_attempts ++;
			connsleep (1 << conn_attempts);
			continue;
		}

		numclients ++;
		return 0;
	}

	eprint (0, "Exhausted reconnection attempts to %s:%s, giving up", cl->host, cl->port);
	return 1;
}

// Deinitialize an ircclient_t, set the pointer back to NULL
void irc_destroy (ircclient_t **clp)
{
	ircclient_t *cl = *clp;

	// Free the client's resources
	free (cl->host);
	free (cl->port);
	free (cl->nick);
	free (cl->owner);
	free (cl->ns_nick);
	free (cl->ns_command);
	free (cl->rbuf);

	free (cl);
	*clp = NULL;

	return;
}

// Main loop. Service all of the IRC clients passed to us
void irc_service (ircclient_t **clients)
{
	int i;
	char buf [MAXBUF] = { 0 };

	while (1)
	{
		if (!numclients)
			return;

		for (i = 0; i < MAXCLIENTS; i++)
			if (clients [i])
				net_addsock (clients [i]->s);

		if (net_select () > 0)
		{
			for (i = 0; i < MAXCLIENTS; i++)
			{
				if (!clients [i])
					continue;

				if (net_isset (clients [i]->s))
				{
					int ret;
					while ((ret = irc_getln (clients [i], buf)) > 0)
					{
						#ifdef DEBUG
						printf ("%s", buf);
						#endif
						irc_parse (clients [i], buf);
					}

					if (ret < 0)
					{
						eprint (0, "Connection to %s:%s lost", clients [i]->host, clients [i]->port);
						numclients --;

						if (clients [i]->run) // unintentional disconnect, try reconnecting
							irc_init (clients [i]);
						else
							irc_destroy (&clients [i]);
					}
				}
			}
		}
	}
}

// Sends NICK and USER
int irc_login (ircclient_t *cl)
{
	return irc_sendln (cl, "NICK %s", cl->nick) || irc_sendln (cl, "USER %s 8 * :%s", cl->username, cl->realname);
}

// Sends JOIN (with optional password)
int irc_join (ircclient_t *cl, ircchannel_t *chan)
{
	if (chan->pass)
		return irc_sendln (cl, "JOIN %s %s", chan->name, chan->pass);
	else
		return irc_sendln (cl, "JOIN %s", chan->name);
}

int irc_part (ircclient_t *cl, ircchannel_t *chan, char *msg)
{
	int ret = irc_sendln (cl, "PART %s :%s", chan->name, msg);
	free (chan->name);
	free (chan->pass);
	chan->name = chan->pass = NULL;

	return ret;
}

// Sends QUIT
int irc_quit (ircclient_t *cl, char *msg)
{
	cl->run = 0;
	return irc_sendln (cl, "QUIT :%s", msg);
}

// Sends PRIVMSG
int irc_privmsg (ircclient_t *cl, char *target, char *message, ...)
{
	char buf [MAXBUF] = { 0 };
	va_list va;

	va_start (va, message);
	vsnprintf (buf, MAXBUF, message, va);
	va_end (va);

	stripw (buf); // get rid of any extra newlines

	ircprint (cl, "[%s] <%s> %s", target, cl->nick, buf);
	return irc_sendln (cl, "PRIVMSG %s :%s", target, buf);
}

// Sends a NOTICE
int irc_notice (ircclient_t *cl, char *target, char *message, ...)
{
	char buf [MAXBUF] = { 0 };
	va_list va;

	va_start (va, message);
	vsnprintf (buf, MAXBUF, message, va);
	va_end (va);

	stripw (buf); // get rid of any extra newlines

	ircprint (cl, "[%s] (%s) \033[1m%s\033[0m", target, cl->nick, buf);
	return irc_sendln (cl, "NOTICE %s :%s", target, buf);
}

// Checks if the specified host is the same as the owner of this ircclient_t
int irc_isowner (ircclient_t *cl, char *host)
{
	if (strlen (host) != strlen (cl->owner))
		return 0;
	else
		return !strncmp (host, cl->owner, strlen (host));
}

// Parses a line of text from IRC
void irc_parse (ircclient_t *cl, char *buf)
{
	int i;
	char *tokbuf = alloca (strlen (buf)); // For strtok_r
	char *targ = NULL, *nick = NULL, *host = NULL, *cmd = NULL, *args = NULL;

	// Strip newlines and any whitespace at the end of the buffer:
	stripw (buf);

	if (buf [0] != ':') // PING
	{
		if (!strncmp (buf, "PING", 4))
			irc_sendln (cl, "PONG %s", &buf [5]);

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
			irchandlers [i].func (cl, nick, host, args);
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
	char *pos = NULL, *tmpos = NULL;
	char tmpbuf [MAXBUF] = { 0 };

	// Clear the buffer we're sent
	memset (buf, 0, MAXBUF);

	while ((tmpos = strstr (cl->rbuf, "\r\n")) == NULL && ret < MAXBUF - strlen (cl->rbuf) - 1)
	{
		memset (tmpbuf, 0, MAXBUF);

		ret = net_recv (cl->s, tmpbuf, MAXBUF - strlen (cl->rbuf) - 1);

		if (ret <= 0)
			return ret;

		// Copy tmpbuf into the recvbuf
		memcpy (cl->rbuf + strlen (cl->rbuf), tmpbuf, MAXBUF - strlen (cl->rbuf));
	}

	// See if we need to truncate the line
	// If the entire buffer filled before we got an \r\n, add it ourselves
	if (tmpos)
		pos = tmpos;
	else
	{
		cl->rbuf [MAXBUF - 2] = '\n';
		cl->rbuf [MAXBUF - 3] = '\r';
		pos = cl->rbuf + MAXBUF - 3;
	}

	// Copy line to buf, add \0
	memcpy (buf, cl->rbuf, pos + 2 - cl->rbuf);
	strstr (buf, "\r\n") [2] = '\0';

	// shift the recvbuf back
	memset (tmpbuf, 0, MAXBUF);
	memcpy (tmpbuf, pos + 2, MAXBUF - (pos + 2 - cl->rbuf));
	memcpy (cl->rbuf, tmpbuf, MAXBUF);

	// Seems hacky... Recursively call irc_getln again if we didn't get a
	// complete line, and there is the rest of the line waiting to be read.
	if (!tmpos)
		irc_getln (cl, tmpbuf);

	return strlen (buf);
}

int irc_sendln (ircclient_t *cl, char *fmt, ...)
{
	int len;
	char buf [MAXBUF + 3]; // + 3 for \r\n\0 :|
	va_list va;

	va_start (va, fmt);
	vsnprintf (buf, MAXBUF, fmt, va);
	va_end (va);

	len = strlen (buf);

	buf [len] = '\r';
	buf [len + 1] = '\n';
	buf [len + 2] = '\0';

	return net_send (cl->s, buf, strlen (buf));
}
