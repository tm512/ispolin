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

#ifndef PLUGIN_H__
#define PLUGIN_H__

typedef void (*joinlistener_f) (ircclient_t *cl, char *nick, char *host, char *channel);
typedef void (*nicklistener_f) (ircclient_t *cl, char *nick, char *host, char *newnick);
typedef void (*partlistener_f) (ircclient_t *cl, char *nick, char *host, char *channel, char *reason);
typedef void (*privmsglistener_f) (ircclient_t *cl, char *nick, char *host, char *source, char *message);
typedef void (*quitlistener_f) (ircclient_t *cl, char *nick, char *host, char *reason);

typedef struct module_s
{
	void *mod;
	char *modname;
	struct module_s *next;
} module_t;

typedef struct listener_s
{
	void *func;
	char *modname;
	struct listener_s *next;
} listener_t;

int module_load (char *path);
int module_unload (char *name);
void module_listener_clear (char *name, listener_t **lp);
void module_registerfunc (listener_t **l, void *func, char *modname);
void module_die (void);

extern listener_t *joinListeners;
extern listener_t *nickListeners;
extern listener_t *partListeners;
extern listener_t *privmsgListeners;
extern listener_t *quitListeners;

#endif // PLUGIN_H__
