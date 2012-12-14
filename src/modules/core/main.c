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
#endif
#include <stdlib.h>
#include <sys/utsname.h>

#include "version.h"

#include "prints.h"
#include "irc.h"
#include "module.h"

char modname [] = "core";

void die (char *msg);

extern int net_recvd, net_sent;
extern char *modpath;

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
	if (message [0] != cl->prefix && message [0] != '\001')
		return;

	// back up message:
	strncpy (buf, message, strlen (message) + 1);

	if (buf [0] == '\001')
		ctcpHandler (cl, nick, host, source, buf);

	buf ++;

	if (strstr (buf, "bw") == buf)
	{
		irc_privmsg (cl, source, "Since I was last started, I have sent a total of %.3fKB and received a total of %.3fKB.",
		             (float) net_sent / 1024, (float) net_recvd / 1024);
		return;
	}

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
		int i;
		for (i = 0; i < MAXCHANS; i++)
			if (!cl->channels [i].name)
				break;

		if (i == MAXCHANS)
		{
			irc_privmsg (cl, source, "Channel limit already reached.");
			return;
		}

		strtok_r (buf, " ", &tokbuf);
		char *tmpchan = strtok_r (NULL, " ", &tokbuf);
		char *tmppw = strtok_r (NULL, " ", &tokbuf);
		cl->channels [i].name = malloc (strlen (tmpchan) + 1);
		strcpy (cl->channels [i].name, tmpchan);

		if (tmppw)
		{
			cl->channels [i].pass = malloc (strlen (tmppw) + 1);
			strcpy (cl->channels [i].pass, tmppw);
		}

		irc_join (cl, &cl->channels [i]);
		return;
	}

	if ((strstr (buf, "module ") == buf || strstr (buf, "mod ") == buf) && irc_isowner (cl, host))
	{
		strtok_r (buf, " ", &tokbuf);
		char *modcmd = strtok_r (NULL, " ", &tokbuf);
		char *modname = modcmd + strlen (modcmd) + 1;;

		if (!strlen (modname))
		{
			irc_notice (cl, nick, "No module name specified!");
			return;
		}

		if (strstr (modcmd, "load") == modcmd)
		{
			char filepath [strlen (modpath) + strlen (modname) + 4];
			sprintf (filepath, "%s/%s.so", modpath, modname);

			if (!module_load (filepath))
				irc_notice (cl, nick, "Loaded module: %s", filepath);
			else
				irc_notice (cl, nick, "Failed to load module: %s", filepath);
		}
		else if (strstr (modcmd, "unload") == modcmd)
		{
			if (strcmp (modname, "core") && !module_unload (modname))
				irc_notice (cl, nick, "Unloaded module: %s", modname);
			else
				irc_notice (cl, nick, "Failed to unload module: %s", modname);
		}

		return;
	}

	if ((strstr (buf, "part ") == buf || strstr (buf, "p ") == buf) && irc_isowner (cl, host))
	{
		strtok_r (buf, " ", &tokbuf);
		char *channame = strtok_r (NULL, " ", &tokbuf);
		int i;

		for (i = 0; i < MAXCHANS; i++)
			if (cl->channels [i].name && !strcmp (channame, cl->channels [i].name))
				break;

		if (i == MAXCHANS)
		{
			irc_privmsg (cl, source, "Not joined to %s", channame);
			return;
		}

		irc_part (cl, &cl->channels [i], "ispolin"); // todo: customizable
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

		pclose (uptime_pipe);

		return;
	}

	return;
}

void init (void)
{
	module_registerfunc (&privmsgListeners, corePrivmsg, modname);
	return;
}
