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

// The following source file is a good reason why manipulating 
// strings in C is a horrible idea.

#define _GNU_SOURCE // :S

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef linux
#include <alloca.h>
#else
#include <stdlib.h>
#endif

#include <curl/curl.h>

#include "prints.h"
#include "irc.h"
#include "module.h"

#include "htmlchar.h"

#define stripw(str) \
	while (strlen (str) > 0 && isspace (str [strlen (str) - 1])) str [strlen (str) - 1] = '\0';

typedef struct
{
	char *data;
	unsigned int len;
} memchunk_t;

char modname [] = "linktitle";
memchunk_t head, body;

size_t writedata (void *ptr, size_t size, size_t nmeb, void *stream)
{
	memchunk_t *tmp = (memchunk_t*) stream;

	tmp->data = realloc (tmp->data, tmp->len + (size * nmeb) + 1);
	if (!tmp->data)
	{
		eprint (1, "linktitle: realloc on 0x%x failed.", stream);
	}

	memcpy (&(tmp->data [tmp->len]), ptr, size * nmeb);
	tmp->len += size * nmeb;
	tmp->data [tmp->len] = 0;

	return size * nmeb;
}

void condense_spaces (char *c)
{
	char *d = c;

	for (; *c; c++)
	{
		*d++ = *c;
		if (isspace (*c))
		{
			while (isspace (*c))
				c++;
			c --;
		}
	}

	*d = 0;
	return;
}

void convertchars (char *c)
{
	int i;
	char *d, *e;
	for (i = 0; htmlchar [i].code; i++)
	{
		while ((d = strstr (c, htmlchar [i].code)))
		{
			if (!d)
				continue;

			e = strstr (d, ";") + 1;

			strcpy (d, htmlchar [i].ch);
			d += strlen (htmlchar [i].ch);

			// In-place copy
			while (*e)
				*(d++) = *(e++);
			*d = '\0';
		}
	}

	return;
}

void get_title (char **link, char **ttag)
{
	int i;
	char *ctag = strcasestr (*ttag, "</title>");

	if (!ctag)
		return;

	// exclude <title>
	*ttag = strstr (*ttag, ">") + 1;

	// null terminate after closing tag
	*ctag = '\0';

	// convert newlines to spaces. Yes, youtube, I'm looking at you
	for (i = 0; (*ttag) [i] != '\0'; i++)
		if ((*ttag) [i] == '\r' || (*ttag) [i] == '\n')
			(*ttag) [i] = ' ';

	// condense spaces, youtube, I'm looking at you again :|
	condense_spaces (*ttag);
	stripw ((*ttag));
	while (isspace (**ttag))
		(*ttag) ++;

	// Convert &amp; type character codes
	if (strstr (*ttag, "&"))
		convertchars (*ttag);

	// Now, we need to strip apart the link
	*link = strstr (*link, "://") + 3;
	if (strstr (*link, "/"))
		strstr (*link, "/") [0] = '\0';

	return;
}

void linktitle (ircclient_t *cl, char *nick, char *host, char *source, char *message)
{
	char *buf, *link, *ttag;
	CURL *c;

	if (source [0] != '#')
		return; // Do not reply to links through private message

	buf = alloca (strlen (message) + 1);
	strncpy (buf, message, strlen (message) + 1); // We don't want to mess up the original

	while (strstr (buf, "http://") || strstr (buf, "https://"))
	{
		// Allocate head and body buffers
		head.data = malloc (1);
		body.data = malloc (1);
		head.len = body.len = 0;

		if (!(link = strstr (buf, "http://"))) // see if we need https
			link = strstr (buf, "https://");

		if (strstr (link, " "))
			strstr (link, " ") [0] = '\0'; // terminate the string at the end of the link

		// Get HEAD of the http request
		c = curl_easy_init ();
		curl_easy_setopt (c, CURLOPT_URL, link);
		curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, writedata);
		curl_easy_setopt (c, CURLOPT_WRITEHEADER, (void*) &head);
		curl_easy_setopt (c, CURLOPT_WRITEDATA, (void*) &body);
		curl_easy_setopt (c, CURLOPT_NOBODY, 1);
		curl_easy_setopt (c, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_perform (c);

		if (!strstr (head.data, "Content-Type: text/html"))
		{
			curl_easy_cleanup (c);
			free (head.data);
			free (body.data);
			return; // According to the web server, this is not an HTML file
		}

		// Get the body of the http request
		curl_easy_setopt (c, CURLOPT_HTTPGET, 1);
		curl_easy_setopt (c, CURLOPT_WRITEHEADER, NULL);
		curl_easy_perform (c);
		curl_easy_cleanup (c);

		// Finally extract the title from the body, if possible
		ttag = strcasestr (body.data, "<title>");

		if (ttag)
		{
			get_title (&link, &ttag);
			irc_privmsg (cl, source, "link title: %s (at %s)", ttag, link);
		}

		free (head.data);
		free (body.data);

		buf = strstr (link + strlen (link) + 1, "\0") + 1;
	}

	return;
}

void init (void)
{
	curl_global_init (CURL_GLOBAL_ALL);
	module_registerfunc (&privmsgListeners, linktitle, modname);
	return;
}

void deinit (void)
{
       curl_global_cleanup ();
       return;
}
