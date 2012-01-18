// crappy test main here
// needs to be replaced, of course...

#include <stdio.h>
#include <stdlib.h>

#include "net.h"
#include "irc.h"
#include "module.h"
#include "config.h"

ircclient_t *clients [MAXCLIENTS] = { 0 };
config_t globalcfg;

int main (void)
{
	module_load ("./modules/core.so");
	config_load ("./config.lua", &globalcfg, clients);
	irc_init (clients [0]);

	return 0;
}
