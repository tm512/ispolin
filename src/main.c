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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "version.h"

#include "prints.h"
#include "net.h"
#include "irc.h"
#include "module.h"
#include "config.h"
#include "luapi.h"

ircclient_t *clients [MAXCLIENTS] = { 0 };
char *configpath = NULL;
char *localdir = NULL;
config_t globalcfg;

#define arg(longform,shortform) if (!strcmp (argv [i], longform) || !strcmp (argv [i], shortform))

void parseargs (int argc, char **argv)
{
	int i;
	for (i = 1; i < argc; i++)
	{
		arg ("--config", "-c")
		{
			if (i + 1 < argc && argv [i + 1] [0] != '-')
				configpath = argv [i + 1];
			continue;
		}

		arg ("--daemon", "-d")
		{
			pid_t pid;
			FILE *pidfile = NULL;

			if (i + 1 >= argc || argv [i + 1] [0] == '-') // use default pidfile
				pidfile = fopen ("./ispolin.pid", "w");
			else
				pidfile = fopen (argv [i + 1], "w");

			if (!pidfile)
			{
				eprint (1, "Could not open pidfile.");
			}

			if (!(pid = fork ()))
			{
				freopen ("/dev/null", "w", stdout);
				freopen ("/dev/null", "w", stderr);
				fclose (pidfile);
			}
			else
			{
				iprint ("Forked to background (pid: %i)", (int) pid);
				fprintf (pidfile, "%i\n", (int) pid);
				fclose (pidfile);
				exit (0);
			}
			continue;
		}

		arg ("--localdir", "-l")
		{
			if (i + 1 < argc && argv [i + 1] [0] != '-')
				localdir = argv [i + 1];
			continue;
		}
	}

	return;
}

#undef arg

// Kills off all connections
void die (char *msg)
{
	int i;
	for (i = 0; i < MAXCLIENTS; i++)
		if (clients [i])
			irc_quit (clients [i], msg);

	module_die ();

	return;
}

void sigdie (int sig)
{
	char msg [16];
	sprintf (msg, "signal %i", sig);

	die (msg);
}

int main (int argc, char **argv)
{
	int i;
	char *coremodule;

	fprintf (stdout, "[\033[1m-\033[0m] \033[1mispolin\033[0m\n");
	fprintf (stdout, "[\033[1m-\033[0m] \033[2mversion %s%s compiled on " __DATE__ "\033[0m\n", ISP_VERSION, GIT_VERSION);

	parseargs (argc, argv);

	luapi_init ();

	// Set up localdir path
	if (!localdir)
	{
		char *home = getenv ("HOME");
		localdir = malloc (10 + strlen (home));
		sprintf (localdir, "%s/.ispolin", home);
	}	

	// Create localdir if needed
	struct stat tmpstat;
	if (stat (localdir, &tmpstat))
	{
		if (!mkdir (localdir, S_IRUSR | S_IWUSR | S_IXUSR))
			iprint ("Created local data directory: %s", localdir);
		else
			eprint (1, "Could not create local data directory: %s", localdir);
	}

	// Set up config path
	if (!configpath)
	{
		configpath = malloc (12 + strlen (localdir));
		sprintf (configpath, "%s/config.lua", localdir);

		// if the config doesn't exist, open the default
		if (stat (configpath, &tmpstat))
		{
			free (configpath);
			configpath = "./config.lua";
		}
	}

	luapi_loadconfig (configpath);

	coremodule = (char*) malloc (strlen (globalcfg.modpath) + 8);
	sprintf (coremodule, "%score.so", globalcfg.modpath);
	module_load (coremodule);
	free (coremodule);

	signal (SIGINT, sigdie);
	signal (SIGTERM, sigdie);

	for (i = 0; i < MAXMODULES; i++)
		if (globalcfg.modlist [i])
			module_load (globalcfg.modlist [i]);

	for (i = 0; i < MAXCLIENTS; i++)
		if (clients [i])
			if (irc_init (clients [i]))
				irc_destroy (&clients [i]);

	// Main loop:
	irc_service (clients);
	return 0;
}
