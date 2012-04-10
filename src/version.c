/* vifm
 * Copyright (C) 2011 xaizek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <assert.h> /* assert() */
#include <stddef.h> /* NULL */
#include <stdio.h> /* sprintf() */
#include <stdlib.h> /* malloc() */
#include <string.h> /* strlen() strdup() */

#include "../config.h"

/* This hash is automatically updated by make. */
static const char GIT_HASH[] = "377ac648bdbaa2bc0d510d871a5b37718f7dbae5";

/* When list is NULL returns maximum number of lines, otherwise returns number
 * of filled lines */
int
fill_version_info(char **list)
{
	const int LEN = 12;
	int x = 0;

	if(list == NULL)
		return LEN;

	list[x++] = strdup("Version: " VERSION);
	list[x] = malloc(sizeof("Git commit hash: ") + strlen(GIT_HASH) + 1);
	sprintf(list[x++], "Git commit hash: %s", GIT_HASH);
	list[x++] = strdup("Compiled at: " __DATE__ " " __TIME__);
	list[x++] = strdup("");

#ifdef ENABLE_COMPATIBILITY_MODE
	list[x++] = strdup("Compatibility mode is on");
#else
	list[x++] = strdup("Compatibility mode is off");
#endif

#ifdef ENABLE_EXTENDED_KEYS
	list[x++] = strdup("Support of extended keys is on");
#else
	list[x++] = strdup("Support of extended keys is off");
#endif

#ifdef ENABLE_DESKTOP_FILES
	list[x++] = strdup("Parsing of .desktop files is enabled");
#else
	list[x++] = strdup("Parsing of .desktop files is disabled");
#endif

#ifdef HAVE_LIBGTK
	list[x++] = strdup("With GTK+ library");
#else
	list[x++] = strdup("Without GTK+ library");
#endif

#ifdef HAVE_LIBMAGIC
	list[x++] = strdup("With magic library");
#else
	list[x++] = strdup("Without magic library");
#endif

#ifdef HAVE_X11
	list[x++] = strdup("With X11 library");
#else
	list[x++] = strdup("Without X11 library");
#endif

#ifdef HAVE_FILE_PROG
	list[x++] = strdup("With file program");
#else
	list[x++] = strdup("Without file program");
#endif

#ifdef SUPPORT_NO_CLOBBER
	list[x++] = strdup("With -n option for cp and mv");
#else
#ifndef _WIN32
	list[x++] = strdup("With -n option for cp and mv");
#endif /* _WIN32 */
#endif

	assert(x == LEN);

	return x;
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */