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

#include "angband.h"

#ifdef USE_GTK_NEW

#ifndef MAIN_GTK_INCLUDE
#define MAIN_GTK_INCLUDE

#include "gtk-common.h"

// Include our standard gtk calls, and all the key symbols needed.
//#include <gtk/gtk.h>
//#include <gdk/gdkkeysyms.h>

extern gboolean quit_gtk(GtkWidget *widget, GdkEvent *event, gpointer user_data);
extern errr CheckEvent(bool wait);
extern gboolean keypress_event_handler(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
extern gboolean on_mouse_click(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

extern gboolean new_gtk_game(GtkWidget *widget, GdkEvent *event, gpointer user_data);
extern gboolean open_gtk_game(GtkWidget *widget, GdkEvent *event, gpointer user_data);
extern gboolean save_gtk_game(GtkWidget *widget, GdkEvent *event, gpointer user_data);

#endif
#endif
