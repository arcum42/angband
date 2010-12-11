/*
 * File: main-gtk.h
 * Purpose: Header file for the GTK port for Angband
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

#pragma once

#include "angband.h"

#ifdef USE_GTK_NEW

// Include our standard gtk calls, and all the key symbols needed.
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// Include Cairo and Pango.
#include "cairo.h"
#include <pango/pangocairo.h>

// Is the game in progress?
extern bool game_in_progress;

// Include the term structures here, so everything has access to them.

/*
 * Extra data to associate with each "window"
 *
 * Each "window" is represented by a "term_data" structure, which
 * contains a "term" structure, which contains a pointer (t->data)
 * back to the term_data structure.
 */

typedef struct font_info font_info;

struct font_info
{
	char name[256];
	uint w, h;
};

typedef struct gtk_window_info gtk_window_info;

struct gtk_window_info
{
	uint w, h;
	uint x, y;
};

typedef struct gtk_signals gtk_signals;

struct gtk_signals
{
	uint expose_evt;
	uint delete_evt;
	uint btn_evt;
	uint key_evt;
};

typedef struct term_data term_data;

struct term_data
{
	term t;
	uint id;
	
	bool visible;
	font_info font;
	
	gtk_window_info win;
	gtk_signals signals;
	
	cairo_surface_t* surface;
	
	GtkWindow* window;
	GtkDrawingArea* drawing;
	GtkWidget* menu_item;
	GtkWidget* toolbar;
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
#define MAX_GTK_NEW_TERM 8

/*
 * An array of "term_data" structures, one for each "sub-window"
 */
extern term_data term_window[MAX_GTK_NEW_TERM];

#endif
