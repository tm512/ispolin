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
#include <string.h>
#include <alloca.h>

#include "prints.h"
#include "irc.h"
#include "irchandler.h"
#include "module.h"

void endmotd_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	irc_join (cl, "#bottest", NULL);

	return;
}

void join_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char *channel;

	if ((channel = strstr (args, ":")) == NULL)
		channel = args;
	else
		channel += 1; // need to remove the ":"

	ircprint ("[%s] %s (%s) joins.", channel, nick, host);

	return;
}

void notice_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char *tokbuf = alloca (strlen (args));
	char *source = strtok_r (args, " ", &tokbuf);
	char *message = source + strlen (source) + 2;

	if (source [0] != '#') // not from a channel, switch source to nick
		source = nick;

	ircprint ("[%s] (%s) - \033[1m%s\033[0m", source, nick, message);

	return;
}

void part_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char *tokbuf = alloca (strlen (args));
	char *channel = strtok_r (args, " ", &tokbuf);
	char *reason = channel + strlen (channel) + 2;

	ircprint ("[%s] %s (%s) parts [%s].", channel, nick, host, reason);

	return;
}

void privmsg_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char *tokbuf = alloca (strlen (args));
	char *source = strtok_r (args, " ", &tokbuf);
	char *message = source + strlen (source) + 2;

	if (source [0] != '#') // not from a channel, switch source to nick
		source = nick;

	ircprint ("[%s] <%s> %s", source, nick, message);

	listener_t *l;
	for (l = &privmsgListeners; l; l = l->next)
		((privmsglistener_f) l->func) (cl, nick, host, source, message);

	return;
}
