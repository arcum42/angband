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
#include "gtk-windows.h"

#include "textui.h"
#include "game-event.h"
#include "macro.h"
#include "files.h"
#include "init.h"

extern void set_graphics(int g);

bool game_in_progress = false;

/*
 * Help message.
 *   1st line = max 68 chars.
 *   Start next lines with 11 spaces
 */
const char help_gtk[] = "Describe XXX, subopts -describe suboptions here";


/*** Function hooks needed by "Term" ***/

gboolean on_mouse_click(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	term_data *td = (term_data*)(Term->data);
	
	/* Where is the mouse? */
	int x = event->x / td->font.w;
	int y = event->y / td->font.h;
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

extern void resize_window(term_data *td);
void pick_font()
{
	char *fontname;
	GtkWidget *dialog;
	term_data* td = &term_window[0];
	
	dialog = gtk_font_selection_dialog_new("Pick a font, preferably fixed-width.");
	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(dialog), td->font.name);
	gtk_font_selection_dialog_set_preview_text(GTK_FONT_SELECTION_DIALOG(dialog), "The Boil-covered wretch drools on you!");
	
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
	  {
		 fontname = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
		for(int i = 0; i < MAX_GTK_NEW_TERM; i++)
		{
			td = &term_window[i];
			my_strcpy(td->font.name, fontname, sizeof(td->font.name));
		}
	  }
	  
	gtk_widget_destroy (dialog);
	resize_window(&term_window[0]);
	
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

static errr get_init_cmd(bool wait)
{
	Term_fresh();

	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 17);
	CheckEvent(wait);

	return 0;
}

/* Command dispatcher for gtk builds */
static errr gtk_get_cmd(cmd_context context, bool wait)
{
	if (context == CMD_INIT) 
		return get_init_cmd(wait);
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
		set_graphics(arg_graphics);
	}
}


static void handle_game(game_event_type type, game_event_data *data, void *user)
{
	set_graphics(arg_graphics);
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
	
	/* Init dirs */
	create_needed_dirs(); 
	
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
	
	/* Do all the things main() in main.c already does */	
	cleanup_angband();
		
	quit(NULL);
	
	exit(0); /* just in case */
	
	/* Success */
	return (0);
}


#endif /* USE_XXX */
