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
#include <iconv.h>

iconv_t conv;

/* this is used to draw the various terrain characters */
static unsigned int graphics_table[32] = {
	000, '*', '#', '?', '?', '?', '?', '.',
	'+', '?', '?', '+', '+', '+', '+', '+',
	'~', '-', '-', '-', '_', '+', '+', '+',
	'+', '|', '?', '?', '?', '?', '?', '.',
};

/*
 * An array of "term_data" structures, one for each "sub-window"
 */
term_data term_window[MAX_GTK_NEW_TERM];

/*
 * Hack -- redraw a term_data.
 * Note that "Term_redraw()" calls "TERM_XTRA_CLEAR"
 */
void term_data_redraw(term_data *td)
{
	term *old = Term;
	
	/* Activate the term passed to it, not term 0! */
	Term_activate(&td->t);

	//Term_resize(td->cols, td->rows);
	Term_resize(80,24);
	Term_redraw();
	Term_fresh();
	
	Term_activate(old);
}

void force_redraw()
{
	if (game_in_progress)
	{
		reset_visuals(TRUE);
		Term_key_push(KTRL('R'));
	}
}

/*
 * An array of "term_data" structures, one for each "sub-window"
 */
term_data term_window[MAX_GTK_NEW_TERM];

/*
 * Init a new "term"
 *
 * Create the surfaces in memory for each window, and the window itself if neccessary.
 *
 */
static void Term_init_gtk(term *t)
{
	term_data *td = (term_data*)(t->data);
	
	get_font_size(td);
	create_surface(td);
	create_window(td);
}

/*
 * Nuke an old "term"
 *
 * Just a stub right now.
 */
static void Term_nuke_gtk(term *t)
{
}

static errr Term_fresh_gtk(void)
{
	term_data *td = (term_data*)(Term->data);
	term_redraw(td);
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
 */
static errr Term_xtra_gtk(int n, int v)
{
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
 * The cursor is implemented by just hiliting the current character in grey right now.
 *
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
 *
 */
static errr Term_wipe_gtk(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);

	clear_chars(td, x, y, n);

	/* Success */
	return (0);
}


static byte Term_xchar_gtk(byte c)
{
	/* Can't translate Latin-1 to UTF-8 here since we have to return a byte. */
	return c;
}

char *process_control_chars(int n, cptr s)
{
	char *s2 = (char *)malloc(sizeof(char) * n);
	int i;
	for (i = 0; i < n; i++) {
		unsigned char c = s[i];
		if (c < 32) {
			s2[i] = graphics_table[c];
		} else if (c == 127) {
			s2[i] = '#';
		} else {
			s2[i] = c;
		}
	}

	return s2;
}

char *latin1_to_utf8(int n, cptr s)
{
	size_t inbytes = n;
	char *s2 = process_control_chars(n, s);
	char *p2 = s2;

	size_t outbytes = 4 * n;
	char *s3 = (char *)malloc(sizeof(char) * outbytes);
	char *p3 = s3;

	size_t result = iconv(conv, &p2, &inbytes, &p3, &outbytes);

	if (result == (size_t)(-1)) {
		printf("iconv() failed: %d\n", errno);
		free(s3);
		return s2;
	} else {
		free(s2);
		return s3;
	}
}

/*
 * Draw some text on the screen
 */
static errr Term_text_gtk(int x, int y, int n, byte a, const char *cp)
{
	term_data *td = (term_data*)(Term->data);
	
	char *s2;
	
	if (conv == NULL)
		s2 = process_control_chars(n, cp);
	else
		s2 = latin1_to_utf8(n, cp);
	
	write_chars(td, x, y, n, a, s2);

	/* Success */
	return (0);
}


/*
 * Draw some attr/char pairs on the screen
 *
 * Basically, what we have here is that Angband wants to draw a series of n tiles at x/y. ap[n]/cp[n] gives you which tiles are in the foreground,
 * and tap/tcp are the terrain tiles for the background. Don't ask me why we don't put it all in a struct, and pass an array of that struct.
 */
static errr Term_pict_gtk(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
{
	term_data *td = (term_data*)(Term->data);
	draw_tiles(td, x, y, n, ap, cp, tap, tcp);

	/* Success */
	return (0);
}


/*** Internal Functions ***/


/*
 * Instantiate a "term_data" structure
 */
void term_data_link(int i)
{
	term_data *td = &term_window[i];
	term *t = &td->t;
	td->id = i;

	conv = iconv_open("UTF-8", "ISO-8859-1");
	if (conv == (iconv_t)(-1)) 
	{
		printf("iconv_open() failed: %d\n", errno);
		conv = NULL;
	}
	
	/* Initialize the term */
	term_init(t, 80, 24, 256);

	/* Use a "soft" cursor */
	t->soft_cursor = true;

	/* Erase with "dark space" */
	t->attr_blank = TERM_DARK;
	t->char_blank = ' ';
	t->higher_pict = true;
	
	/* Prepare the init/nuke hooks */
	t->init_hook = Term_init_gtk;
	t->nuke_hook = Term_nuke_gtk;

	
	/* Prepare the template hooks */
	t->xtra_hook = Term_xtra_gtk;
	t->text_hook = Term_text_gtk;
	t->wipe_hook = Term_wipe_gtk;
	t->curs_hook = Term_curs_gtk;
	t->pict_hook = Term_pict_gtk;
	if (conv != NULL) t->xchar_hook = Term_xchar_gtk;
	
	/* Remember where we came from */
	t->data = td;
	my_strcpy(td->font.name, "Monospace 10", 13);

	/* Activate it */
	Term_activate(t);

	/* Global pointer */
	angband_term[i] = t;
}

#endif
