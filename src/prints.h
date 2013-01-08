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

#ifndef PRINTS_H__
#define PRINTS_H__

#define DEBUG

inline void iprint (const char *fmt, ...);
inline void eprint (char fatal, const char *fmt, ...);
inline void dprint (const char *fmt, ...);
void ircprint (void *clv, const char *fmt, ...); // void ircclient to remove dependency on irc.h

#endif // PRINTS_H__
