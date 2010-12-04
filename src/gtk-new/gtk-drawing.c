/*
 * File: main-xxx.c
 * Purpose: Outline how to make a new "main-xxx" file.
 *
 * Copyright (c) 1997 Ben Harrison
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
#include "gtk-drawing.h"

#ifdef USE_GTK_NEW 

#include "gtk-term.h"
#include "cairo.h"


gboolean expose_drawing(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	  cairo_t *cr;
	  cr = gdk_cairo_create(event->window);
	  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	  cairo_paint(cr);
	  cairo_destroy(cr);
	return true;
}

void create_drawing_area(term_data* td)
{
	GtkWidget* widget;
	
	widget = gtk_drawing_area_new();
	td->drawing = GTK_DRAWING_AREA(widget);
	gtk_container_add(GTK_CONTAINER (td->window), GTK_WIDGET(td->drawing));
	
	gtk_widget_set_size_request (GTK_WIDGET(td->drawing), 100, 100);
	g_signal_connect (G_OBJECT (td->drawing), "expose_event", G_CALLBACK (expose_drawing), NULL);
}

#endif
