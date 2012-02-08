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
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

#include "version.h"

#include "prints.h"
#include "net.h"
#include "irc.h"
#include "module.h"
#include "config.h"

FILE *p_out;
FILE *p_err;

ircclient_t *clients [MAXCLIENTS] = { 0 };
pthread_t threads [MAXCLIENTS];
char *configpath = "./config.lua";
config_t globalcfg;

void parseargs (int argc, char **argv)
{
	int i;
	for (i = 1; i < argc; i++)
	{
		int arglen = strlen (argv [i]);
		if (!strcmp (argv [i], "--config") || !strcmp (argv [i], "-c"))
		{
			configpath = argv [i + 1];
			continue;
		}

		if (!strcmp (argv [i], "--daemon") || !strcmp (argv [i], "-d"))
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
				p_out = fopen ("/dev/null", "a");
				p_err = fopen ("/dev/null", "a");
			}
			else
			{
				iprint ("Forked to background (pid: %i)", (int) pid);
				fprintf (pidfile, "%i\n", (int) pid);
				exit (0);
			}
			continue;
		}
	}

	return;
}

// Kills off all connections
void die (char *msg)
{
	int i;
	for (i = 0; i < MAXCLIENTS; i++)
		if (clients [i])
			irc_quit (clients [i], msg);

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

	p_out = stdout;
	p_err = stderr;

	parseargs (argc, argv);

	config_load (configpath, &globalcfg, clients);

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
			pthread_create (&threads [i], NULL, irc_init, (void*) &clients [i]);

	for (i = 0; i < MAXCLIENTS; i++)
		if (clients [i])
			pthread_join (threads [i], NULL);

	return 0;
}
