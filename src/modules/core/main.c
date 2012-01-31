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
#ifdef linux
#include <alloca.h>
#else
#include <stdlib.h>
#endif
#include <sys/utsname.h>

#include "version.h"

#include "irc.h"
#include "config.h"
#include "module.h"

void die (void);

// TODO - Move this somewhere else (maybe a sort of API file?)
char is_owner (ircclient_t *cl, char *host)
{
	if (strlen (host) != strlen (cl->owner))
		return 0;
	else
		return !strncmp (host, cl->owner, strlen (host));
}

void corePrivmsg (ircclient_t *cl, char *nick, char *host, char *source, char *message)
{
	char *tokbuf = alloca (strlen (message));
	char *buf = alloca (strlen (message) + 1);

	// check for prefix:
	if (message [0] != globalcfg.prefix)
		return;

	// back up message:
	strncpy (buf, message, strlen (message) + 1);

	buf ++;

	if (strstr (buf, "info") == buf)
	{
		struct utsname sysinfo;
		uname (&sysinfo);

		irc_privmsg (cl, source, "Hi, I am an ispolin bot version " ISP_VERSION GIT_VERSION " running on %s %s.",
		             sysinfo.sysname, sysinfo.release);
		return;
	}

	if ((strstr (buf, "join ") == buf || strstr (buf, "j ") == buf) && is_owner (cl, host))
	{
		strtok_r (buf, " ", &tokbuf);
		char *channel = strtok_r (NULL, " ", &tokbuf);
		char *password = strtok_r (NULL, " ", &tokbuf);

		irc_join (cl, channel, password);
		return;
	}

	if ((strstr (buf, "part ") == buf || strstr (buf, "p ") == buf) && is_owner (cl, host))
	{
		strtok_r (buf, " ", &tokbuf);
		char *channel = strtok_r (NULL, " ", &tokbuf);

		irc_part (cl, channel, "ispolin"); // todo: customizable
		return;
	}

	if (strstr (buf, "quit") == buf && is_owner (cl, host))
	{
		die ();
		return;
	}

	if (strstr (buf, "uptime") == buf)
	{
		FILE *uptime_pipe = popen ("uptime", "r");
		char *uptime_out = alloca (96);

		if (uptime_pipe && uptime_out)
		{
			fgets (uptime_out, 96, uptime_pipe);
			irc_privmsg (cl, source, uptime_out);
		}

		return;
	}

	return;
}

void init (listener_t *privmsg)
{
	privmsg->func = corePrivmsg;
	privmsg->next = NULL;

	return;
}
