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
#include <stdarg.h>

#include "prints.h"
#include "irc.h"

inline void iprint (const char *fmt, ...)
{
	va_list va;

	va_start (va, fmt);

	fprintf (stdout, "[\033[1;32;40m*\033[0m] ");
	vfprintf (stdout, fmt, va);
	fprintf (stdout, "\n");

	va_end (va);

	return;
}

inline void eprint (char fatal, const char *fmt, ...)
{
	va_list va;

	va_start (va, fmt);

	fprintf (stderr, "[\033[1;31;40m!\033[0m] ");
	vfprintf (stderr, fmt, va);
	fprintf (stderr, "\n");

	va_end (va);

	if (fatal)
		exit (1);

	return;
}

inline void dprint (const char *fmt, ...)
{
	#ifdef DEBUG
	va_list va;

	va_start (va, fmt);

	fprintf (stderr, "[\033[1;33;40m~\033[0m] ");
	vfprintf (stderr, fmt, va);
	fprintf (stderr, "\n");

	va_end (va);
	#endif

	return;
}

void ircprint (void *clv, const char *fmt, ...)
{
	va_list va;
	ircclient_t *cl = clv;

	va_start (va, fmt);

	fprintf (stdout, "[\033[1;34;40m%s\033[0m] ", cl->host);
	vfprintf (stdout, fmt, va);
	fprintf (stdout, "\n");

	va_end (va);

	return;
}
