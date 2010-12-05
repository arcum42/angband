/*
 * File: gtk-drawing.c
 * Purpose: Contains all the drawing code, using cairo and pango.
 *
 * Copyright (c) 2010 Shanoah Alkire
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

cairo_surface_t* graphical_tiles;
cairo_pattern_t* tile_pattern;
double tile_w, tile_h;
cairo_matrix_t scale;

cairo_matrix_t cairo_font_scaling(cairo_surface_t *surface, double font_w, double font_h)
{
	cairo_t *cr;
	cairo_matrix_t m;
	double sx, sy;

	cr = cairo_create(surface);
	
	/* Get a matrix set up to scale the graphics. */
	cairo_get_matrix(cr, &m);
	sx = (tile_w)/(font_w);
	sy = (tile_h)/(font_h);
	cairo_matrix_scale(&m, sx, sy);
	
	cairo_destroy(cr);
	
	return(m);
}

void init_graf(int g)
{
	char buf[1024];
	term_data* td = &term_window[0];
	
	arg_graphics = g;
	
	switch(arg_graphics)
	{
		case GRAPHICS_NONE:
		{
			ANGBAND_GRAF = "none";
			use_transparency = false;
			tile_w = tile_h = 0;
			printf("Graphics: None\n");
			break;
		}

		case GRAPHICS_ORIGINAL:
		{
			ANGBAND_GRAF = "old";
			path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, "8x8.png");
			use_transparency = false;
			tile_w = tile_h = 8;
			printf("Graphics: Original\n");
			break;
		}

		case GRAPHICS_ADAM_BOLT:
		{
			ANGBAND_GRAF = "new";
			path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, "16x16.png");
			use_transparency = true;
			tile_w = tile_h = 16;
			printf("Graphics: Adam Bolt\n");
			break;
		}

		case GRAPHICS_DAVID_GERVAIS:
		{
			ANGBAND_GRAF = "david";
			path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, "32x32.png");
			use_transparency = false;
			tile_w = tile_h = 32;
			printf("Graphics: David Gervais\n");
			break;
		}
		/*case GRAPHICS_NOMAD:	
		{
			ANGBAND_GRAF = "nomad";
			path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, "8x16.png");
			use_transparency = TRUE;
			tile_w = tile_h = 16;
			break;
			
		}*/
	}

	/* Free up old graphics */
	if (graphical_tiles != NULL) cairo_surface_destroy(graphical_tiles);
	if (tile_pattern != NULL) cairo_pattern_destroy(tile_pattern);
		
	graphical_tiles = cairo_image_surface_create_from_png(buf);
	tile_pattern = cairo_pattern_create_for_surface(graphical_tiles);
	
	g_assert(graphical_tiles != NULL);
	g_assert(tile_pattern != NULL);
	scale = cairo_font_scaling(td->surface, td->font_w, td->font_h);
	force_redraw();
}

void set_foreground_color(cairo_t *cr, byte a)
{
	cairo_set_source_rgb(cr, 
	(double)angband_color_table[a][1] / 256, 
	(double)angband_color_table[a][2] / 256, 
	(double)angband_color_table[a][3] / 256);
}

void term_redraw(term_data* td)
{
	if (td->drawing != NULL) gtk_widget_queue_draw(GTK_WIDGET(td->drawing));
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

void draw_tile(term_data* td, int x, int y, int tx, int ty)
{
	cairo_t* cr;
	cr = cairo_create(td->surface);
	
	cairo_rectangle(cr, x * td->font_w, y * td->font_h, td->font_w, td->font_h);
	cairo_set_source (cr, tile_pattern);
	cairo_surface_set_device_offset(graphical_tiles, tx - (x * td->font_w), ty - (y * td->font_h));
	
	// Use transparency.
	cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
	
	/* Use the matrix with our pattern */
	cairo_pattern_set_matrix(tile_pattern, &scale);
	
	/* Draw it */
	cairo_fill(cr);
	
}

void draw_tiles(term_data* td, int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
{
	cairo_t *cr;
	
	/* Tile & Current Position */
	int tx, ty;
	int cx, cy;
	
	cr = cairo_create(td->surface);
	
	cairo_rectangle(cr, x * td->font_w, y * td->font_h, td->font_w * n, td->font_h);
	set_foreground_color(cr, TERM_DARK);
	cairo_fill(cr);
	
	/* Get the current position, Minus cx, which changes for each iteration */
	cx = 0;
	cy = (y * td->font_h);
	
	for (int i = 0; i < n; i++)
	{
		/* Increment x 1 step; use the font width because of equippy chars and the gap between 
   		 * the status bar and the map.
		 */
		cx += x * td->font_w;
		
		/* Get the terrain tile, scaled to the font size */
		tx= (tcp[i] & 0x7F) * td->font_w;
		ty = (tap[i] & 0x7F) * td->font_h;
		
		draw_tile(td, x + i, y, tx, ty);
	
		/* If foreground is the same as background, we're done */
		if ((tap[i] == ap[i]) && (tcp[i] == cp[i])) continue;
		
		/* Get the foreground tile size, scaled to the font size */
		tx = (cp[i] & 0x7F) * td->font_w;
		ty = (ap[i] & 0x7F) * td->font_h;
	
		draw_tile(td, x + i, y, tx, ty);
	}
	
	cairo_destroy(cr);
	
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
