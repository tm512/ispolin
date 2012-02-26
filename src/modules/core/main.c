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

#include "prints.h"
#include "irc.h"
#include "config.h"
#include "module.h"

#define modcheck(lst) \
{ \
	listener_t *it; \
	for (it = lst; it; it = it->next) \
		if (strstr (it->modname, modname) == it->modname) \
		{ \
			irc_notice (cl, nick, "Module already loaded!"); \
			return; \
		} \
}

const char modname [] = "core";

void die (char *msg);

void ctcpHandler (ircclient_t *cl, char *nick, char *host, char *source, char *message)
{
	message ++;

	if (strstr (message, "VERSION") == message && strstr (nick, source))
		irc_notice (cl, source, "\001VERSION ispolin " ISP_VERSION GIT_VERSION " compiled " __DATE__ "\001");

	return;
}

void corePrivmsg (ircclient_t *cl, char *nick, char *host, char *source, char *message)
{
	char *tokbuf = alloca (strlen (message));
	char *buf = alloca (strlen (message) + 1);

	// check for prefix:
	if (message [0] != globalcfg.prefix && message [0] != '\001')
		return;

	// back up message:
	strncpy (buf, message, strlen (message) + 1);

	if (buf [0] == '\001')
		ctcpHandler (cl, nick, host, source, buf);

	buf ++;

	if (strstr (buf, "info") == buf)
	{
		struct utsname sysinfo;
		uname (&sysinfo);

		irc_privmsg (cl, source, "Hi, I am an ispolin bot version " ISP_VERSION GIT_VERSION " running on %s %s.",
		             sysinfo.sysname, sysinfo.release);
		return;
	}

	if ((strstr (buf, "join ") == buf || strstr (buf, "j ") == buf) && irc_isowner (cl, host))
	{
		strtok_r (buf, " ", &tokbuf);
		char *channel = strtok_r (NULL, " ", &tokbuf);
		char *password = strtok_r (NULL, " ", &tokbuf);

		irc_join (cl, channel, password);
		return;
	}

	if ((strstr (buf, "module ") == buf || strstr (buf, "mod ") == buf) && irc_isowner (cl, host))
	{
		strtok_r (buf, " ", &tokbuf);
		char *modcmd = strtok_r (NULL, " ", &tokbuf);
		char *modname = modcmd + strlen (modcmd) + 1;;

		dprint ("%s %s", modcmd, modname);

		if (!strlen (modname))
		{
			irc_notice (cl, nick, "No module name specified!");
			return;
		}

		if (strstr (modcmd, "load") == modcmd)
		{
			char filepath [strlen (globalcfg.modpath) + strlen (modname) + 4];
			sprintf (filepath, "%s/%s.so", globalcfg.modpath, modname);

			modcheck (privmsgListeners);
			modcheck (joinListeners);
			modcheck (partListeners);
			modcheck (quitListeners);

			if (!module_load (filepath))
				irc_notice (cl, nick, "Loaded module: %s", filepath);
			else
				irc_notice (cl, nick, "Failed to load module: %s", filepath);
		}
		else if (strstr (modcmd, "unload") == modcmd)
		{
			if (!module_unload (modname, &privmsgListeners))
				irc_notice (cl, nick, "Unloaded module: %s", modname);
			else
				irc_notice (cl, nick, "Failed to unload module: %s", modname);
		}

		return;
	}

	if ((strstr (buf, "part ") == buf || strstr (buf, "p ") == buf) && irc_isowner (cl, host))
	{
		strtok_r (buf, " ", &tokbuf);
		char *channel = strtok_r (NULL, " ", &tokbuf);

		irc_part (cl, channel, "ispolin"); // todo: customizable
		return;
	}

	if (strstr (buf, "quit") == buf && irc_isowner (cl, host))
	{
		die ("ispolin");
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

void init (void *mod)
{
	module_registerfunc (&privmsgListeners, corePrivmsg, mod, modname);
	return;
}
