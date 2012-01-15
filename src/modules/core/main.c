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
#include <alloca.h>

#include "irc.h"
#include "module.h"

void corePrivmsg (ircclient_t *cl, char *nick, char *host, char *source, char *message)
{
	char *tokbuf = alloca (strlen (message));

	if (strstr (message, ".info") == message)
	{
		// todo: expand, of course...
		irc_privmsg (cl, source, "Hi, I am an ispolin bot!");
	}

	return;
}

void init (listener_t *privmsg)
{
	privmsg->func = &corePrivmsg;
	privmsg->next = NULL;

	return;
}
