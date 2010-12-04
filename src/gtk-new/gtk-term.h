/*
 * File: gtk-term.h
 * Purpose: Header file for the term code of the GTK port for Angband
 *
 * Copyright (c) 2000-2010 Shanoah Alkire
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
 
#include "angband.h"

#ifdef USE_GTK_NEW
#ifndef GTK_TERM_INCLUDE

#define GTK_TERM_INCLUDE
#include "main-gtk.h"
#include "cairo.h"
#include <pango/pangocairo.h>

/*
 * Extra data to associate with each "window"
 *
 * Each "window" is represented by a "term_data" structure, which
 * contains a "term" structure, which contains a pointer (t->data)
 * back to the term_data structure.
 */

typedef struct term_data term_data;

struct term_data
{
	term t;
	
	/* Other fields if needed XXX XXX XXX */
	GtkWindow* window;
	GtkDrawingArea* drawing;
	cairo_surface_t* surface;
	char font[20];
	int font_w, font_h;
	int window_w, window_h;
	uint id;
};

/*
 * Number of "term_data" structures to support XXX XXX XXX
 *
 * You MUST support at least one "term_data" structure, and the
 * game will currently use up to eight "term_data" structures if
 * they are available.
 *
 * Actually, MAX_TERM_DATA is now defined as eight in 'defines.h'.
 *
 * If only one "term_data" structure is supported, then a lot of
 * the things that would normally go into a "term_data" structure
 * could be made into global variables instead.
 */
#define MAX_GTK_NEW_TERM 2

/*
 * An array of "term_data" structures, one for each "sub-window"
 */
term_data term_window[MAX_GTK_NEW_TERM];

extern void term_data_link(int i);

#endif
#endif
