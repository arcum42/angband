/*
 * File: gtk-term.c
 * Purpose: Contains the term code for the Gtk port.
 *
 * Copyright (c) 2010 Shanoah Alkire (with occasional code from Ben Harrison.)
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
#include "gtk-drawing.h"
#include <pango/pangocairo.h>


extern void create_window(term_data *td);
extern void delete_window(term_data *td);

/*
 * An array of "term_data" structures, one for each "sub-window"
 */
term_data term_window[MAX_GTK_NEW_TERM];

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
	
	create_font(td);
	create_surface(td);
	
	if ((td->visible) || (td->id == 0))
	{
		create_window(td);
	}
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
	//term_data *td = (term_data*)(t->data);
}

static errr Term_fresh_gtk(void)
{
	term_data *td = (term_data*)(Term->data);
	term_redraw(td);
	//gdk_window_process_updates(td->win->window, 1);
	return (0);
}

static  errr Term_flush_gtk()
{
	term_data *td = (term_data*)(Term->data);
	term_redraw(td);
	return 0;
}

static  errr Term_clear_gtk()
{
	term_data *td = (term_data*)(Term->data);
	clear_surface(td);
	return 0;
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
	//term_data *td = (term_data*)(Term->data);
	
	/* Handle a subset of the legal requests */
	switch (n)
	{
		/* Make a noise */
		case TERM_XTRA_NOISE: 
			return (0);

		/* Flush the output */
		case TERM_XTRA_FRESH: 
			return (Term_fresh_gtk());

		/* Process random events */
		case TERM_XTRA_BORED: 
			return (CheckEvent(0));

		/* Process Events */
		case TERM_XTRA_EVENT: 
			return (CheckEvent(v));

		/* Flush the events */
		case TERM_XTRA_FLUSH: 
			return (Term_flush_gtk());

		/* Handle change in the "level" */
		case TERM_XTRA_LEVEL: 
			return (0);

		/* Clear the screen */
		case TERM_XTRA_CLEAR: 
			return (Term_clear_gtk());

		/* Delay for some milliseconds */
		case TERM_XTRA_DELAY:
			if (v > 0) usleep(1000 * v);
			return (0);

		/* React to changes */
		case TERM_XTRA_REACT: 
			return (0);
	}

	/* Unknown */
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
	
	hilite_char(td, x, y, TERM_SLATE);

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

	clear_chars(td, x, y, n);
	/* XXX XXX XXX */

	/* Success */
	return (0);
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
	write_chars(td, x, y, n, a, cp);

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
	//term_data *td = (term_data*)(Term->data);

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

	/* Use a "soft" cursor */
	t->soft_cursor = TRUE;
	
	/* Prepare the init/nuke hooks */
	t->init_hook = Term_init_gtk;
	t->nuke_hook = Term_nuke_gtk;

	
	/* Prepare the template hooks */
	t->xtra_hook = Term_xtra_gtk;
	t->text_hook = Term_text_gtk;
	t->wipe_hook = Term_wipe_gtk;
	t->curs_hook = Term_curs_gtk;
	t->pict_hook = Term_pict_gtk;

	/* Remember where we came from */
	t->data = td;
	my_strcpy(td->font, "Monospace 10", 13);

	/* Activate it */
	Term_activate(t);

	/* Global pointer */
	angband_term[i] = t;
}

#endif
