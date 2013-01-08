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
#include "module.h"
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

static ircclient_t *getclient (lua_State *L, int cln)
{
	ircclient_t *cl;
	if (cln >= MAXCLIENTS)
	{
		luaL_error (L, "Client number exceeds maximum clients");
		return NULL;
	}

	cl = clients [cln];

	if (!cl)
	{
		luaL_error (L, "Invalid client");
		return NULL;
	}

	return cl;
}

// Functions to register to Lua:

int luapi_client_new (lua_State *L)
{
	int i;

	for (i = 0; i < MAXCLIENTS && clients [i]; i++); // find empty slot
	if (i >= MAXCLIENTS)
	{
		eprint (0, "Connection to %s:%s would exceed maximum client connection count", lua_tostring (L, 1), lua_tostring (L, 2));
		luaL_error (L, "Too many clients");
	}

	clients [i] = malloc (sizeof (ircclient_t));
	memset (clients [i], 0, sizeof (ircclient_t));
	memset (clients [i]->channels, 0, sizeof (ircchannel_t) * MAXCHANS);

	clients [i]->host = deconst (luaL_checkstring (L, 1));
	clients [i]->port = deconst (luaL_checkstring (L, 2));

	lua_pushnumber (L, i);
	return 1;
}

int luapi_client_ref (lua_State *L)
{
	// store reference to the table that an ircclient_t is represented by
	ircclient_t *cl = getclient (L, luaL_checknumber (L, 1));

	cl->ref = luaL_ref (L, LUA_REGISTRYINDEX);
	return 0;
}

int luapi_client_setcfg (lua_State *L)
{
	ircclient_t *cl = getclient (L, luaL_checknumber (L, 1));

	cl->nick = deconst (luaL_checkstring (L, 2));
	cl->username = deconst (luaL_checkstring (L, 3));
	cl->realname = deconst (luaL_checkstring (L, 4));
	cl->prefix = luaL_checkstring (L, 5) [0];
	cl->owner = deconst (luaL_checkstring (L, 6));

	return 0;
}

int luapi_client_connect (lua_State *L)
{
	ircclient_t *cl = getclient (L, luaL_checknumber (L, 1));
	int i = luaL_checknumber (L, 1);

	if (irc_init (cl))
	{
		lua_pushboolean (L, 0);
		irc_destroy (&clients [i]);
	}

	lua_pushboolean (L, 1);
	return 1;
}

int luapi_client_join (lua_State *L)
{
	int i;
	ircclient_t *cl = getclient (L, luaL_checknumber (L, 1));

	for (i = 0; i < MAXCHANS; i++)
		if (!cl->channels [i].name)
			break;

	if (i == MAXCHANS)
	{
		lua_pushstring (L, "Exceeded maximum channel count");
		lua_error (L);
	}

	cl->channels [i].name = deconst (luaL_checkstring (L, 2));
	cl->channels [i].pass = deconst (luaL_optstring (L, 3, NULL));

	irc_join (cl, &cl->channels [i]);

	return 0;
}

int luapi_client_part (lua_State *L)
{
	ircclient_t *cl = getclient (L, luaL_checknumber (L, 1));
	char *channame = deconst (luaL_checkstring (L, 2));
	int i;

	for (i = 0; i < MAXCHANS; i++)
		if (cl->channels [i].name && !strcmp (channame, cl->channels [i].name))
			break;

	if (i == MAXCHANS)
	{
		luaL_error (L, "Not joined to channel");
		return;
	}

	irc_part (cl, &cl->channels [i], "ispolin"); // todo: customizable
	return 0;
}

int luapi_client_raw (lua_State *L)
{
	ircclient_t *cl = getclient (L, luaL_checknumber (L, 1));
	char *message = deconst (luaL_optstring (L, 2, NULL));

	if (message)
		irc_sendln (cl, "%s", message);

	free (message);
	return 0;
}

int luapi_client_privmsg (lua_State *L)
{
	ircclient_t *cl = getclient (L, luaL_checknumber (L, 1));
	char *target = deconst (luaL_optstring (L, 2, NULL));
	char *message = deconst (luaL_optstring (L, 3, NULL));
	
	if (target && message)
		irc_privmsg (cl, target, message);

	free (target);
	free (message);
	return 0;
}

int luapi_module_load (lua_State *L)
{
	char *path = deconst (luaL_checkstring (L, 1));
	module_t *mod = module_load (path);
	lua_pushstring (L, mod->modname);

	free (path);
	return 1;
}

int luapi_module_unload (lua_State *L)
{
	char *name = deconst (luaL_checkstring (L, 1));
	module_unload (name);

	free (name);
	return 0;
}

// Functions used to control the API from C

luaL_Reg core [] =
{
	{ "client_new", luapi_client_new },
	{ "client_ref", luapi_client_ref },
	{ "client_setcfg", luapi_client_setcfg },
	{ "client_connect", luapi_client_connect },
	{ "client_join", luapi_client_join },
	{ "client_part", luapi_client_part },
	{ "client_raw", luapi_client_raw },
	{ "client_privmsg", luapi_client_privmsg },
	{ "module_load", luapi_module_load },
	{ "module_unload", luapi_module_unload }
};

void luapi_init (void)
{
	iprint ("Initializing Lua API (Using %s)", LUA_RELEASE);

	if (!(Lst = luaL_newstate ()))
		eprint (1, "Could not initialize Lua");

	luaL_openlibs (Lst);

	// put all of the core functions into a table
	luaL_register (Lst, "core", core);

	lua_pushnumber (Lst, MAXCLIENTS);
	lua_setglobal (Lst, "core.maxclients");

	// run lua script that wraps all the functions nicely
	if (luaL_dofile (Lst, "scripts/init.lua"))
		eprint (1, "Error running scripts/init.lua");

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

// Call method in a client
void luapi_call (ircclient_t *cl, const char *method)
{
	lua_rawgeti (Lst, LUA_REGISTRYINDEX, cl->ref);
	lua_getfield (Lst, -1, method);
	lua_pushvalue (Lst, -2);

	if (lua_isfunction (Lst, -2))
		lua_call (Lst, 1, 0);

	lua_pop (Lst, 3);
	return;
}
