/*
 * File: main-windows.h
 * Purpose: Header file for the windows code for the GTK port for Angband
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

#include "gtk-common.h"

extern gboolean close_window(GtkWidget *widget, GdkEvent *event, gpointer user_data);
extern gboolean hide_window(GtkWidget *widget, GdkEvent *event, gpointer user_data);
extern gboolean show_window(GtkWidget *widget, GdkEvent *event, gpointer user_data);

extern void set_term_visible(term_data* td, bool visible);

extern gboolean expose_drawing(GtkWidget *widget, GdkEventExpose *event, cairo_surface_t* surface);

extern GtkWidget* create_menus();

#endif
