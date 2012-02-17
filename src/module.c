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
#include <dlfcn.h>

#include "prints.h"
#include "irc.h"
#include "module.h"

typedef void (*modinit_f) (void *mod);

listener_t *privmsgListeners = NULL;

// Loads a module, passes all of our listener lists to the modules init function
// Returns 0 on success
int module_load (char *path)
{
	modinit_f init;
	void *mod = dlopen (path, RTLD_LAZY);

	iprint ("Loading module: %s", path);

	if (!mod)
	{
		eprint (0, "Couldn't load module %s (%s).", path, dlerror ());
		return 1;
	}

	init = (modinit_f) dlsym (mod, "init");

	if (!init)
	{
		eprint (0, "Couldn't load init function from module %s (%s).", path, dlerror ());
		return 2;
	}

	init (mod);

	return 0;
}

#define MAX(a,b) ((a < b) ? b : a)

int module_unload (char *name, listener_t **lp)
{
	listener_t *it = *lp;
	void *mod = NULL;

	if (!strncmp ("core", name, MAX (4, strlen (it->modname))))
		return 1; // Do not unload the core module

	if (!strncmp (name, it->modname, MAX (strlen (name), strlen (it->modname))))
	{
		(*lp) = it->next;
		mod = it->mod;
		free (it);
	}

	while (1)
	{
		if (!it->next)
			break;

		if (!strncmp (name, it->next->modname, MAX (strlen (name), strlen (it->next->modname))))
		{
			listener_t *next = it->next->next;

			if (mod && mod != it->next->mod) // Huh?
				dlclose (mod);

			mod = it->next->mod;

			free (it->next);
			it->next = next;
		}
	}

	if (mod)
	{
		iprint ("Unloaded module: %s", name);
		dlclose (mod);
	}
	else
	{
		eprint (0, "Module %s doesn't appear to be loaded", name);
		return 2;
	}

	return 0;
}

#undef MAX

void module_registerfunc (listener_t **lp, void *func, void *mod, const char *modname)
{
	listener_t *l = *lp;

	if (!l)
	{
		*lp = (listener_t*) malloc (sizeof (listener_t));
		l = *lp;
		l->func = func;
		l->modname = modname;
		l->mod = mod;
		l->next = NULL;
	}
	else
	{
		while (l->next)
			l = l->next;

		l->next = (listener_t*) malloc (sizeof (listener_t));
		l->next->func = func;
		l->next->modname = modname;
		l->next->mod = mod;
		l->next->next = NULL;
	}

	return;
}
