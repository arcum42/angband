/*
 * File: gtk-drawing.h
 * Purpose: Header file for the drawing section of the GTK port for Angband
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

#ifndef GTK_DRAWING_INCLUDE
#define GTK_DRAWING_INCLUDE

#include "gtk-term.h"
extern int window_height;

extern void create_drawing_area(term_data* td);
extern void create_surface(term_data* td);
extern void resize_surface(term_data* td);
extern void get_font_size(term_data* td);
extern void write_char(term_data* td, char text, int x, int y, byte a);
extern void write_chars(term_data* td, int x, int y, int n, byte a, const char* text);
extern void draw_tiles(term_data* td, int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp);
extern void clear_surface(term_data* td);
extern void clear_chars(term_data* td, int x, int y, int n);
extern void term_redraw(term_data* td);
extern void hilite_char(term_data* td, int x, int y, byte a);

extern void init_graf(int g);
#endif
#endif
