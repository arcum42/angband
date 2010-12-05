/*
 * File: main-gtk.c
 * Purpose: GTK port for Angband
 *
 * Copyright (c) 2000-2010 Robert Ruehlmann, Shanoah Alkire
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

#include "main-gtk.h"
#include "main.h"
#include "gtk-term.h"
#include "gtk-drawing.h"

#include "textui.h"
#include "game-event.h"
#include "macro.h"
#include "files.h"

bool game_in_progress = false;

extern void create_window(term_data *td);
extern void delete_window(term_data *td);
void pick_font();
/*
 * Help message.
 *   1st line = max 68 chars.
 *   Start next lines with 11 spaces
 */
const char help_gtk[] = "Describe XXX, subopts -describe suboptions here";


gboolean change_graphics(GtkWidget *widget, int user_data)
{
	printf("Change graphics to %d\n", user_data);
	init_graf(user_data);
	return true;
}

gboolean toggle_window(GtkWidget *widget, gchar* user_data)
{
	int i = -1;
	term_data* td;
	
	sscanf((char*)user_data, "Window %d", &i);
	td = &term_window[i];
	
	td->visible = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
	if (td != 0)
	{
		if (td->visible)
			create_window(td);
		else
			delete_window(td);
	}
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
/*** Function hooks needed by "Term" ***/

GtkWidget* create_menus()
{
	GtkWidget* menubar;
	
	menubar = gtk_menu_bar_new();
	
	gtk_menu_bar_append( GTK_MENU_BAR (menubar), create_file_menu());
	gtk_menu_bar_append( GTK_MENU_BAR (menubar), create_window_menu());
	gtk_menu_bar_append( GTK_MENU_BAR (menubar), create_graphics_menu());
	return menubar;
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
		g_signal_connect(td->window, "delete-event", G_CALLBACK (quit_gtk), NULL);
		g_signal_connect(td->window, "destroy",  G_CALLBACK (quit_gtk), NULL);
		gtk_container_add(GTK_CONTAINER (box), GTK_WIDGET(create_menus()));
	}
	else
	{
		char title[10];
		strnfmt(title, sizeof(title), "Term %d", td->id);
		gtk_window_set_title(td->window, title);
		g_signal_connect(td->window, "delete-event", G_CALLBACK (close_window), NULL);
		g_signal_connect(td->window, "destroy",  G_CALLBACK (close_window), NULL);
	}
	
	g_signal_connect(td->window, "key_press_event",  G_CALLBACK (keypress_event_handler), NULL);
	gtk_window_set_resizable(td->window, false);
	create_drawing_area(td);
	gtk_container_add(GTK_CONTAINER (box),GTK_WIDGET(td->drawing));
	
	gtk_widget_show_all(GTK_WIDGET(td->window));
}

void delete_window(term_data *td)
{
	td->visible = false;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(td->menu_item), td->visible);
	gtk_widget_destroy(GTK_WIDGET(td->window));
}

gboolean on_mouse_click(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	term_data *td = (term_data*)(Term->data);
	
	/* Where is the mouse? */
	int x = event->x / td->font_w;
	int y = event->y / td->font_h;
	int z = event->button;
	
	Term_mousepress(x, y, z);

	return false;
}

gboolean keypress_event_handler(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	int i, mc, ms, mo, mx;
	guint modifiers;

	char msg[128];

	modifiers = gtk_accelerator_get_default_mod_mask ();
	
	/* Extract four "modifier flags" */
	mc = ((event->state & modifiers) == GDK_CONTROL_MASK);
	ms = ((event->state & modifiers) == GDK_SHIFT_MASK);
	mo = ((event->state & modifiers) == GDK_MOD1_MASK);
	mx = ((event->state & modifiers) == GDK_MOD3_MASK);

	/*
	 * Hack XXX
	 * Parse shifted numeric (keypad) keys specially.
	 */
	if (ms && (event->keyval >= GDK_KP_0) && (event->keyval <= GDK_KP_9))
	{
		/* Build the macro trigger string */
		strnfmt(msg, sizeof(msg), "%cS_%X%c", 31, event->keyval, 13);

		/* Enqueue the "macro trigger" string */
		for (i = 0; msg[i]; i++) Term_keypress(msg[i]);

		/* Hack -- auto-define macros as needed */
		if (event->length && (macro_find_exact(msg) < 0))
		{
			/* Create a macro */
			macro_add(msg, event->string);
		}

		return true;
	}

	/* Normal keys with no modifiers (except control) */
	if (event->length && !mo && !mx)
	{
		/* Enqueue the normal key(s) */
		for (i = 0; i < event->length; i++) Term_keypress(event->string[i]);
		
		if (!mc)
			return true; /* Not a control key, so the keypress is handled */
		else
			return false; /* Pass the keypress along, so the menus get it */
	}


	/* Handle a few standard keys (bypass modifiers) XXX XXX XXX */
	switch (event->keyval)
	{
		case GDK_Escape:
		{
			Term_keypress(ESCAPE);
			return true;
		}

		case GDK_Return:
		{
			Term_keypress('\r');
			return true;
		}

		case GDK_Tab:
		{
			Term_keypress('\t');
			return true;
		}

		case GDK_Delete:
		case GDK_BackSpace:
		{
			Term_keypress('\010');
			return true;
		}

		case GDK_Shift_L:
		case GDK_Shift_R:
		case GDK_Control_L:
		case GDK_Control_R:
		case GDK_Caps_Lock:
		case GDK_Shift_Lock:
		case GDK_Meta_L:
		case GDK_Meta_R:
		case GDK_Alt_L:
		case GDK_Alt_R:
		case GDK_Super_L:
		case GDK_Super_R:
		case GDK_Hyper_L:
		case GDK_Hyper_R:
		{
			/* Hack - do nothing to control characters */
			return true;
		}
	}

	/* Build the macro trigger string */
	strnfmt(msg, sizeof(msg), "%c%s%s%s%s_%X%c", 31,
	        mc ? "N" : "", ms ? "S" : "",
	        mo ? "O" : "", mx ? "M" : "",
	        event->keyval, 13);

	/* Enqueue the "macro trigger" string */
	for (i = 0; msg[i]; i++) Term_keypress(msg[i]);

	/* Hack -- auto-define macros as needed */
	if (event->length && (macro_find_exact(msg) < 0))
	{
		/* Create a macro */
		macro_add(msg, event->string);
	}

	return true;
}

void pick_font()
{
	char *fontname;
	GtkWidget *dialog;
	term_data* td = &term_window[0];
	
	dialog = gtk_font_selection_dialog_new("Pick a font, preferably fixed-width.");
	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(dialog), td->font);
	gtk_font_selection_dialog_set_preview_text(GTK_FONT_SELECTION_DIALOG(dialog), "The Boil-covered wretch drools on you!");
	
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
	  {
		 fontname = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
		for(int i = 0; i < MAX_GTK_NEW_TERM; i++)
		{
			td = &term_window[i];
			//my_strcpy(td->font, fontname, sizeof(td->font));
		}
	  }
	  
	gtk_widget_destroy (dialog);
	
}

char* open_dialog_box()
{
	char *filename;
	GtkWidget *dialog;
	char buf[1024];
	
	dialog = gtk_file_chooser_dialog_new ("Open File",
					     NULL,
					      GTK_FILE_CHOOSER_ACTION_OPEN,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					      NULL);
	
	/* Get the current directory (so we can find lib/save/) */
	filename = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
	path_build(buf, sizeof buf, filename, ANGBAND_DIR_SAVE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), buf);
	
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	  {
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		gtk_widget_destroy (dialog);
		
		return filename;
	  }
	  
	gtk_widget_destroy (dialog);
	return NULL;
}

char* save_dialog_box()
{
	char *filename;
	GtkWidget *dialog;
	char buf[1024];
	
	dialog = gtk_file_chooser_dialog_new ("Save File",
					      NULL,
					      GTK_FILE_CHOOSER_ACTION_SAVE,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					      NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
	
	/* Get the current directory (so we can find lib/save/) */
	filename = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
	path_build(buf, sizeof buf, filename, ANGBAND_DIR_SAVE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), buf);
	gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), op_ptr->full_name);
	
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	  {
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		gtk_widget_destroy (dialog);
		
		return filename;
	  }
	  
	gtk_widget_destroy (dialog);
	return NULL;
}

gboolean close_window(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	for(int i = 1; i < MAX_GTK_NEW_TERM; i++)
	{
		term_data* td = &term_window[i];
		if (td->window == GTK_WINDOW(widget)) 
		{
			td->visible = false;
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(td->menu_item), td->visible);
			gtk_widget_destroy(GTK_WIDGET(td->window));
		}
	}
	return true;
}

gboolean new_gtk_game(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	cmd_insert(CMD_NEWGAME);
	game_in_progress = true;
	return false;
}

gboolean open_gtk_game(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	char* str;
	str = open_dialog_box();
	if (str != NULL)
	{
		my_strcpy(savefile, str, sizeof(savefile));
		cmd_insert(CMD_LOADFILE);
		game_in_progress = true;
	}
	return true;
}

gboolean save_gtk_game(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	if (game_in_progress && character_generated)
	{
		if (!inkey_flag)
		{
			printf( "You may not do that right now.");
			return false;
		}

		/* Hack -- Forget messages */
		msg_flag = false;
		
		/* Save the game */
		save_game();
	}
	return true;
}

/*
 * Quit the game.
 */
gboolean quit_gtk(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	quit(NULL);
	exit(0);
}

static void hook_quit(cptr str)
{
	exit(0);
}

errr CheckEvent(bool wait)
{
	if (wait)
		gtk_main_iteration();
	else
		while (gtk_events_pending())
			gtk_main_iteration();

	return (0);
}

static errr get_init_cmd()
{
	Term_fresh();

	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 17);
	CheckEvent(TRUE);

	return 0;
}

/* Command dispatcher for gtk builds */
static errr gtk_get_cmd(cmd_context context, bool wait)
{
	if (context == CMD_INIT) 
		return get_init_cmd();
	else 
		return textui_get_cmd(context, wait);
}

/*
 * Initialization function
 */

static void handle_map(game_event_type type, game_event_data *data, void *user)
{
	if (use_graphics != arg_graphics)
	{
		use_graphics = arg_graphics;
		init_graf(arg_graphics);
	}
}


static void handle_game(game_event_type type, game_event_data *data, void *user)
{
	init_graf(arg_graphics);
}

void init_handlers()
{
	
	/* Activate hooks */
	quit_aux = hook_quit;

	/* Set command hook */
	cmd_get_hook = gtk_get_cmd;

	// Use event_add_handler_set here to create events, and event_add_handler to add them. See game-event.h.
	event_add_handler(EVENT_MAP, handle_map, NULL);
	event_add_handler(EVENT_ENTER_GAME, handle_game, NULL);
}

errr init_gtk(int argc, char **argv)
{
	int i, err;
	char logo[1024];
	
	  g_thread_init(NULL);
	  gdk_threads_init();
	
	/* Initialize the environment */
	gtk_init(&argc, &argv);
	
	path_build(logo, sizeof(logo), ANGBAND_DIR_XTRA_ICON, "att-256.png");
	err = gtk_window_set_default_icon_from_file(logo, NULL);
	
	 init_handlers();

	/* Create windows (backwards!) */
	for (i = MAX_GTK_NEW_TERM - 1; i >= 0; i--)
	{
		/* Link */
		term_data_link(i);
	}
	
	Term_activate(&term_window[0].t);
	
	/* Set the system suffix */
	ANGBAND_SYS = "gtk";
	
	/* Catch nasty signals, unless we want to see them */
	#ifndef GTK_DEBUG
	//signals_init();
	#endif
	
	Term_fresh();
	
	/* Set up the display handlers and things. */
	init_display();
	
	/* Let's play */
	play_game();
	/* Stop now */
	exit(0);
	
	/* Success */
	return (0);
}


#endif /* USE_XXX */
