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

#ifndef IRCHANDLER_H__
#define IRCHANDLER_H__

typedef void (*linehandler_f) (ircclient_t *cl, char *nick, char *host, char *args);

typedef struct
{
	const char *command;
	linehandler_f func;
} irchandler_t;

void topic_get_handler (ircclient_t *cl, char *nick, char *host, char *args);
void topic_info_handler (ircclient_t *cl, char *nick, char *host, char *args);
void motd_handler (ircclient_t *cl, char *nick, char *host, char *args);
void endmotd_handler (ircclient_t *cl, char *nick, char *host, char *args);
void join_handler (ircclient_t *cl, char *nick, char *host, char *args);
void kick_handler (ircclient_t *cl, char *nick, char *host, char *args);
void mode_handler (ircclient_t *cl, char *nick, char *host, char *args);
void nick_handler (ircclient_t *cl, char *nick, char *host, char *args);
void notice_handler (ircclient_t *cl, char *nick, char *host, char *args);
void part_handler (ircclient_t *cl, char *nick, char *host, char *args);
void privmsg_handler (ircclient_t *cl, char *nick, char *host, char *args);
void quit_handler (ircclient_t *cl, char *nick, char *host, char *args);
void topic_set_handler (ircclient_t *cl, char *nick, char *host, char *args);

static const irchandler_t irchandlers [] =
{
	{ "332", topic_get_handler },
	{ "333", topic_info_handler },
	{ "372", motd_handler },
	{ "376", endmotd_handler },
	{ "JOIN", join_handler },
	{ "KICK", kick_handler },
	{ "MODE", mode_handler },
	{ "NICK", nick_handler },
	{ "NOTICE", notice_handler },
	{ "PART", part_handler },
	{ "PRIVMSG", privmsg_handler },
	{ "QUIT", quit_handler },
	{ "TOPIC", topic_set_handler }
};

#endif // IRCHANDLER_H__
