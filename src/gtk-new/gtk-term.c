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

#include "main.h"
#include "gtk-term.h"


/*
 * An array of "term_data" structures, one for each "sub-window"
 */
term_data term_window[MAX_GTK_NEW_TERM];


/*** Function hooks needed by "Term" ***/

// I'll change this later, but for now it's convenient.
extern gboolean quit_gtk(GtkWidget *widget, GdkEvent *event, gpointer user_data);

gboolean close_window(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_destroy(widget);
	return true;
}

/*
 * Init a new "term"
 *
 * This function should do whatever is necessary to prepare a new "term"
 * for use by the "z-term.c" package.  This may include clearing the window,
 * preparing the cursor, setting the font/colors, etc.  Usually, this
 * function does nothing, and the "init_xxx()" function does it all.
 */
static void Term_init_gtk(term *t)
{
	term_data *td = (term_data*)(t->data);

	td->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	if (td->id == 1)
	{
		gtk_window_set_title(GTK_WINDOW(td->window), "Angband");
		g_signal_connect(td->window, "delete-event", G_CALLBACK (quit_gtk), NULL);
		g_signal_connect(td->window, "destroy",  G_CALLBACK (quit_gtk), NULL);
	}
	else
	{
		char title[10];
		strnfmt(title, sizeof(title), "Term %d", td->id);
		gtk_window_set_title(GTK_WINDOW(td->window), title);
		g_signal_connect(td->window, "delete-event", G_CALLBACK (close_window), NULL);
		g_signal_connect(td->window, "destroy",  G_CALLBACK (close_window), NULL);
	}
	
	gtk_widget_show(td->window);
	/* XXX XXX XXX */
}



/*
 * Nuke an old "term"
 *
 * This function is called when an old "term" is no longer needed.  It should
 * do whatever is needed to clean up before the program exits, such as wiping
 * the screen, restoring the cursor, fixing the font, etc.  Often this function
 * does nothing and lets the operating system clean up when the program quits.
 */
static void Term_nuke_gtk(term *t)
{
	term_data *td = (term_data*)(t->data);

	/* XXX XXX XXX */
}



/*
 * Do a "user action" on the current "term"
 *
 * This function allows the visual module to do implementation defined
 * things when the user activates the "system defined command" command.
 *
 * This function is normally not used.
 *
 * In general, this function should return zero if the action is successfully
 * handled, and non-zero if the action is unknown or incorrectly handled.
 */
static errr Term_user_gtk(int n)
{
	term_data *td = (term_data*)(Term->data);

	/* XXX XXX XXX */

	/* Unknown */
	return (1);
}


/*
 * Do a "special thing" to the current "term"
 *
 * This function must react to a large number of possible arguments, each
 * corresponding to a different "action request" by the "z-term.c" package,
 * or by the application itself.
 *
 * The "action type" is specified by the first argument, which must be a
 * constant of the form "TERM_XTRA_*" as given in "z-term.h", and the second
 * argument specifies the "information" for that argument, if any, and will
 * vary according to the first argument.
 *
 * In general, this function should return zero if the action is successfully
 * handled, and non-zero if the action is unknown or incorrectly handled.
 */
static errr Term_xtra_gtk(int n, int v)
{
	term_data *td = (term_data*)(Term->data);

	/* Analyze */
	switch (n)
	{
		case TERM_XTRA_EVENT:
		{
			/*
			 * Process some pending events XXX XXX XXX
			 *
			 * Wait for at least one event if "v" is non-zero
			 * otherwise, if no events are ready, return at once.
			 * When "keypress" events are encountered, the "ascii"
			 * value corresponding to the key should be sent to the
			 * "Term_keypress()" function.  Certain "bizarre" keys,
			 * such as function keys or arrow keys, may send special
			 * sequences of characters, such as control-underscore,
			 * plus letters corresponding to modifier keys, plus an
			 * underscore, plus carriage return, which can be used by
			 * the main program for "macro" triggers.  This action
			 * should handle as many events as is efficiently possible
			 * but is only required to handle a single event, and then
			 * only if one is ready or "v" is true.
			 *
			 * This action is required.
			 */

			return (0);
		}

		case TERM_XTRA_FLUSH:
		{
			/*
			 * Flush all pending events XXX XXX XXX
			 *
			 * This action should handle all events waiting on the
			 * queue, optionally discarding all "keypress" events,
			 * since they will be discarded anyway in "z-term.c".
			 *
			 * This action is required, but may not be "essential".
			 */

			return (0);
		}

		case TERM_XTRA_CLEAR:
		{
			/*
			 * Clear the entire window XXX XXX XXX
			 *
			 * This action should clear the entire window, and redraw
			 * any "borders" or other "graphic" aspects of the window.
			 *
			 * This action is required.
			 */

			return (0);
		}

		case TERM_XTRA_SHAPE:
		{
			/*
			 * Set the cursor visibility XXX XXX XXX
			 *
			 * This action should change the visibility of the cursor,
			 * if possible, to the requested value (0=off, 1=on)
			 *
			 * This action is optional, but can improve both the
			 * efficiency (and attractiveness) of the program.
			 */

			return (0);
		}

		case TERM_XTRA_FROSH:
		{
			/*
			 * Flush a row of output XXX XXX XXX
			 *
			 * This action should make sure that row "v" of the "output"
			 * to the window will actually appear on the window.
			 *
			 * This action is optional, assuming that "Term_text_xxx()"
			 * (and similar functions) draw directly to the screen, or
			 * that the "TERM_XTRA_FRESH" entry below takes care of any
			 * necessary flushing issues.
			 */

			return (0);
		}

		case TERM_XTRA_FRESH:
		{
			/*
			 * Flush output XXX XXX XXX
			 *
			 * This action should make sure that all "output" to the
			 * window will actually appear on the window.
			 *
			 * This action is optional, assuming that "Term_text_xxx()"
			 * (and similar functions) draw directly to the screen, or
			 * that the "TERM_XTRA_FROSH" entry above takes care of any
			 * necessary flushing issues.
			 */

			return (0);
		}

		case TERM_XTRA_NOISE:
		{
			/*
			 * Make a noise XXX XXX XXX
			 *
			 * This action should produce a "beep" noise.
			 *
			 * This action is optional, but convenient.
			 */

			return (0);
		}

		case TERM_XTRA_BORED:
		{
			/*
			 * Handle random events when bored XXX XXX XXX
			 *
			 * This action is optional, and normally not important
			 */

			return (0);
		}

		case TERM_XTRA_REACT:
		{
			/*
			 * React to global changes XXX XXX XXX
			 *
			 * For example, this action can be used to react to
			 * changes in the global "angband_color_table[MAX_COLORS][4]" array.
			 *
			 * This action is optional, but can be very useful for
			 * handling "color changes" and the "arg_sound" and/or
			 * "arg_graphics" options.
			 */

			return (0);
		}

		case TERM_XTRA_ALIVE:
		{
			/*
			 * Change the "hard" level XXX XXX XXX
			 *
			 * This action is used if the program changes "aliveness"
			 * by being either "suspended" (v=0) or "resumed" (v=1)
			 * This action is optional, unless the computer uses the
			 * same "physical screen" for multiple programs, in which
			 * case this action should clean up to let other programs
			 * use the screen, or resume from such a cleaned up state.
			 *
			 * This action is currently only used by "main-gcu.c",
			 * on UNIX machines, to allow proper "suspending".
			 */

			return (0);
		}

		case TERM_XTRA_LEVEL:
		{
			/*
			 * Change the "soft" level XXX XXX XXX
			 *
			 * This action is used when the term window changes "activation"
			 * either by becoming "inactive" (v=0) or "active" (v=1)
			 *
			 * This action can be used to do things like activate the proper
			 * font / drawing mode for the newly active term window.  This
			 * action should NOT change which window has the "focus", which
			 * window is "raised", or anything like that.
			 *
			 * This action is optional if all the other things which depend
			 * on what term is active handle activation themself, or if only
			 * one "term_data" structure is supported by this file.
			 */

			return (0);
		}

		case TERM_XTRA_DELAY:
		{
			/*
			 * Delay for some milliseconds XXX XXX XXX
			 *
			 * This action is useful for proper "timing" of certain
			 * visual effects, such as breath attacks.
			 *
			 * This action is optional, but may be required by this file,
			 * especially if special "macro sequences" must be supported.
			 */

			return (0);
		}
	}

	/* Unknown or Unhandled action */
	return (1);
}


/*
 * Display the cursor
 *
 * This routine should display the cursor at the given location
 * (x,y) in some manner.  On some machines this involves actually
 * moving the physical cursor, on others it involves drawing a fake
 * cursor in some form of graphics mode.  Note the "soft_cursor"
 * flag which tells "z-term.c" to treat the "cursor" as a "visual"
 * thing and not as a "hardware" cursor.
 *
 * You may assume "valid" input if the window is properly sized.
 *
 * You may use the "Term_what(x, y, &a, &c)" function, if needed,
 * to determine what attr/char should be "under" the new cursor,
 * for "inverting" purposes or whatever.
 */
static errr Term_curs_gtk(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	/* XXX XXX XXX */

	/* Success */
	return (0);
}


/*
 * Erase some characters
 *
 * This function should erase "n" characters starting at (x,y).
 *
 * You may assume "valid" input if the window is properly sized.
 */
static errr Term_wipe_gtk(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);

	/* XXX XXX XXX */

	/* Success */
	return (0);
}

/*
 * Given a position in the ISO Latin-1 character set, return
 * the correct character on this system.
 */
 static byte Term_xchar_gtk(byte c)
{
 	/* The xxx port uses the Latin-1 standard */
 	return (c);
}


/*
 * Draw some text on the screen
 *
 * This function should actually display an array of characters
 * starting at the given location, using the given "attribute",
 * and using the given string of characters, which contains
 * exactly "n" characters and which is NOT null-terminated.
 *
 * You may assume "valid" input if the window is properly sized.
 *
 * You must be sure that the string, when written, erases anything
 * (including any visual cursor) that used to be where the text is
 * drawn.  On many machines this happens automatically, on others,
 * you must first call "Term_wipe_xxx()" to clear the area.
 *
 * In color environments, you should activate the color contained
 * in "color_data[a & BASIC_COLORS]", if needed, before drawing anything.
 *
 * You may ignore the "attribute" if you are only supporting a
 * monochrome environment, since this routine is normally never
 * called to display "black" (invisible) text, including the
 * default "spaces", and all other colors should be drawn in
 * the "normal" color in a monochrome environment.
 *
 * Note that if you have changed the "attr_blank" to something
 * which is not black, then this function must be able to draw
 * the resulting "blank" correctly.
 *
 * Note that this function must correctly handle "black" text if
 * the "always_text" flag is set, if this flag is not set, all the
 * "black" text will be handled by the "Term_wipe_xxx()" hook.
 */
static errr Term_text_gtk(int x, int y, int n, byte a, const char *cp)
{
	term_data *td = (term_data*)(Term->data);

	/* XXX XXX XXX */

	/* Success */
	return (0);
}


/*
 * Draw some attr/char pairs on the screen
 *
 * This routine should display the given "n" attr/char pairs at
 * the given location (x,y).  This function is only used if one
 * of the flags "always_pict" or "higher_pict" is defined.
 *
 * You must be sure that the attr/char pairs, when displayed, will
 * erase anything (including any visual cursor) that used to be at
 * the given location.  On many machines this is automatic, but on
 * others, you must first call "Term_wipe_xxx(x, y, 1)".
 *
 * With the "higher_pict" flag, this function can be used to allow
 * the display of "pseudo-graphic" pictures, for example, by using
 * the attr/char pair as an encoded index into a pixmap of special
 * "pictures".
 *
 * With the "always_pict" flag, this function can be used to force
 * every attr/char pair to be drawn by this function, which can be
 * very useful if this file can optimize its own display calls.
 *
 * This function is often associated with the "arg_graphics" flag.
 *
 * This function is only used if one of the "higher_pict" and/or
 * "always_pict" flags are set.
 */
static errr Term_pict_gtk(int x, int y, int n, const byte *ap, const char *cp,
                          const byte *tap, const char *tcp)
{
	term_data *td = (term_data*)(Term->data);

	/* XXX XXX XXX */

	/* Success */
	return (0);
}



/*** Internal Functions ***/


/*
 * Instantiate a "term_data" structure
 *
 * This is one way to prepare the "term_data" structures and to
 * "link" the various informational pieces together.
 *
 * This function assumes that every window should be 80x24 in size
 * (the standard size) and should be able to queue 256 characters.
 * Technically, only the "main screen window" needs to queue any
 * characters, but this method is simple.  One way to allow some
 * variation is to add fields to the "term_data" structure listing
 * parameters for that window, initialize them in the "init_xxx()"
 * function, and then use them in the code below.
 *
 * Note that "activation" calls the "Term_init_xxx()" hook for
 * the "term" structure, if needed.
 */
void term_data_link(int i)
{
	term_data *td = &term_window[i];
	term *t = &td->t;
	td->id = i;

	/* Initialize the term */
	term_init(t, 80, 24, 256);

	/* Prepare the init/nuke hooks */
	t->init_hook = Term_init_gtk;
	t->nuke_hook = Term_nuke_gtk;

	/* Prepare the template hooks */
	t->user_hook = Term_user_gtk;
	t->xtra_hook = Term_xtra_gtk;
	t->curs_hook = Term_curs_gtk;
	t->wipe_hook = Term_wipe_gtk;
	t->text_hook = Term_text_gtk;
	t->pict_hook = Term_pict_gtk;
	t->xchar_hook = Term_xchar_gtk;

	/* Remember where we came from */
	t->data = td;

	/* Activate it */
	Term_activate(t);

	/* Global pointer */
	angband_term[i] = t;
}

#endif
