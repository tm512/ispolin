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
#include <dlfcn.h>

#include "prints.h"
#include "irc.h"
#include "module.h"

typedef void (*modinit_f) (listener_t *privmsg); // TODO: More listeners, of course...

listener_t privmsgListeners;

// Loads a module, passes all of our listener lists to the modules init function
// Returns 0 on success
int module_load (char *path)
{
	modinit_f init;
	void *mod = dlopen (path, RTLD_LAZY);

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

	init (&privmsgListeners);

	return 0;
}
