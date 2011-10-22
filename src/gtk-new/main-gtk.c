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
#include "buildid.h"
#include "player/player.h"

#ifdef USE_GTK_NEW

#include "main-gtk.h"
#include "main.h"
#include "gtk-term.h"
#include "gtk-windows.h"

#include "textui.h"
#include "game-event.h"
#include "files.h"
#include "init.h"
#include "grafmode.h"

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
	byte mods;

	int ch = 0;
	guint modifiers = gtk_accelerator_get_default_mod_mask();

	/* Extract four "modifier flags" */
	int mc = ((event->state & modifiers) == GDK_CONTROL_MASK) ? TRUE : FALSE;
	int ms = ((event->state & modifiers) == GDK_SHIFT_MASK) ? TRUE : FALSE;
	int mo = ((event->state & modifiers) == GDK_MOD1_MASK) ? TRUE : FALSE;
	int mx = ((event->state & modifiers) == GDK_MOD3_MASK) ? TRUE : FALSE;
	int kp = FALSE;

	/* see gdk/gdkkeysyms.h */
	// http://www.koders.com/c/fidD9E5E78FD91FE6ABDD6D3F78DA5E4A0FADE79933.aspx
	switch (event->keyval) {
		case GDK_Shift_L: case GDK_Shift_R: case GDK_Control_L:
		case GDK_Control_R: case GDK_Caps_Lock: case GDK_Shift_Lock:
		case GDK_Meta_L: case GDK_Meta_R: case GDK_Alt_L: case GDK_Alt_R:
		case GDK_Super_L: case GDK_Super_R: case GDK_Hyper_L:
		case GDK_Hyper_R:
			/* ignore things that are just modifiers */
			return TRUE;

		case GDK_BackSpace: ch = KC_BACKSPACE; break;
		case GDK_Tab: ch = KC_TAB; break;
		case GDK_Return: ch = KC_RETURN; break;
		case GDK_Escape: ch = ESCAPE; break;
		case GDK_Delete: ch = KC_DELETE; break;

		case GDK_Home: ch = KC_HOME; break;
		case GDK_Left: ch = ARROW_LEFT; break;
		case GDK_Up: ch = ARROW_UP; break;
		case GDK_Right: ch = ARROW_RIGHT; break;
		case GDK_Down: ch = ARROW_DOWN; break;
		case GDK_Page_Up: ch = KC_PGUP; break;
		case GDK_Page_Down: ch = KC_PGDOWN; break;
		case GDK_End: ch = KC_END; break;
		case GDK_Insert: ch = KC_INSERT; break;

		/* keypad */
		case GDK_KP_Insert:
		case GDK_KP_End:
		case GDK_KP_Down:
		case GDK_KP_Page_Down:
		case GDK_KP_Left:
		case GDK_KP_Right:
		case GDK_KP_Home:
		case GDK_KP_Up:
		case GDK_KP_Page_Up:
		case GDK_KP_Decimal:
		case GDK_KP_Divide:
		case GDK_KP_Multiply:
		case GDK_KP_Subtract:
		case GDK_KP_Add:
		case GDK_KP_Enter:
		case GDK_KP_Equal:
			switch(event->keyval) {
				case GDK_KP_Insert: ch = '0'; break;
				case GDK_KP_End: ch = '1'; break;
				case GDK_KP_Down: ch = '2'; break;
				case GDK_KP_Page_Down: ch = '3'; break;
				case GDK_KP_Left: ch = '4'; break;
				case GDK_KP_Right: ch = '6'; break;
				case GDK_KP_Home: ch = '7'; break;
				case GDK_KP_Up: ch = '8'; break;
				case GDK_KP_Page_Up: ch = '9'; break;
				case GDK_KP_Decimal: ch = '.'; break;
				case GDK_KP_Divide: ch = '/'; break;
				case GDK_KP_Multiply: ch = '*'; break;
				case GDK_KP_Subtract: ch = '-'; break;
				case GDK_KP_Add: ch = '+'; break;
				case GDK_KP_Enter: ch = '\n';break;
				case GDK_KP_Equal: ch = '='; break;
			}

		/* intentional fall-though */
		case GDK_KP_0: case GDK_KP_1: case GDK_KP_2:
		case GDK_KP_3: case GDK_KP_4: case GDK_KP_5:
		case GDK_KP_6: case GDK_KP_7: case GDK_KP_8:
		case GDK_KP_9: kp = TRUE; break;

		case GDK_F1: ch = KC_F1; break;
		case GDK_F2: ch = KC_F2; break;
		case GDK_F3: ch = KC_F3; break;
		case GDK_F4: ch = KC_F4; break;
		case GDK_F5: ch = KC_F5; break;
		case GDK_F6: ch = KC_F6; break;
		case GDK_F7: ch = KC_F7; break;
		case GDK_F8: ch = KC_F8; break;
		case GDK_F9: ch = KC_F9; break;
		case GDK_F10: ch = KC_F10; break;
		case GDK_F11: ch = KC_F11; break;
		case GDK_F12: ch = KC_F12; break;
		case GDK_F13: ch = KC_F13; break;
		case GDK_F14: ch = KC_F14; break;
		case GDK_F15: ch = KC_F15; break;
	}

	mods = (mo ? KC_MOD_ALT : 0) | (mx ? KC_MOD_META : 0) |
			(kp ? KC_MOD_KEYPAD : 0);

	if (ch) 
	{
		mods |= (mc ? KC_MOD_CONTROL : 0) | (ms ? KC_MOD_SHIFT : 0);
		Term_keypress(ch, mods);
	} 
	else if (event->length == 1) 
	{
		keycode_t code = event->string[0];

		if (mc && MODS_INCLUDE_CONTROL(code)) mods |= KC_MOD_CONTROL;
		if (ms && MODS_INCLUDE_SHIFT(code)) mods |= KC_MOD_SHIFT;

		Term_keypress(code, mods);

		/* Control keys get passed along to menus, are not "handled" */
		return !mc;
	}

	return TRUE;
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
	
	if (gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
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

static void hook_quit(const char *str)
{
	printf("%s", str);
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
	
	/* load possible graphics modes */
	init_graphics_modes("graphics.txt");
	
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
