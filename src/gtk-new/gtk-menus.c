/*
 * File: gtk-menus.c
 * Purpose: Contains the menu code for the Gtk port.
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
#include "gtk-menus.h"

extern void set_graphics(int g);

gboolean change_graphics(GtkWidget *widget, int user_data)
{
	printf("Change graphics to %d\n", user_data);
	set_graphics(user_data);
	return true;
}

term_data* gtk_checkbox_to_term(gchar* user_data)
{
	int i = -1;
	
	sscanf((char*)user_data, "Window %d", &i);
	if (i == -1)
		return NULL;
	else
		return &term_window[i];
}

gboolean toggle_window(GtkWidget *widget, gchar* user_data)
{
	term_data* td;
	bool is_visible = false;
	
	td = gtk_checkbox_to_term(user_data);
	is_visible = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
	if (td != 0) set_term_visible(td, is_visible);
	return false;
}

GtkWidget* create_file_menu()
{
	GtkWidget* file_menu, *file_item, *new_item, *open_item, *save_item, *quit_item;
		
	file_menu = gtk_menu_new();
	file_item = gtk_menu_item_new_with_label("File");
	
	new_item = gtk_menu_item_new_with_label("New");
	gtk_menu_append(GTK_MENU(file_menu), new_item);
	g_signal_connect(GTK_OBJECT(new_item), "activate", G_CALLBACK(new_gtk_game), NULL);
	
	open_item = gtk_menu_item_new_with_label("Open");
	gtk_menu_append(GTK_MENU(file_menu), open_item);
	g_signal_connect(GTK_OBJECT(open_item), "activate", G_CALLBACK(open_gtk_game), NULL);
	
	save_item = gtk_menu_item_new_with_label("Save");
	gtk_menu_append(GTK_MENU(file_menu), save_item);
	g_signal_connect(GTK_OBJECT(save_item), "activate", G_CALLBACK(save_gtk_game), NULL);
	
	quit_item = gtk_menu_item_new_with_label("Quit");
	gtk_menu_append(GTK_MENU(file_menu), quit_item);
	g_signal_connect(GTK_OBJECT(quit_item), "activate", G_CALLBACK(quit_gtk), NULL);
	
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(file_item), file_menu);
	return file_item;
}

GtkWidget* create_window_menu()
{
	GtkWidget* window_menu, *window_item;
	//GtkWidget *font_item;
	
	window_menu = gtk_menu_new();
	window_item = gtk_menu_item_new_with_label("Window");
	
	for(int i = 1; i < MAX_GTK_NEW_TERM; i++)
	{
		char title[10];
		term_data* td =  &term_window[i];
			
		sprintf (title, "Window %d", i);
		 td->menu_item = gtk_check_menu_item_new_with_label(title);
		gtk_menu_append(GTK_MENU(window_menu),  td->menu_item);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(td->menu_item), td->visible);
		g_signal_connect(GTK_OBJECT( td->menu_item), "activate", G_CALLBACK(toggle_window), (gpointer) g_strdup ((gchar*)&title));
	}
	
	/*font_item = gtk_menu_item_new_with_label("Pick Font...");
	gtk_menu_append(GTK_MENU(window_menu), font_item);
	g_signal_connect(GTK_OBJECT(font_item), "activate", G_CALLBACK(pick_font), NULL);*/
	
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(window_item), window_menu);
	
	return window_item;
}

GtkWidget* create_graphics_menu()
{
	GtkWidget* graf_menu, *graf_item, *graf1_item, *graf2_item, *graf3_item, *graf4_item, *graf5_item;
	
	graf_menu = gtk_menu_new();
	graf_item = gtk_menu_item_new_with_label("Graphics");
	
	graf1_item = gtk_menu_item_new_with_label("No Graphics");
	gtk_menu_append(GTK_MENU(graf_menu), graf1_item);
	g_signal_connect(GTK_OBJECT(graf1_item), "activate", G_CALLBACK(change_graphics), (gpointer)GRAPHICS_NONE);
	
	graf2_item = gtk_menu_item_new_with_label("Original");
	gtk_menu_append(GTK_MENU(graf_menu), graf2_item);
	g_signal_connect(GTK_OBJECT(graf2_item), "activate", G_CALLBACK(change_graphics), (gpointer)GRAPHICS_ORIGINAL);
	
	graf3_item = gtk_menu_item_new_with_label("Adam Bolt");
	gtk_menu_append(GTK_MENU(graf_menu), graf3_item);
	g_signal_connect(GTK_OBJECT(graf3_item), "activate", G_CALLBACK(change_graphics), (gpointer)GRAPHICS_ADAM_BOLT);
	
	graf4_item = gtk_menu_item_new_with_label("David Gervais");
	gtk_menu_append(GTK_MENU(graf_menu), graf4_item);
	g_signal_connect(GTK_OBJECT(graf4_item), "activate", G_CALLBACK(change_graphics), (gpointer)GRAPHICS_DAVID_GERVAIS);
	
	graf5_item = gtk_menu_item_new_with_label("Nomad");
	gtk_menu_append(GTK_MENU(graf_menu), graf5_item);
	g_signal_connect(GTK_OBJECT(graf5_item), "activate", G_CALLBACK(change_graphics), (gpointer)GRAPHICS_NOMAD);
	
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(graf_item), graf_menu);
	return graf_item;
}

GtkWidget* create_menus()
{
	GtkWidget* menubar;
	
	menubar = gtk_menu_bar_new();
	
	gtk_menu_bar_append( GTK_MENU_BAR (menubar), create_file_menu());
	gtk_menu_bar_append( GTK_MENU_BAR (menubar), create_window_menu());
	gtk_menu_bar_append( GTK_MENU_BAR (menubar), create_graphics_menu());
	return menubar;
}

#endif
