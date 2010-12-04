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

/*
 * Help message.
 *   1st line = max 68 chars.
 *   Start next lines with 11 spaces
 */
const char help_gtk[] = "Describe XXX, subopts -describe suboptions here";

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
