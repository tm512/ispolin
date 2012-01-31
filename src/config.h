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

#ifndef CONFIG_H__
#define CONFIG_H__

typedef struct
{
	char *nick;
	char *username;
	char *realname;

	char prefix;
} config_t;

int load_config (char *filename, config_t *cfg, ircclient_t **clients);

extern config_t globalcfg;

#endif // CONFIG_H__
