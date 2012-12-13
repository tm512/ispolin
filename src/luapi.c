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
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "prints.h"
#include "irc.h"
#include "config.h"
#include "luapi.h"

lua_State *Lst = NULL;

// copy const char* to regular char*
// useful for things that lua gives us
static char *deconst (const char *source)
{
	if (!source)
		return NULL;

	char *tmp = malloc (strlen (source) + 1);
	strcpy (tmp, source);
	return tmp;
}

// Functions to register to Lua:

int luapi_setuser (lua_State *L)
{
	// setuser (nick, username, realname)
	globalcfg.nick = deconst (luaL_checkstring (L, 1));
	globalcfg.username = deconst (luaL_checkstring (L, 2));
	globalcfg.realname = deconst (luaL_checkstring (L, 3));

	return 0;
}

int luapi_setprefix (lua_State *L)
{
	// setprefix (prefix)

	globalcfg.prefix = luaL_checkstring (L, 1) [0];

	return 0;
}

int luapi_irc_connect (lua_State *L)
{
	// irc_connect (host, port, [password])
	int i;

	for (i = 0; i < MAXCLIENTS && clients [i]; i++); // find empty slot
	if (i >= MAXCLIENTS)
	{
		eprint (0, "Connection to %s:%s would exceed maximum client connection count", lua_tostring (L, 1), lua_tostring (L, 2));
		lua_pushstring (L, "Too many clients");
		lua_error (L);
	}

	clients [i] = malloc (sizeof (ircclient_t));
	memset (clients [i], 0, sizeof (ircclient_t));

	clients [i]->host = deconst (luaL_checkstring (L, 1));
	clients [i]->port = deconst (luaL_checkstring (L, 2));
	clients [i]->nick = globalcfg.nick;

	lua_pushnumber (L, i);
	return 1;
}

int luapi_irc_setowner (lua_State *L)
{
	int cln;
	ircclient_t *cl;

	cln = luaL_checknumber (L, 1);
	if (cln >= MAXCLIENTS)
	{
		lua_pushstring (L, "Client number too high");
		lua_error (L);
	}

	cl = clients [cln];
	if (!cl)
	{
		lua_pushstring (L, "No such client");
		lua_error (L);
	}

	cl->owner = deconst (luaL_checkstring (L, 2));
	return 0;
}

int luapi_irc_addchannel (lua_State *L)
{
	int cln;
	ircclient_t *cl;

	cln = luaL_checknumber (L, 1);
	if (cln >= MAXCLIENTS)
	{
		lua_pushstring (L, "Client number too high");
		lua_error (L);
	}

	cl = clients [cln];
	if (!cl)
	{
		lua_pushstring (L, "No such client");
		lua_error (L);
	}

	chanlist_t *ch;
	if (!cl->channels)
	{
		cl->channels = malloc (sizeof (chanlist_t));
		memset (cl->channels, 0, sizeof (chanlist_t));
		ch = cl->channels;
	}
	else
	{
		ch = cl->channels;
		while (ch->next)
			ch = ch->next;

		ch->next = malloc (sizeof (chanlist_t));
		ch = ch->next;
		memset (ch, 0, sizeof (chanlist_t));
	}

	ch->name = deconst (luaL_checkstring (L, 2));
	ch->pass = deconst (luaL_optstring (L, 3, NULL));

	return 0;
}

// Functions used to control the API from C

void luapi_init (void)
{
	iprint ("Initializing Lua API (Using %s)", LUA_RELEASE);

	if (!(Lst = luaL_newstate ()))
		eprint (1, "Could not initialize Lua");

	luaL_openlibs (Lst);

	// bind functions
	lua_register (Lst, "setuser", luapi_setuser);
	lua_register (Lst, "setprefix", luapi_setprefix);
	lua_register (Lst, "irc_connect", luapi_irc_connect);
	lua_register (Lst, "irc_setowner", luapi_irc_setowner);
	lua_register (Lst, "irc_addchannel", luapi_irc_addchannel);

	// run lua script that wraps all the functions nicely
	luaL_dofile (Lst, "scripts/init.lua");

	return;
}

// Load configuration file and run it
void luapi_loadconfig (char *path)
{
	iprint ("Loading configuration file: %s", path);
	
	if (luaL_dofile (Lst, path))
		eprint (1, "Error loading configuration file");

	return;
}
