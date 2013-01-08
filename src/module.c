/*
   ispolin
   Copyright [c] 2011-2013 tm512 (Kyle Davis), All Rights Reserved.

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
#include <dlfcn.h>

#include "prints.h"
#include "irc.h"
#include "module.h"

typedef void (*modinit_f) (void);
typedef void (*moddeinit_f) (void);

module_t *modules = NULL;

listener_t *joinListeners = NULL;
listener_t *nickListeners = NULL;
listener_t *partListeners = NULL;
listener_t *privmsgListeners = NULL;
listener_t *quitListeners = NULL;

// Loads a module, passes all of our listener lists to the modules init function
// Returns 0 on success
int module_load (char *path)
{
	modinit_f init;
	void *mod = dlopen (path, RTLD_LAZY);
	module_t *it = modules;
	char *modname;

	iprint ("Loading module: %s", path);

	if (!mod)
	{
		eprint (0, "Couldn't load module %s (%s).", path, dlerror ());
		dlclose (mod);
		return 1;
	}

	init = (modinit_f) dlsym (mod, "init");

	if (!init)
	{
		eprint (0, "Couldn't load init function from module %s (%s).", path, dlerror ());
		dlclose (mod);
		return 2;
	}

	modname = (char*) dlsym (mod, "modname");
	if (!modname)
	{
		eprint (0, "Module %s has no name.");
		dlclose (mod);
		return 3;
	}

	if (it) // ensure that this module isn't loaded
	{
		while (it)
		{
			if (!strcmp (modname, it->modname))
			{
				eprint (0, "Module %s is already loaded", modname);
				dlclose (mod);
				return 4;
			}
			it = it->next;
		}
		it = modules;
	}
			
	// Add module to linked list
	if (it)
	{
		while (it->next)
			it = it->next;

		it->next = malloc (sizeof (module_t));
		it = it->next;
	}
	else
		modules = it = malloc (sizeof (module_t));

	it->mod = mod;
	it->modname = modname;
	it->next = NULL;

	if (!it->modname)
	{
		free (it);
		it = NULL;
		return 3;
	}

	init ();

	return 0;
}

int module_unload (char *name)
{
	module_t *it = modules;
	void *mod = NULL;
	moddeinit_f deinit;

	// Clear all listeners that this module has
	module_listener_clear (name, &joinListeners);
	module_listener_clear (name, &nickListeners);
	module_listener_clear (name, &partListeners);
	module_listener_clear (name, &privmsgListeners);
	module_listener_clear (name, &quitListeners);

	// This closes up the linked list to exclude the module_t we need to unload
	// Might not be that effecient for huge linked lists, but it will work for us
	while (it)
	{
		module_t *next = it->next;
		module_t *prev = modules;

		if (!strcmp (name, it->modname)) // this module matches, need to delete it
		{
			if (it == modules) // if this is the first in the list
				modules = it->next; // move the start of the list forward
			else // we need to find the previous
			{
				while (prev->next != it)
					prev = prev->next;

				prev->next = it->next;
			}

			deinit = (moddeinit_f) dlsym (it->mod, "deinit");
			if (deinit)
				deinit ();

			dlclose (it->mod);
			free (it);
			break;
		}

		it = next;
	}

	if (it)
	{
		iprint ("Successfully unloaded module: %s", name);
		return 0;
	}
	else
	{
		iprint ("Module %s is not loaded", name);
		return 1;
	}
}

void module_listener_clear (char *name, listener_t **lp)
{
	listener_t *it = *lp;

	while (it)
	{
		listener_t *next = it->next;
		listener_t *prev = *lp;

		if (!strcmp (name, it->modname))
		{
			if (it == *lp) // first in the list...
				(*lp) = it->next;
			else
			{
				while (prev->next != it)
					prev = prev->next;

				prev->next = it->next;
			}

			free (it);
		}

		it = next;
	}

	return;
}

void module_registerfunc (listener_t **lp, void *func, char *modname)
{
	listener_t *l = *lp;

	if (!l)
	{
		*lp = (listener_t*) malloc (sizeof (listener_t));
		l = *lp;
		l->func = func;
		l->modname = modname;
		l->next = NULL;
	}
	else
	{
		while (l->next)
			l = l->next;

		l->next = (listener_t*) malloc (sizeof (listener_t));
		l->next->func = func;
		l->next->modname = modname;
		l->next->next = NULL;
	}

	return;
}

void module_die (void)
{
	module_t *it = modules, *next;
	moddeinit_f deinit;

	while (it)
	{
		next = it->next;

		deinit = (moddeinit_f) dlsym (it->mod, "deinit");
		if (deinit)
			deinit ();

		free (it);
		it = next;
	}

	modules = NULL; // Don't run module_die again

	return;
}
