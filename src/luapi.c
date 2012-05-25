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

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "prints.h"
#include "irc.h"
#include "config.h"
#include "luapi.h"

lua_State *Lst = NULL;

// Functions to register to Lua:

int luapi_setuser (lua_State *L)
{
	// setuser (nick, username, realname)
	int nargs = lua_gettop (L), i;
	if (nargs < 3)
	{
		lua_pushstring (L, "Not enough arguments");
		lua_error (L);
	}

	for (i = 1; i <= nargs; i++)
		if (!lua_isstring (L, i))
		{
			lua_pushstring (L, "Invalid argument");
			lua_error (L);
		}

	dprint ("setuser (\"%s\", \"%s\", \"%s\")", lua_tostring (L, 1), lua_tostring (L, 2), lua_tostring (L, 3));

	globalcfg.nick = lua_tostring (L, 1);
	globalcfg.username = lua_tostring (L, 2);
	globalcfg.realname = lua_tostring (L, 3);

	return 0;
}

// Functions used to control the API from C

void luapi_init (void)
{
	iprint ("Initializing Lua API");

	if (!(Lst = lua_open ()))
		eprint (1, "Could not initialize Lua");

	luaL_openlibs (Lst);
	lua_register (Lst, "setuser", luapi_setuser);

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
