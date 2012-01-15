// crappy test main here
// needs to be replaced, of course...

#include <stdio.h>
#include <stdlib.h>

#include "net.h"
#include "irc.h"
#include "module.h"

static const char host [] = "crimson.lostsig.net", port [] = "6667";
// static const char host [] = "localhost", port [] = "1337";

int main (void)
{
	ircclient_t c;

	module_load ("./modules/core.so");
	irc_init (&c, host, port);

	return 0;
}
