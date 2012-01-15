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

#ifndef PLUGIN_H__
#define PLUGIN_H__

int module_load (char *path);

typedef void (*privmsglistener_f) (ircclient_t *cl, char *nick, char *host, char *source, char *message);

typedef struct listener_s
{
	void *func;
	struct listener_s *next;
} listener_t;

extern listener_t privmsgListeners;
/*
extern listener_t joinListeners;
extern listener_t partListeners;
*/

#endif // PLUGIN_H__
