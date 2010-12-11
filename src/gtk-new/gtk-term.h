/*
 * File: gtk-term.h
 * Purpose: Header file for the term code of the GTK port for Angband
 *
 * Copyright (c) 2000-2010 Shanoah Alkire
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
 
#pragma once

#include "angband.h"

#ifdef USE_GTK_NEW

#ifndef GTK_TERM_INCLUDE
#define GTK_TERM_INCLUDE

#include "gtk-common.h"

extern void term_data_link(int i);
extern void force_redraw();
extern void term_data_redraw(term_data *td);

extern void create_window(term_data *td);
extern void delete_window(term_data *td);

#endif
#endif
