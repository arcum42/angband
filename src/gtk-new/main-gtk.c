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

#ifdef USE_GTK_NEW

#include "main-gtk.h"
#include "main.h"
#include "gtk-term.h"

#include "textui.h"
#include "game-event.h"
#include "macro.h"
#include "files.h"

/*
 * Help message.
 *   1st line = max 68 chars.
 *   Start next lines with 11 spaces
 */
const char help_gtk[] = "Describe XXX, subopts -describe suboptions here";


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

char* open_dialog_box()
{
	char *filename;
	GtkWidget *dialog;
	
	dialog = gtk_file_chooser_dialog_new ("Open File",
					     NULL,
					      GTK_FILE_CHOOSER_ACTION_OPEN,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					      NULL);
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
	
	dialog = gtk_file_chooser_dialog_new ("Save File",
					      NULL,
					      GTK_FILE_CHOOSER_ACTION_SAVE,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					      NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
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
	gtk_widget_destroy(widget);
	return true;
}

gboolean new_gtk_game(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	cmd_insert(CMD_NEWGAME);
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
	}
	return true;
}

gboolean save_gtk_game(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	char* str;
	//str = save_dialog_box();
	//my_strcpy(savefile, str, sizeof(savefile));
	save_game();
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


void init_handlers()
{
	
	/* Activate hooks */
	quit_aux = hook_quit;

	/* Set command hook */
	cmd_get_hook = gtk_get_cmd;

	// Use event_add_handler_set here to create events, and event_add_handler to add them. See game-event.h.
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
	ANGBAND_SYS = "gtk-new";
	
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
