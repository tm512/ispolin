// crappy test main here
// needs to be replaced, of course...

#include <stdio.h>
#include <stdlib.h>

#include "net.h"
#include "irc.h"

static const char host [] = "localhost", port [] = "1337";

int main (void)
{
	ircclient_t c;

	irc_init (&c, host, port);

	return 0;
}
