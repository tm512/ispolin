/*
   ispolin
   Copyright [c] 2011 tm512 (Kyle Davis), All Rights Reserved.

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

#ifndef PRINTS_H__
#define PRINTS_H__

#define DEBUG 1
#undef assert // We want to use our own assert, so undefine if necessary

/* convenient macros... */

#define iprint(s, ...) \
	fprintf (stdout, "[\033[1;32;40m*\033[0m] " s "\n", ##__VA_ARGS__)

#ifdef DEBUG
#define dprint(s, ...) \
	fprintf (stderr, "[\033[1;33;40m~\033[0m] " s " (%s:%i)\n", ##__VA_ARGS__, __FILE__, __LINE__)
#else
#define dprint(s, ...)
#endif 

#define eprint(fatal, s, ...) \
	{ \
	fprintf (stderr, "[\033[1;31;40m!\033[0m] " s "\n", ##__VA_ARGS__); \
	if (fatal) exit (1); \
	}

#define assert(E) \
	if (!(E)) { eprint (1, "Assertion at %s:%i failed.", __FILE__, __LINE__) }

#endif // PRINTS_H__
