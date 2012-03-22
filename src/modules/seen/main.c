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
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef linux
#include <alloca.h>
#endif

#include "prints.h"
#include "irc.h"
#include "config.h"
#include "module.h"

char modname [] = "seen";

typedef struct seen_entry_s
{
	char nick [32];
	unsigned int seen_date;
	unsigned int offset;
	struct seen_entry_s *next;
} seen_entry_t;

seen_entry_t *table [128];
int seendb;

unsigned int quickhash (char *str)
{
	unsigned int hash = 0, c;

	while (c = *str++)
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash;
}

// Adds an entry to the in-memory hash table
seen_entry_t *insert_entry (char *nick, unsigned int seen_date)
{
	seen_entry_t *entry = table [quickhash (nick) % 128];

	if (!entry)
	{
		entry = malloc (sizeof (seen_entry_t));
		table [quickhash (nick) % 128] = entry;
	}
	else
	{
		while (entry->next)
			entry = entry->next;

		entry->next = malloc (sizeof (seen_entry_t));
		entry = entry->next;
	}

	strncpy (entry->nick, nick, 32);
	entry->seen_date = seen_date;
	entry->next = NULL;

	return entry;
}

// Adds an entry to the in-memory database, and then writes to the on-disk database
seen_entry_t *insert_entry_file (char *nick, unsigned int seen_date)
{
	seen_entry_t *entry = insert_entry (nick, seen_date);

	entry->offset = lseek (seendb, 0, SEEK_END);
	write (seendb, entry->nick, 32);
	write (seendb, &(entry->seen_date), 4);

	return entry;
}

// Returns the entry for the provided nick, or NULL if it isn't found
seen_entry_t *get_entry (char *nick)
{
	seen_entry_t *entry = table [quickhash (nick) % 128];

	while (entry && strncmp (nick, entry->nick, strlen (nick) < strlen (entry->nick) ? strlen (nick) : strlen (entry->nick)))
		entry = entry->next;

	return entry;
}

void update_entry (char *nick, unsigned int seen_date)
{
	seen_entry_t *entry = get_entry (nick);

	if (!entry)
	{
		insert_entry_file (nick, seen_date);
		return;
	}

	lseek (seendb, entry->offset + 32, SEEK_SET);
	entry->seen_date = seen_date;
	write (seendb, &(entry->seen_date), 4);

	return;
}

void seenJoin (ircclient_t *cl, char *nick, char *host, char *channel)
{
	update_entry (nick, time (NULL));
	return;
}		

void seenNick (ircclient_t *cl, char *nick, char *host, char *newnick)
{
	update_entry (nick, time (NULL));
	update_entry (newnick, time (NULL));
	return;
}

void seenPart (ircclient_t *cl, char *nick, char *host, char *channel, char *reason)
{
	update_entry (nick, time (NULL));
	return;
}

void seenQuit (ircclient_t *cl, char *nick, char *host, char *reason)
{
	update_entry (nick, time (NULL));
	return;
}

void seenPrivmsg (ircclient_t *cl, char *nick, char *host, char *source, char *message)
{
	char *buf, *tokbuf, *tnick;
	seen_entry_t *entry;
	update_entry (nick, time (NULL));

	if (message [0] == globalcfg.prefix)
	{
		buf = alloca (strlen (message));
		tokbuf = alloca (strlen (message));
		strncpy (buf, message + 1, strlen (message));

		if (strstr (buf, "seen ") == buf)
		{
			strtok_r (buf, " ", &tokbuf);
			tnick = strtok_r (NULL, " ", &tokbuf);

			if (!strlen (tnick))
				return;

			entry = get_entry (tnick);

			if (entry)
			{
				char timestring [32];
				time_t t = (time_t) entry->seen_date;
				strftime (timestring, 32, "%b %d, %Y, at %T %Z", localtime (&t));
				irc_privmsg (cl, source, "%s: The last time I saw activity from %s was on %s.", nick, entry->nick, timestring);
			}
			else
				irc_privmsg (cl, source, "%s: I have never seen %s before.", nick, tnick);
		}
	}
	return;
}

void init (void)
{
	char nickbuf [32];
	unsigned int timebuf;
	seen_entry_t *entp;
	int i;

	for (i = 0; i < 128; i++)
		table [i] = NULL;

	seendb = open ("./seen.db", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); // todo - variable
	if (seendb < 0)
	{
		eprint (0, "seen: Couldn't open or create database file.");
		return;
	}

	while (1)
	{
		if (read (seendb, nickbuf, 32) != 32)
			break;

		if (read (seendb, &timebuf, 4) != 4)
			break;

		entp = insert_entry (nickbuf, timebuf);
		entp->offset = lseek (seendb, 0, SEEK_CUR) - 36;
	}

	module_registerfunc (&joinListeners, seenJoin, modname);
	module_registerfunc (&nickListeners, seenNick, modname);
	module_registerfunc (&partListeners, seenPart, modname);
	module_registerfunc (&privmsgListeners, seenPrivmsg, modname);
	module_registerfunc (&quitListeners, seenQuit, modname);

	return;
}

void deinit (void)
{
       int i;

       for (i = 0; i < 128; i++)
       {
               seen_entry_t *entry = table [i], *next;
               while (entry)
               {
                       next = entry->next;
                       free (entry);
                       entry = next;
               }
       }

       close (seendb);
       return;
}
