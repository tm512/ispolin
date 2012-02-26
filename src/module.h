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

typedef void (*privmsglistener_f) (ircclient_t *cl, char *nick, char *host, char *source, char *message);
typedef void (*joinlistener_f) (ircclient_t *cl, char *nick, char *host, char *channel);
typedef void (*partlistener_f) (ircclient_t *cl, char *nick, char *host, char *channel, char *reason);
typedef void (*quitlistener_f) (ircclient_t *cl, char *nick, char *host, char *reason);

typedef struct listener_s
{
	void *func;
	const char *modname;
	void *mod;
	struct listener_s *next;
} listener_t;

int module_load (char *path);
int module_unload (char *name, listener_t **lp);
void module_registerfunc (listener_t **l, void *func, void *mod, const char *modname);

extern listener_t *privmsgListeners;
extern listener_t *joinListeners;
extern listener_t *partListeners;
extern listener_t *quitListeners;

#endif // PLUGIN_H__
