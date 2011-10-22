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
#include "grafmode.h"
#include <pango/pangocairo.h>

cairo_surface_t* graphical_tiles;
cairo_pattern_t* tile_pattern;
double tile_w, tile_h;
cairo_matrix_t scale;

cairo_matrix_t cairo_font_scaling(term_data* td)
{
	cairo_t *cr;
	cairo_matrix_t m;
	double sx, sy;

	cr = cairo_create(td->surface);
	
	/* Get a matrix set up to scale the graphics. */
	cairo_get_matrix(cr, &m);
	sx = (tile_w)/(td->font.w);
	sy = (tile_h)/(td->font.h);
	cairo_matrix_scale(&m, sx, sy);
	cairo_destroy(cr);
	
	return(m);
}
	
void set_graphics(int g)
{
	char buf[1024];
	term_data* td = &term_window[0];
	int i = 0;
	
	do 
	{
		if (g == graphics_modes[i].grafID) 
		{
			arg_graphics = g;
			ANGBAND_GRAF = graphics_modes[i].pref;
			path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, graphics_modes[i].file);
			use_transparency = FALSE;
			tile_w = graphics_modes[i].cell_width;
			tile_h = graphics_modes[i].cell_height;
			break;
		}
	} while (graphics_modes[i++].grafID != 0);

	/* Free up old graphics */
	if (graphical_tiles != NULL) cairo_surface_destroy(graphical_tiles);
	if (tile_pattern != NULL) cairo_pattern_destroy(tile_pattern);
		
	graphical_tiles = cairo_image_surface_create_from_png(buf);
	tile_pattern = cairo_pattern_create_for_surface(graphical_tiles);
	
	g_assert(graphical_tiles != NULL);
	g_assert(tile_pattern != NULL);
	scale = cairo_font_scaling(td);
	cairo_pattern_set_matrix(tile_pattern, &scale);
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
	temp_font = pango_font_description_from_string(td->font.name);
	pango_layout_set_font_description(temp, temp_font);
	pango_layout_set_text(temp, "@", 1);
	pango_cairo_show_layout(cr, temp);
	pango_layout_get_pixel_extents(temp, NULL, &r);
	
	td->font.w = r.width;
	td->font.h = r.height;
	
	pango_font_description_free(temp_font);
	cairo_destroy(cr);
	g_object_unref(temp);
	
	td->win.w = td->font.w * td->cols;
	td->win.h = td->font.h * td->rows;
//	printf("font width == %d, height = %d.\n", td->font.w, td->font.h);
}

void clear_chars(term_data* td, int x, int y, int n)
{
	cairo_t* cr;
	cr = cairo_create(td->surface);
	set_foreground_color(cr, TERM_DARK);
	cairo_rectangle(cr, x * td->font.w, y * td->font.h, n * td->font.w, td->font.h);
	cairo_fill(cr);
	cairo_destroy(cr);
}

void color_chars(term_data* td, int x, int y, int n, byte a, bool trans)
{
	cairo_t* cr;
	cr = cairo_create(td->surface);
	set_foreground_color(cr, a);
	cairo_rectangle(cr, x * td->font.w, y * td->font.h, n * td->font.w, td->font.h);
	if (trans) cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
	cairo_fill(cr);
	cairo_destroy(cr);
}

void draw_tile(term_data* td, int x, int y, int tx, int ty, bool trans)
{
	cairo_t* cr;
	cr = cairo_create(td->surface);
	
	cairo_rectangle(cr, x * td->font.w, y * td->font.h, td->font.w, td->font.h);
	cairo_set_source(cr, tile_pattern);
	cairo_surface_set_device_offset(graphical_tiles, (tx - x) * td->font.w, (ty - y) * td->font.h);
	
	// Use transparency.
	if (trans) cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
	
	/* Draw it */
	cairo_fill(cr);
}

void draw_tiles(term_data* td, int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
{
	clear_chars(td, x, y, n);
	//color_chars(td, x, y, n, TERM_SLATE, false);
	
	for (int i = 0; i < n; i++)
	{
		/* Draw the terrain, scaled to the font size */
		draw_tile(td, x + i, y, tcp[i] & 0x7f, tap[i] & 0x7f, false);
	
		/* If foreground is the same as background, we're done */
		if ((tap[i] == ap[i]) && (tcp[i] == cp[i])) continue;
		
		/* Draw the foreground tile, scaled to the font size */
		draw_tile(td, x + i, y, cp[i] & 0x7f, ap[i] & 0x7f, true);
	}
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
	font = pango_font_description_from_string(td->font.name);
	pango_layout_set_font_description (layout, font);
	pango_layout_set_text (layout, text, n);
	
	cairo_move_to(cr, x * td->font.w, y * td->font.h);
	pango_cairo_show_layout (cr, layout);

	g_object_unref (layout);
	pango_font_description_free(font);
	cairo_destroy(cr);
}

void clear_surface(term_data* td)
{
	cairo_t* cr;
	cr = cairo_create(td->surface);
	set_foreground_color(cr, TERM_DARK);
	cairo_paint(cr);
	cairo_destroy(cr);
}

void create_surface(term_data* td)
{
	if (td->surface != NULL) cairo_surface_destroy(td->surface);
	td->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, td->win.w, td->win.h);
	clear_surface(td);
}

#endif
