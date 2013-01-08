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

#ifndef NET_H__
#define NET_H__

int net_connect (const char *host, const char *port);
int net_send (int sock, char *buf, unsigned int len);
int net_recv (int sock, char *buf, unsigned int len);
int net_close (int sock);
void net_addsock (int sock);
int net_isset (int sock);
int net_select (void);

#endif // NET_H__
