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

#define MAXCLIENTS 16

typedef struct chanlist_s
{
	char *name;
	char *pass;
	struct chanlist_s *next;
} chanlist_t;

typedef struct
{
	int s; // Socket
	char run;

	char *host;
	char *port;
	char *nick;
	char *owner;
	char *ns_nick;
	char *ns_command;

	char *rbuf; // Receive buffer, for irc_getln
	chanlist_t *channels;
} ircclient_t;

int irc_init (ircclient_t *cl);
void irc_destroy (ircclient_t **clp);
void irc_service (ircclient_t **clients);
int irc_login (ircclient_t *cl, char *nick);
int irc_join (ircclient_t *cl, char *chan, char *pw);
int irc_part (ircclient_t *cl, char *chan, char *msg);
int irc_privmsg (ircclient_t *cl, char *target, char *message, ...);
int irc_notice (ircclient_t *cl, char *target, char *message, ...);
int irc_isowner (ircclient_t *cl, char *host);
int irc_quit (ircclient_t *cl, char *msg);
void irc_parse (ircclient_t *cl, char *buf);
int irc_getln (ircclient_t *cl, char *buf);
int irc_sendln (ircclient_t *cl, char *fmt, ...);

extern int numclients;
extern ircclient_t *clients [MAXCLIENTS];

#endif // IRC_H__
