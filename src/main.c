// crappy test main here
// needs to be replaced, of course...

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "net.h"
#include "irc.h"
#include "module.h"
#include "config.h"

ircclient_t *clients [MAXCLIENTS] = { 0 };
pthread_t threads [MAXCLIENTS];
config_t globalcfg;

int main (void)
{
	int i;

	module_load ("./modules/core.so");
	config_load ("./config.lua", &globalcfg, clients);

	for (i = 0; i < MAXCLIENTS; i++)
		if (clients [i])
			pthread_create (&threads [i], NULL, irc_init, (void*) clients [i]);

	for (i = 0; i < MAXCLIENTS; i++)
		if (clients [i])
			pthread_join (threads [i], NULL);

	return 0;
}

// Kills off all connections
void die (void)
{
	int i;
	for (i = 0; i < MAXCLIENTS; i++)
		if (clients [i])
			irc_quit (clients [i], "ispolin");

	return;
}
