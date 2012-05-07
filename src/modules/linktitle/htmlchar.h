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

#ifndef HTMLCHAR_H__
#define HTMLCHAR_H__

typedef struct
{
	const char *code;
	const char *ch;
} html_char_t;

html_char_t htmlchar [] =
{
	{ "&quot;", "\"" },
	{ "&amp;", "&" },
	{ "&bull;", "•" },
	{ "&mdash;", "-" },
	{ "&#39;", "'" },
	{ "&lt;", "<" },
	{ "&gt;", ">" },
	{ "&nbsp;", " " },
	{ "&iexcl;", "¡" },
	{ "&cent;", "¢" },
	{ "&pound;", "£" },
	{ "&curren;", "¤" },
	{ "&yen;", "¥" },
	{ "&euro;", "€" },
	{ "&brvbar;", "¦" },
	{ "&sect;", "§" },
	{ "&uml;", "¨" },
	{ "&copy;", "©" },
	{ "&ordf;", "ª" },
	{ "&laquo;", "«" },
	{ "&not;", "¬" },
	{ "&shy;", "-" },
	{ "&reg;", "®" },
	{ "&macr;", "¯" },
	{ "&deg;", "°" },
	{ "&plusmn;", "±" },
	{ "&sup1;", "¹" },
	{ "&sup2;", "²" },
	{ "&sup3;", "³" },
	{ "&acute;", "´" },
	{ "&micro;", "µ" },
	{ "&para;", "¶" },
	{ "&middot;", "·" },
	{ "&cedil;", "¸" },
	{ "&ordm;", "º" },
	{ "&raquo;", "»" },
	{ "&frac14;", "¼" },
	{ "&frac12;", "½" },
	{ "&frac34;", "¾" },
	{ "&iquest;", "¿" },
	{ NULL, "\0" }
};

#endif // HTMLCHAR_H__
