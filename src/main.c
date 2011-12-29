// crappy test main here
// needs to be replaced, of course...

#include <stdio.h>

#include "net.h"

int main (void)
{
	int s = net_connect ("localhost", "1337");

	net_send (s, "NICK ispolin\r\n", 128);
	net_send (s, "USER ispolin 8 * :fuck\r\n", 128);
	sleep (1);
	net_send (s, "JOIN #test\r\n", 128);

	for (;;)
	{
		sleep (20);
		net_send (s, "PONG :entryway\r\n", 128);
	}

	return 0;
}
