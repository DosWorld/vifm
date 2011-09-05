/* vifm
 * Copyright (C) 2001 Ken Steen.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <curses.h>

#include <sys/stat.h>

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "color_scheme.h"
#include "completion.h"
#include "config.h"
#include "macros.h"
#include "menus.h"
#include "status.h"
#include "utils.h"

#define MAX_LEN 1024
#define MAX_COLOR_SCHEMES_CURSES (COLOR_PAIRS/MAXNUM_COLOR)

Col_scheme *col_schemes;

char *HI_GROUPS[] = {
	"Menu",
	"Border",
	"Win",
	"Status_bar",
	"CurrLine",
	"Directory",
	"Link",
	"Socket",
	"Device",
	"Executable",
	"Selected",
	"Current",
	"BrokenLink",
	"TopLine",
	"StatusLine",
	"Fifo",
	"ErrorMsg",
};

static int _gnuc_unused HI_GROUPS_size_guard[
	(ARRAY_LEN(HI_GROUPS) + 1 == MAXNUM_COLOR) ? 1 : -1
];

char *COLOR_NAMES[8] = {
	"black",
	"red",
	"green",
	"yellow",
	"blue",
	"magenta",
	"cyan",
	"white",
};

int COLOR_VALS[8] = {
	COLOR_BLACK,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_YELLOW,
	COLOR_BLUE,
	COLOR_MAGENTA,
	COLOR_CYAN,
	COLOR_WHITE,
};

static const int default_colors[][2] = {
	{ COLOR_WHITE,   COLOR_BLACK }, /* MENU_COLOR */
	{ COLOR_BLACK,   COLOR_WHITE }, /* BORDER_COLOR */
	{ COLOR_WHITE,   COLOR_BLACK }, /* WIN_COLOR */
	{ COLOR_WHITE,   COLOR_BLACK }, /* STATUS_BAR_COLOR */
	{ COLOR_WHITE,   COLOR_BLUE  }, /* CURR_LINE_COLOR */
	{ COLOR_CYAN,    COLOR_BLACK }, /* DIRECTORY_COLOR */
	{ COLOR_YELLOW,  COLOR_BLACK }, /* LINK_COLOR */
	{ COLOR_MAGENTA, COLOR_BLACK }, /* SOCKET_COLOR */
	{ COLOR_RED,     COLOR_BLACK }, /* DEVICE_COLOR */
	{ COLOR_GREEN,   COLOR_BLACK }, /* EXECUTABLE_COLOR */
	{ COLOR_MAGENTA, COLOR_BLACK }, /* SELECTED_COLOR */
	{ COLOR_BLUE,    COLOR_BLACK }, /* CURRENT_COLOR */
	{ COLOR_RED,     COLOR_BLACK }, /* BROKEN_LINK_COLOR */
	{ COLOR_BLACK,   COLOR_WHITE }, /* TOP_LINE_COLOR */
	{ COLOR_BLACK,   COLOR_WHITE }, /* STATUS_LINE_COLOR */
	{ COLOR_CYAN,    COLOR_BLACK }, /* FIFO_COLOR */
	{ COLOR_RED,     COLOR_BLACK }, /* ERROR_MSG_COLOR */
};

static int _gnuc_unused default_colors_size_guard[
	(ARRAY_LEN(default_colors) + 1 == MAXNUM_COLOR) ? 1 : -1
];

static void
init_color_scheme(Col_scheme *cs)
{
	int i;
	strcpy(cs->dir, "/");
	cs->defaulted = 0;

	for(i = 0; i < MAXNUM_COLOR; i++)
	{
		cs->color[i].fg = default_colors[i][0];
		cs->color[i].bg = default_colors[i][1];
	}
}

static void
check_color_scheme(Col_scheme *cs)
{
	int need_correction = 0;
	int i;
	for(i = 0; i < ARRAY_LEN(cs->color) - 1; i++)
	{
		if(cs->color[i].bg > COLORS || cs->color[i].fg > COLORS)
		{
			need_correction = 1;
			break;
		}
	}

	if(!need_correction)
		return;

	cs->defaulted = 1;
	for(i = 0; i < ARRAY_LEN(cs->color); i++)
	{
		cs->color[i].fg = default_colors[i][0];
		cs->color[i].bg = default_colors[i][1];
	}
}

void
check_color_schemes(void)
{
	int i;

	cfg.color_scheme_num = MIN(cfg.color_scheme_num, MAX_COLOR_SCHEMES_CURSES);

	for(i = 0; i < cfg.color_scheme_num; i++)
		check_color_scheme(col_schemes + i);
}

int
add_color_scheme(const char *name, const char *directory)
{
	if(cfg.color_scheme_num + 1 > MAX_COLOR_SCHEMES_CURSES)
	{
		(void)show_error_msg("Create Color Scheme", "Too many color schemes");
		return 1;
	}

	cfg.color_scheme_num++;
	init_color_scheme(&col_schemes[cfg.color_scheme_num - 1]);
	snprintf(col_schemes[cfg.color_scheme_num - 1].name, NAME_MAX, "%s", name);
	if(directory != NULL)
		snprintf(col_schemes[cfg.color_scheme_num - 1].dir, PATH_MAX, "%s",
				directory);
	load_color_schemes();
	return 0;
}

int
find_color_scheme(const char *name)
{
	int i;
	for(i = 0; i < cfg.color_scheme_num; i++)
	{
		if(strcmp(col_schemes[i].name, name) == 0)
			return i;
	}
	return -1;
}

/* This function is called only when colorschemes file doesn't exist */
static void
write_color_scheme_file(void)
{
	FILE *fp;
	char config_file[PATH_MAX];
	int x, y;
	char buf[128];
	char fg_buf[64];
	char bg_buf[64];

	snprintf(config_file, sizeof(config_file), "%s/colorschemes", cfg.config_dir);

	if((fp = fopen(config_file, "w")) == NULL)
		return;

	fprintf(fp, "# You can edit this file by hand.\n");
	fprintf(fp, "# The # character at the beginning of a line comments out the line.\n");
	fprintf(fp, "# Blank lines are ignored.\n\n");

	fprintf(fp, "# The Default color scheme is used for any directory that does not have\n");
	fprintf(fp, "# a specified scheme.	A color scheme set for a base directory will also\n");
	fprintf(fp, "# be used for the sub directories.\n\n");

	fprintf(fp, "# The standard ncurses colors are: \n");
	fprintf(fp, "# Default = -1 can be used for transparency\n");
	fprintf(fp, "# Black = 0\n");
	fprintf(fp, "# Red = 1\n");
	fprintf(fp, "# Green = 2\n");
	fprintf(fp, "# Yellow = 3\n");
	fprintf(fp, "# Blue = 4\n");
	fprintf(fp, "# Magenta = 5\n");
	fprintf(fp, "# Cyan = 6\n");
	fprintf(fp, "# White = 7\n\n");

	fprintf(fp, "# Vifm supports 256 colors you can use color numbers 0-255\n");
	fprintf(fp, "# (requires properly set up terminal: set your TERM environment variable\n");
	fprintf(fp, "# (directly or using resources) to some color terminal name (e.g.\n");
	fprintf(fp, "# xterm-256color) from /usr/lib/terminfo/; you can check current number\n");
	fprintf(fp, "# of colors in your terminal with tput colors command)\n\n");

	fprintf(fp, "# COLORSCHEME=OneWordDescription\n");
	fprintf(fp, "# DIRECTORY=/Full/Path/To/Base/Directory\n");
	fprintf(fp, "# COLOR=Window_name=foreground_color_number=background_color_number\n\n");

	for(x = 0; x < cfg.color_scheme_num; x++)
	{
		fprintf(fp, "\nCOLORSCHEME=%s\n", col_schemes[x].name);
		fprintf(fp, "DIRECTORY=%s\n", col_schemes[x].dir);

		for(y = 0; y < MAXNUM_COLOR; y++)
		{
			static const char *ELEM_NAMES[] = {
				"MENU",
				"BORDER",
				"WIN",
				"STATUS_BAR",
				"CURR_LINE",
				"DIRECTORY",
				"LINK",
				"SOCKET",
				"DEVICE",
				"EXECUTABLE",
				"SELECTED",
				"CURRENT",
				"BROKEN_LINK",
				"TOP_LINE",
				"STATUS_LINE",
				"FIFO",
				"ERROR_MSG",
			};

			static int _gnuc_unused ELEM_NAMES_size_guard[
				(ARRAY_LEN(ELEM_NAMES) + 1 == MAXNUM_COLOR) ? 1 : -1
			];

			static const char *COLOR_STR[] = {
				"default",
				"black",
				"red",
				"green",
				"yellow",
				"blue",
				"magenta",
				"cyan",
				"white",
			};
			int t;

			snprintf(buf, sizeof(buf), "%s", ELEM_NAMES[y]);

			t = col_schemes[x].color[y].fg + 1;
			if((size_t)t < sizeof(COLOR_STR)/sizeof(COLOR_STR[0]))
				snprintf(fg_buf, sizeof(fg_buf), "%s", COLOR_STR[t]);
			else
				snprintf(fg_buf, sizeof(fg_buf), "%d", t + 1);

			t = col_schemes[x].color[y].bg + 1;
			if((size_t)t < sizeof(COLOR_STR)/sizeof(COLOR_STR[0]))
				snprintf(bg_buf, sizeof(bg_buf), "%s", COLOR_STR[t]);
			else
				snprintf(bg_buf, sizeof(bg_buf), "%d", t + 1);

			fprintf(fp, "COLOR=%s=%s=%s\n", buf, fg_buf, bg_buf);
		}
	}

	fclose(fp);
	return;
}

static void
load_default_colors()
{
	init_color_scheme(&col_schemes[0]);

	snprintf(col_schemes[0].name, NAME_MAX, "Default");
	snprintf(col_schemes[0].dir, PATH_MAX, "/");
}

/*
 * convert possible <color_name> to <int>
 */
static int
colname2int(char col[])
{
	/* test if col[] is a number... */
	if(isdigit(col[0]))
		return atoi(col);

	/* otherwise convert */
	if(!strcasecmp(col, "black"))
		return 0;
	if(!strcasecmp(col, "red"))
		return 1;
	if(!strcasecmp(col, "green"))
		return 2;
	if(!strcasecmp(col, "yellow"))
		return 3;
	if(!strcasecmp(col, "blue"))
		return 4;
	if(!strcasecmp(col, "magenta"))
		return 5;
	if(!strcasecmp(col, "cyan"))
		return 6;
	if(!strcasecmp(col, "white"))
		return 7;
	/* return default color */
	return -1;
}

static void
add_color(char s1[], char s2[], char s3[])
{
	int fg, bg;
	const int x = cfg.color_scheme_num - 1;
	int y;

	fg = colname2int(s2);
	bg = colname2int(s3);

	if(!strcmp(s1, "MENU"))
		y = MENU_COLOR;
	else if(!strcmp(s1, "BORDER"))
		y = BORDER_COLOR;
	else if(!strcmp(s1, "WIN"))
		y = WIN_COLOR;
	else if(!strcmp(s1, "STATUS_BAR"))
		y = STATUS_BAR_COLOR;
	else if(!strcmp(s1, "CURR_LINE"))
		y = CURR_LINE_COLOR;
	else if(!strcmp(s1, "DIRECTORY"))
		y = DIRECTORY_COLOR;
	else if(!strcmp(s1, "LINK"))
		y = LINK_COLOR;
	else if(!strcmp(s1, "SOCKET"))
		y = SOCKET_COLOR;
	else if(!strcmp(s1, "DEVICE"))
		y = DEVICE_COLOR;
	else if(!strcmp(s1, "EXECUTABLE"))
		y = EXECUTABLE_COLOR;
	else if(!strcmp(s1, "SELECTED"))
		y = SELECTED_COLOR;
	else if(!strcmp(s1, "CURRENT"))
		y = CURRENT_COLOR;
	else if(!strcmp(s1, "BROKEN_LINK"))
		y = BROKEN_LINK_COLOR;
	else if(!strcmp(s1, "TOP_LINE"))
		y = TOP_LINE_COLOR;
	else if(!strcmp(s1, "STATUS_LINE"))
		y = STATUS_LINE_COLOR;
	else if(!strcmp(s1, "FIFO"))
		y = FIFO_COLOR;
	else if(!strcmp(s1, "ERROR_MSG"))
		y = ERROR_MSG_COLOR;
	else
		return;

	col_schemes[x].color[y].fg = fg;
	col_schemes[x].color[y].bg = bg;
}

void
read_color_scheme_file(void)
{
	FILE *fp;
	char config_file[PATH_MAX];
	char line[MAX_LEN];
	char *s1 = NULL;
	char *s2 = NULL;
	char *s3 = NULL;
	char *sx = NULL;

	snprintf(config_file, sizeof(config_file), "%s/colorschemes", cfg.config_dir);

	if((fp = fopen(config_file, "r")) == NULL)
	{
		load_default_colors();
		cfg.color_scheme_num = 1;

		write_color_scheme_file();
		return;
	}

	while(fgets(line, MAX_LEN, fp))
	{
		int args;

		if(line[0] == '#')
			continue;

		if((sx = s1 = strchr(line, '=')) != NULL)
		{
			s1++;
			chomp(s1);
			*sx = '\0';
			args = 1;
		}
		else
			continue;
		if((sx = s2 = strchr(s1, '=')) != NULL)
		{
			s2++;
			chomp(s2);
			*sx = '\0';
			args = 2;
		}
		if((args == 2) && ((sx = s3 = strchr(s2, '=')) != NULL))
		{
			s3++;
			chomp(s3);
			*sx = '\0';
			args = 3;
		}

		if(args == 1)
		{
			if(!strcmp(line, "COLORSCHEME"))
			{
				cfg.color_scheme_num++;

				if(cfg.color_scheme_num > MAX_COLOR_SCHEMES)
					break;

				init_color_scheme(&col_schemes[cfg.color_scheme_num - 1]);

				snprintf(col_schemes[cfg.color_scheme_num - 1].name, NAME_MAX, "%s",
						s1);

				continue;
			}
			if(!strcmp(line, "DIRECTORY"))
			{
				Col_scheme* cs;

				cs = col_schemes + cfg.color_scheme_num - 1;
				snprintf(cs->dir, PATH_MAX, "%s", s1);
				if(!is_root_dir(cs->dir))
					chosp(cs->dir);
				continue;
			}
		}
		if(!strcmp(line, "COLOR") && args == 3)
			add_color(s1, s2, s3);
	}

	fclose(fp);
}

void
load_color_schemes(void)
{
	int i;

	for(i = 0; i < cfg.color_scheme_num; i++)
	{
		int x;
		for(x = 0; x < MAXNUM_COLOR; x++)
		{
			if(x == MENU_COLOR)
				init_pair(1 + i*MAXNUM_COLOR + x, col_schemes[i].color[x].bg,
						col_schemes[i].color[x].fg);
			else
				init_pair(1 + i*MAXNUM_COLOR + x, col_schemes[i].color[x].fg,
						col_schemes[i].color[x].bg);
		}
	}
}

/* The return value is the color scheme base number for the colorpairs.
 * There are 12 color pairs for each color scheme.
 *
 * Default returns 0;
 * Second color scheme returns 12
 * Third color scheme returns 24
 *
 * The color scheme with the longest matching directory path is the one that
 * should be returned.
 */
int
check_directory_for_color_scheme(const char *dir)
{
	int i;
	int max_len = 0;
	int max_index = -1;

	for(i = 0; i < cfg.color_scheme_num; i++)
	{
		size_t len = strlen(col_schemes[i].dir);

		if(path_starts_with(dir, col_schemes[i].dir))
		{
			if(len > max_len)
			{
				max_len = len;
				max_index = i;
			}
		}
	}

	if(path_starts_with(dir, col_schemes[cfg.color_scheme_cur].dir) &&
			(max_len == strlen(col_schemes[cfg.color_scheme_cur].dir)))
		return cfg.color_scheme;

	if(max_index == -1)
		return 1 + cfg.color_scheme_cur*MAXNUM_COLOR;

	return 1 + max_index*MAXNUM_COLOR;
}

void
complete_colorschemes(const char *name)
{
	size_t len;
	int i;

	len = strlen(name);

	for(i = 0; i < cfg.color_scheme_num; i++)
	{
		if(strncmp(name, col_schemes[i].name, len) == 0)
			add_completion(col_schemes[i].name);
	}
	completion_group_end();
	add_completion(name);
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
