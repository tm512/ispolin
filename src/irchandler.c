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
#include <alloca.h>

#include "prints.h"
#include "irchandler.h"

void join_handler (char *nick, char *host, char *args)
{
	char *channel = strstr (args, ":") + 1;

	iprint ("[%s] %s (%s) joins.", channel, nick, host);

	return;
}

void part_handler (char *nick, char *host, char *args)
{
	char *tokbuf = alloca (strlen (args));
	char *channel = strtok_r (args, " ", &tokbuf);
	char *reason = strtok_r (NULL, " ", &tokbuf) + 1;

	iprint ("[%s] %s (%s) parts [%s].", channel, nick, host, reason);

	return;
}

void privmsg_handler (char *nick, char *host, char *args)
{
	return;
}
