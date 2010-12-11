/*
 * File: gtk-windows.c
 * Purpose: Contains the windows code for the Gtk port.
 *
 * Copyright (c) 2010 Shanoah Alkire.
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

#include "gtk-term.h"
#include "gtk-windows.h"

void set_term_visible(term_data* td, bool visible)
{
	td->visible = visible;
	if (td->menu_item != NULL) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(td->menu_item), td->visible);
	
	if (visible)
		gtk_widget_show_all(GTK_WIDGET(td->window));
	else
		gtk_widget_hide_all(GTK_WIDGET(td->window));	
}

term_data* get_term(GtkWidget* widget)
{
	for(int i = 1; i < MAX_GTK_NEW_TERM; i++)
	{
		term_data* td = &term_window[i];
		if (td->window == GTK_WINDOW(widget))  return td;
	}
	return NULL;
}

gboolean hide_window(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	set_term_visible(get_term(widget), false);
	return true;
}

gboolean show_window(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	set_term_visible(get_term(widget), true);
	return true;
}

void create_drawing_area(term_data* td)
{
	GtkWidget* widget;
	
	widget = gtk_drawing_area_new();
	td->drawing = GTK_DRAWING_AREA(widget);
	
	gtk_widget_set_size_request(GTK_WIDGET(td->drawing), td->win.w, td->win.h);
	gtk_widget_add_events(GTK_WIDGET(td->drawing), GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	
	td->signals.expose_evt = g_signal_connect(G_OBJECT (td->drawing), "expose_event", G_CALLBACK (expose_drawing), td->surface);
	td->signals.btn_evt = g_signal_connect(td->drawing, "button_release_event",  G_CALLBACK (on_mouse_click), NULL);
}

void create_window(term_data *td)
{
	GtkWidget* widget, *box;
	
	widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	td->window = GTK_WINDOW(widget);
	box = gtk_vbox_new(false, 0);
	gtk_container_add(GTK_CONTAINER (td->window), GTK_WIDGET(box));
	
	if (td->id == 0)
	{
		td->visible = true;
		gtk_window_set_title(td->window, "Angband");
		td->signals.delete_evt = g_signal_connect(td->window, "delete-event", G_CALLBACK (quit_gtk), NULL);
		gtk_container_add(GTK_CONTAINER (box), GTK_WIDGET(create_menus()));
	}
	else
	{
		char title[10];
		strnfmt(title, sizeof(title), "Term %d", td->id);
		gtk_window_set_title(td->window, title);
		td->signals.delete_evt = g_signal_connect(td->window, "delete-event", G_CALLBACK (hide_window), NULL);
	}
	
	td->signals.key_evt = g_signal_connect(td->window, "key_press_event",  G_CALLBACK (keypress_event_handler), NULL);
	gtk_window_set_resizable(td->window, false);
	create_drawing_area(td);
	gtk_container_add(GTK_CONTAINER (box),GTK_WIDGET(td->drawing));

	set_term_visible(td, td->visible);
}

void delete_window(term_data *td)
{
	td->visible = false;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(td->menu_item), td->visible);
	gtk_widget_destroy(GTK_WIDGET(td->window));
}

#endif
