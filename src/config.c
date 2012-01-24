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

int config_load (char *filename, config_t *cfg, ircclient_t **clients)
{
	int i;
	lua_State *L = lua_open ();

	if (!L)
	{
		eprint (1, "Could not initialize Lua", filename);
	}

	luaL_openlibs (L);

	if (luaL_loadfile (L, filename) || lua_pcall (L, 0, 0, 0))
	{
		eprint (1, "Could not open config file %s.", filename);
	}

	// Load defaults
	lua_getglobal (L, "nick");
	lua_getglobal (L, "username");
	lua_getglobal (L, "realname");

	if (!lua_isstring (L, -1) || !lua_isstring (L, -2) || !lua_isstring (L, -3))
	{
		eprint (1, "Could not load defaults. Make sure that \"nick\", \"username\", and \"realname\" are set in %s.", filename);
	}

	cfg->nick = (char*) malloc (strlen (lua_tostring (L, -3)));
	cfg->username = (char*) malloc (strlen (lua_tostring (L, -2)));
	cfg->realname = (char*) malloc (strlen (lua_tostring (L, -1)));

	strcpy (cfg->nick, lua_tostring (L, -3));
	strcpy (cfg->username, lua_tostring (L, -2));
	strcpy (cfg->realname, lua_tostring (L, -1));

	lua_pop (L, 3);

	dprint ("nick = %s, username = %s, realname = %s", cfg->nick, cfg->username, cfg->realname);

	lua_getglobal (L, "servers");

	if (!lua_istable (L, -1))
	{
		eprint (1, "Server table not set in %s.", filename);
	}

	// Load up the server tables
	for (i = 1; i <= MAXCLIENTS; i++)
	{
		ircclient_t *cl;
		int j;

		lua_pushnumber (L, i);
		lua_gettable (L, -2);

		if (!lua_istable (L, -1))
		{
			lua_pop (L, 1);
			continue;
		}

		lua_pushstring (L, "host");
		lua_gettable (L, -2);
		lua_pushstring (L, "port");
		lua_gettable (L, -3);
		lua_pushstring (L, "nick");
		lua_gettable (L, -4);

		if (!lua_isstring (L, -3))
		{
			lua_pop (L, 4);
			continue;
		}

		clients [i - 1] = (ircclient_t*) malloc (sizeof (ircclient_t));
		cl = clients [i - 1];

		cl->host = (char*) malloc (strlen (lua_tostring (L, -3)));
		cl->port = (char*) malloc (5); // port number only goes from 0 to 65535
		cl->nick = (char*) malloc (lua_isstring (L, -1) ? strlen (lua_tostring (L, -1)) : strlen (cfg->nick));

		strcpy (clients [i - 1]->host, lua_tostring (L, -3));

		if (lua_isnumber (L, -2))
			sprintf (cl->port, "%i", (int) lua_tonumber (L, -2));
		else
			sprintf (cl->port, "%i", 6667);

		strcpy (cl->nick, lua_isstring (L, -1) ? lua_tostring (L, -1) : cfg->nick);

		lua_pop (L, 3);

		// Load up channels
		lua_pushstring (L, "channels");
		lua_gettable (L, -2);

		if (!lua_istable (L, -1))
		{
			lua_pop (L, 2);
			eprint (0, "Channel array for %s wasn't found. Continuing without joining any channels", cl->host);
			cl->channels = NULL;
			continue;
		}

		cl->channels = (chanlist_t*) malloc (sizeof (chanlist_t));
		chanlist_t *it = cl->channels;

		j = 1;
		while (1)
		{
			lua_pushnumber (L, j);
			lua_gettable (L, -2);

			if (lua_isstring (L, -1)) // no password
			{
				dprint ("adding %s", lua_tostring (L, -1));
				strncpy ((char*) it->name, lua_tostring (L, -1), strlen (lua_tostring (L, -1)));
				it->pass [0] = '\0';
				lua_pop (L, 1);
			}
			else if (lua_istable (L, -1)) // Passworded
			{
				lua_pushnumber (L, 1);
				lua_gettable (L, -2);
				lua_pushnumber (L, 2);
				lua_gettable (L, -3);

				if (lua_isstring (L, -1) && lua_isstring (L, -2))
				{
					dprint ("adding %s with password %s", lua_tostring (L, -2), lua_tostring (L, -1));
					strncpy ((char*) it->name, lua_tostring (L, -2), strlen (lua_tostring (L, -2)));
					strncpy ((char*) it->pass, lua_tostring (L, -1), strlen (lua_tostring (L, -1)));
				}

				lua_pop (L, 3);
			}
			else if (lua_isnil (L, -1))
				break;

			it->next = (chanlist_t*) malloc (sizeof (chanlist_t));
			it = it->next;
			it->next = NULL;
			j++;
		}

		lua_pop (L, 3);
	}

	lua_close (L);

	return 0;
}
