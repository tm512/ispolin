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

#ifndef IRC_H__
#define IRC_H__

typedef struct
{
	int s; // Socket

	const char *host;
	const char *port;

	char *rbuf; // Receive buffer, for irc_getln
} ircclient_t;

void irc_init (ircclient_t *cl, const char *host, const char *port);
int irc_login (ircclient_t *cl, char *nick);
int irc_join (ircclient_t *cl, char *chan, char *pw);
int irc_privmsg (ircclient_t *cl, char *target, char *message, ...);
void irc_parse (ircclient_t *cl, char *buf, int *running);
int irc_getln (ircclient_t *cl, char *buf);
int irc_sendln (ircclient_t *cl, char *fmt, ...);

#endif // IRC_H__
