/*
 * File: main-gtk.h
 * Purpose: Header file for the GTK port for Angband
 *
 * Copyright (c) 2000-2007 Robert Ruehlmann, Shanoah Alkire
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


#ifdef USE_GTK_NEW

#include "angband.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

extern gboolean quit_gtk(GtkWidget *widget, GdkEvent *event, gpointer user_data);

#endif
