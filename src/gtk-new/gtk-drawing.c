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
#include <pango/pangocairo.h>

void set_foreground_color(cairo_t *cr, byte a)
{
	cairo_set_source_rgb(cr, 
	(double)angband_color_table[a][1] / 256, 
	(double)angband_color_table[a][2] / 256, 
	(double)angband_color_table[a][3] / 256);
}

void term_redraw(term_data* td)
{
	gtk_widget_queue_draw(GTK_WIDGET(td->drawing));
	gdk_flush();
}

gboolean expose_drawing(GtkWidget *widget, GdkEventExpose *event, cairo_surface_t* surface)
{
	cairo_t *cr;
	cr = gdk_cairo_create(event->window);
	cairo_set_source_surface(cr, surface, 0, 0);
	cairo_paint(cr);
	cairo_destroy(cr);
	return true;
}

void get_font_size(term_data* td)
{
	PangoRectangle r;
	PangoLayout *temp;
	PangoFontDescription* temp_font;
	cairo_t *cr;
	cairo_surface_t *surface;
	
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 200);
	cr = cairo_create(surface);
	
	temp = pango_cairo_create_layout(cr);
	
	/* Draw an @, and measure it */
	temp_font = pango_font_description_from_string(td->font);
	pango_layout_set_font_description(temp, temp_font);
	pango_layout_set_text(temp, "@", 1);
	pango_cairo_show_layout(cr, temp);
	pango_layout_get_pixel_extents(temp, NULL, &r);
	
	td->font_w = r.width;
	td->font_h = r.height;
	
	pango_font_description_free(temp_font);
	cairo_destroy(cr);
	g_object_unref(temp);
	
	td->window_w = td->font_w * 80;
	td->window_h = td->font_h * 24;
}

void create_font(term_data* td)
{
	get_font_size(td);
}

void hilite_char(term_data* td, int x, int y, byte a)
{
	cairo_t* cr;
	cr = cairo_create(td->surface);
	cairo_rectangle(cr, x * td->font_w, y * td->font_h, td->font_w, td->font_h);
	set_foreground_color(cr, a);
	cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
	cairo_fill(cr);
	cairo_destroy(cr);
}

void clear_chars(term_data* td, int x, int y, int n)
{
	cairo_t* cr;
	cr = cairo_create(td->surface);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_rectangle(cr, x * td->font_w, y * td->font_h, n * td->font_w, td->font_h);
	cairo_fill(cr);
	cairo_destroy(cr);
}

void write_chars(term_data* td, int x, int y, int n, byte a, const char* text)
{
	PangoLayout *layout;
	PangoFontDescription* font;
	cairo_t *cr;
	
	clear_chars(td, x, y, n);
	
	cr = cairo_create(td->surface);
	set_foreground_color(cr, a);
	
	layout = pango_cairo_create_layout(cr);
	font = pango_font_description_from_string(td->font);
	pango_layout_set_font_description (layout, font);
	pango_layout_set_text (layout, text, n);
	
	cairo_move_to(cr, x * td->font_w, y * td->font_h);
	pango_cairo_show_layout (cr, layout);

	g_object_unref (layout);
	pango_font_description_free(font);
	cairo_destroy(cr);
}

void clear_surface(term_data* td)
{
	cairo_t* cr;
	cr = cairo_create(td->surface);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_paint(cr);
	cairo_destroy(cr);
}

void create_surface(term_data* td)
{
	cairo_t* cr;
	td->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, td->window_w, td->window_h);
	cr = cairo_create(td->surface);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_paint(cr);
	cairo_destroy(cr);
}

void create_drawing_area(term_data* td)
{
	GtkWidget* widget;
	
	widget = gtk_drawing_area_new();
	td->drawing = GTK_DRAWING_AREA(widget);
	
	gtk_widget_set_size_request(GTK_WIDGET(td->drawing), td->window_w, td->window_h);
	g_signal_connect(G_OBJECT (td->drawing), "expose_event", G_CALLBACK (expose_drawing), td->surface);
}


#endif
