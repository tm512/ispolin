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
#include <string.h>
#include <time.h>
#ifdef linux
#include <alloca.h>
#endif // linux

#include "prints.h"
#include "irc.h"
#include "irchandler.h"
#include "module.h"

void topic_get_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char *tokbuf = alloca (strlen (args));
	strtok_r (args, " ", &tokbuf); // get rid of our name first
	char *channel = strtok_r (NULL, " ", &tokbuf);
	char *topic = channel + strlen (channel) + 2;

	ircprint (cl, "[%s] topic is: %s", channel, topic);

	return;
}

void topic_info_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char timefmt [25];
	char *tokbuf = alloca (strlen (args));
	strtok_r (args, " ", &tokbuf); // get rid of our name first
	char *channel = strtok_r (NULL, " ", &tokbuf);
	char *setter = strtok_r (NULL, " ", &tokbuf);

	// convert set time into time_t struct
	time_t st = (time_t) atol (strtok_r (NULL, " ", &tokbuf));
	strftime ((char*)timefmt, 25, "%a %b %d %T %Y", localtime (&st));

	ircprint (cl, "[%s] topic set by %s on %s.", channel, setter, timefmt);

	return;
}

void motd_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char *message = strstr (args, ":") + 1;

	if (message)
		ircprint (cl, "\033[1m%s\033[0m", message);

	return;
}

void endmotd_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	chanlist_t *it;
	for (it = cl->channels; it; it = it->next)
		if (strlen (it->name) && strlen (it->pass))
			irc_join (cl, it->name, it->pass);
		else if (strlen (it->name))
			irc_join (cl, it->name, NULL);

	return;
}

void join_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char *channel;

	if ((channel = strstr (args, ":")) == NULL)
		channel = args;
	else
		channel += 1; // need to remove the ":"

	ircprint (cl, "[%s] %s (%s) joins.", channel, nick, host);

	listener_t *l;
	for (l = joinListeners; l; l = l->next)
		((joinlistener_f) l->func) (cl, nick, host, channel);

	return;
}

void kick_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char *tokbuf = alloca (strlen (args));
	char *channel = strtok_r (args, " ", &tokbuf);
	char *victim = strtok_r (NULL, " ", &tokbuf);
	char *reason = victim + strlen (victim) + 2;

	ircprint (cl, "[%s] %s was kicked by %s [%s].", channel, victim, nick, reason);

	return;
}

void mode_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char *tokbuf = alloca (strlen (args));
	char *channel = strtok_r (args, " ", &tokbuf);
	char *mode = channel + strlen (channel) + 1;

	ircprint (cl, "[%s] %s sets mode [%s].", channel, nick, mode);

	return;
}

void notice_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char *tokbuf = alloca (strlen (args));
	char *source = strtok_r (args, " ", &tokbuf);
	char *message = source + strlen (source) + 2;

	if (source [0] != '#') // not from a channel, switch source to nick
		source = nick;

	ircprint (cl, "[%s] (%s) - \033[1m%s\033[0m", source, nick, message);

	return;
}

void part_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char *tokbuf = alloca (strlen (args));
	char *channel = strtok_r (args, " ", &tokbuf);
	char *reason = channel + strlen (channel) + 2;

	ircprint (cl, "[%s] %s (%s) parts [%s].", channel, nick, host, reason);

	listener_t *l;
	for (l = partListeners; l; l = l->next)
		((partlistener_f) l->func) (cl, nick, host, channel, reason);

	return;
}

void privmsg_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char *tokbuf = alloca (strlen (args));
	char *source = strtok_r (args, " ", &tokbuf);
	char *message = source + strlen (source) + 2;

	if (source [0] != '#') // not from a channel, switch source to nick
		source = nick;

	ircprint (cl, "[%s] <%s> %s", source, nick, message);

	listener_t *l;
	for (l = privmsgListeners; l; l = l->next)
		((privmsglistener_f) l->func) (cl, nick, host, source, message);

	return;
}

void quit_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char *reason = strstr (args, ":") + 1;

	ircprint (cl, "%s (%s) quits [%s].", nick, host, reason ? reason : "");

	listener_t *l;
	for (l = quitListeners; l; l = l->next)
		((quitlistener_f) l->func) (cl, nick, host, reason);

	return;
}

void topic_set_handler (ircclient_t *cl, char *nick, char *host, char *args)
{
	char *tokbuf = alloca (strlen (args));
	char *channel = strtok_r (args, " ", &tokbuf);
	char *topic = channel + strlen (channel) + 2;

	ircprint (cl, "[%s] %s sets topic to: %s", channel, nick, topic);

	return;
}
