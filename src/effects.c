/*
 * File: effects.c
 * Purpose: Big switch statement for every effect in the game
 *
 * Copyright (c) 2007 Andi Sidwell
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
#include "cave.h"
#include "effects.h"
#include "dungeon.h"
#include "monster/mon-spell.h"
#include "monster/mon-util.h"
#include "trap.h"
#include "spells.h"

/*
 * Entries for spell/activation descriptions
 */
typedef struct
{
	u16b index;          /* Effect index */
	bool aim;            /* Whether the effect requires aiming */
	u16b power;	     /* Power rating for obj-power.c */
	const char *desc;    /* Effect description */
} info_entry;

/*
 * Useful things about effects.
 */
static const info_entry effects[] =
{
	#define EFFECT(x, y, r, z)    { EF_##x, y, r, z },
	#include "list-effects.h"
	#undef EFFECT
};


/*
 * Utility functions
 */
bool effect_aim(effect_type effect)
{
	if (effect < 1 || effect > EF_MAX)
		return FALSE;

	return effects[effect].aim;
}

int effect_power(effect_type effect)
{
	if (effect < 1 || effect > EF_MAX)
		return FALSE;

	return effects[effect].power;
}

const char *effect_desc(effect_type effect)
{
	if (effect < 1 || effect > EF_MAX)
		return FALSE;

	return effects[effect].desc;
}

bool effect_obvious(effect_type effect)
{
	if (effect == EF_IDENTIFY)
		return TRUE;

	return FALSE;
}


/*
 * The "wonder" effect.
 *
 * Returns TRUE if the effect is evident.
 */
bool effect_wonder(int dir, int die, int beam)
{
/* This spell should become more useful (more
   controlled) as the player gains experience levels.
   Thus, add 1/5 of the player's level to the die roll.
   This eliminates the worst effects later on, while
   keeping the results quite random.  It also allows
   some potent effects only at high level. */

	bool visible = FALSE;
	int py = p_ptr->py;
	int px = p_ptr->px;
	int plev = p_ptr->lev;

	if (die > 100)
	{
		/* above 100 the effect is always visible */
		msg("You feel a surge of power!");
		visible = TRUE;
	}

	if (die < 8) visible = clone_monster(dir);
	else if (die < 14) visible = speed_monster(dir);
	else if (die < 26) visible = heal_monster(dir);
	else if (die < 31) visible = poly_monster(dir);
	else if (die < 36)
		visible = fire_bolt_or_beam(beam - 10, GF_MISSILE, dir,
		                            damroll(3 + ((plev - 1) / 5), 4));
	else if (die < 41) visible = confuse_monster(dir, plev, FALSE);
	else if (die < 46) visible = fire_ball(GF_POIS, dir, 20 + (plev / 2), 3);
	else if (die < 51) visible = light_line(dir);
	else if (die < 56)
		visible = fire_beam(GF_ELEC, dir, damroll(3+((plev-5)/6), 6));
	else if (die < 61)
		visible = fire_bolt_or_beam(beam-10, GF_COLD, dir,
		                            damroll(5+((plev-5)/4), 8));
	else if (die < 66)
		visible = fire_bolt_or_beam(beam, GF_ACID, dir,
		                            damroll(6+((plev-5)/4), 8));
	else if (die < 71)
		visible = fire_bolt_or_beam(beam, GF_FIRE, dir,
		                            damroll(8+((plev-5)/4), 8));
	else if (die < 76) visible = drain_life(dir, 75);
	else if (die < 81) visible = fire_ball(GF_ELEC, dir, 30 + plev / 2, 2);
	else if (die < 86) visible = fire_ball(GF_ACID, dir, 40 + plev, 2);
	else if (die < 91) visible = fire_ball(GF_ICE, dir, 70 + plev, 3);
	else if (die < 96) visible = fire_ball(GF_FIRE, dir, 80 + plev, 3);
	/* above 100 'visible' is already true */
	else if (die < 101) drain_life(dir, 100 + plev);
	else if (die < 104) earthquake(py, px, 12);
	else if (die < 106) destroy_area(py, px, 15, TRUE);
	else if (die < 108) banishment();
	else if (die < 110) dispel_monsters(120);
	else /* RARE */
	{
		dispel_monsters(150);
		slow_monsters();
		sleep_monsters(TRUE);
		hp_player(300);
	}

	return visible;
}


typedef struct effect_handler_context_s {
	const effect_type effect;
	const bool aware;
	const int dir;
	const int beam;
	const int boost;
	bool ident;
} effect_handler_context_t;


typedef struct effect_breath_info_s {
	int msg_type;
	int type;
	const char *msg;
} effect_breath_info_t;


#define EFFECT_STOP -1


bool effect_increment_timed_obvious(effect_handler_context_t *context, int type, int amount)
{
	player_inc_timed(p_ptr, type, amount, TRUE, TRUE);
	context->ident = TRUE;
	return TRUE;
}

bool effect_increment_timed_normal(effect_handler_context_t *context, int type, int amount)
{
	context->ident = player_inc_timed(p_ptr, type, amount, TRUE, TRUE);
	return TRUE;
}

bool effect_clear_timed_one(effect_handler_context_t *context, int type)
{
	context->ident = player_clear_timed(p_ptr, type, TRUE);
	return TRUE;
}

bool effect_clear_timed_multiple(effect_handler_context_t *context, ...)
{
	va_list type_list;
	int type;

	va_start(type_list, context);

	while (TRUE) {
		type = va_arg(type_list, int);

		if (type == EFFECT_STOP)
			break;

		effect_clear_timed_one(context, type);
	}

	va_end(type_list);

	return TRUE;
}

bool effect_breathe_one(effect_handler_context_t *context, int dam, int type, int msg_type, const char *msg)
{
	msgt(msg_type, "You breathe %s.", msg);
	fire_ball(type, context->dir, dam, 2);
	return TRUE;
}

bool effect_breathe_random(effect_handler_context_t *context, int dam, effect_breath_info_t const * const breaths, size_t breaths_max)
{
	int chance = randint0((u32b)breaths_max);
	return effect_breathe_one(context, dam, breaths[chance].type, breaths[chance].msg_type, breaths[chance].msg);
}

bool effect_stat_gain(effect_handler_context_t *context, int stat)
{
	context->ident = do_inc_stat(stat);
	return TRUE;
}

bool effect_stat_lose(effect_handler_context_t *context, int stat)
{
	take_hit(p_ptr, damroll(5, 5), "stat drain");
	(void)do_dec_stat(stat, FALSE);
	context->ident = TRUE;
	return TRUE;
}

bool effect_stat_restore_one(effect_handler_context_t *context, int stat)
{
	context->ident = do_res_stat(stat);
	return TRUE;
}

bool effect_stat_restore_all(effect_handler_context_t *context)
{
	effect_stat_restore_one(context, A_STR);
	effect_stat_restore_one(context, A_INT);
	effect_stat_restore_one(context, A_WIS);
	effect_stat_restore_one(context, A_DEX);
	effect_stat_restore_one(context, A_CON);
	return TRUE;
}

bool effect_project_bolt_or_beam(effect_handler_context_t *context, int type, int dam)
{
	fire_bolt_or_beam(context->beam, type, context->dir, dam);
	context->ident = TRUE;
	return TRUE;
}

bool effect_project_bolt_only(effect_handler_context_t *context, int type, int dam)
{
	fire_bolt(type, context->dir, dam);
	context->ident = TRUE;
	return TRUE;
}

bool effect_project_ball(effect_handler_context_t *context, int type, int dam, int radius)
{
	fire_ball(type, context->dir, dam, radius);
	context->ident = TRUE;
	return TRUE;
}

#pragma mark handlers



bool effect_handler_POISON(effect_handler_context_t *context)
{
	return effect_increment_timed_obvious(context, TMD_POISONED, damroll(2, 7) + 10);
}

bool effect_handler_BLIND(effect_handler_context_t *context)
{
	return effect_increment_timed_obvious(context, TMD_BLIND, damroll(4, 25) + 75);
}

bool effect_handler_SCARE(effect_handler_context_t *context)
{
	return effect_increment_timed_obvious(context, TMD_AFRAID, randint0(10) + 10);
}

bool effect_handler_CONFUSE(effect_handler_context_t *context)
{
	return effect_increment_timed_obvious(context, TMD_CONFUSED, damroll(4, 5) + 10);
}

bool effect_handler_HALLUC(effect_handler_context_t *context)
{
	return effect_increment_timed_obvious(context, TMD_IMAGE, randint0(250) + 250);
}

bool effect_handler_PARALYZE(effect_handler_context_t *context)
{
	return effect_increment_timed_obvious(context, TMD_PARALYZED, randint0(5) + 5);
}

bool effect_handler_SLOW(effect_handler_context_t *context)
{
	return effect_increment_timed_normal(context, TMD_SLOW, randint1(25) + 15);
}

bool effect_handler_CURE_POISON(effect_handler_context_t *context)
{
	return effect_clear_timed_one(context, TMD_POISONED);
}

bool effect_handler_CURE_BLINDNESS(effect_handler_context_t *context)
{
	return effect_clear_timed_one(context, TMD_BLIND);
}

bool effect_handler_CURE_PARANOIA(effect_handler_context_t *context)
{
	return effect_clear_timed_one(context, TMD_AFRAID);
}

bool effect_handler_CURE_CONFUSION(effect_handler_context_t *context)
{
	return effect_clear_timed_one(context, TMD_CONFUSED);
}

bool effect_handler_CURE_MIND(effect_handler_context_t *context)
{
	if (player_restore_mana(p_ptr, 10)) context->ident = TRUE;
	effect_clear_timed_multiple(context, TMD_CONFUSED, TMD_AFRAID, TMD_IMAGE, EFFECT_STOP);

	if (!of_has(p_ptr->state.flags, OF_RES_CONFU) &&
		player_inc_timed(p_ptr, TMD_OPP_CONF, 12 + damroll(6, 10), TRUE, TRUE))
		context->ident = TRUE;

	if (context->ident) msg("Your feel your head clear.");
	return TRUE;
}

bool effect_handler_CURE_BODY(effect_handler_context_t *context)
{
	if (hp_player(30)) context->ident = TRUE;
	effect_clear_timed_multiple(context, TMD_STUN, TMD_CUT, TMD_POISONED, TMD_BLIND, EFFECT_STOP);
	return TRUE;
}

bool effect_handler_CURE_LIGHT(effect_handler_context_t *context)
{
	if (hp_player(20)) context->ident = TRUE;
	effect_clear_timed_one(context, TMD_BLIND);
	if (player_dec_timed(p_ptr, TMD_CUT, 20, TRUE)) context->ident = TRUE;
	if (player_dec_timed(p_ptr, TMD_CONFUSED, 20, TRUE)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_CURE_SERIOUS(effect_handler_context_t *context)
{
	if (hp_player(40)) context->ident = TRUE;
	effect_clear_timed_multiple(context, TMD_CUT, TMD_BLIND, TMD_CONFUSED, EFFECT_STOP);
	return TRUE;
}

bool effect_handler_CURE_CRITICAL(effect_handler_context_t *context)
{
	if (hp_player(60)) context->ident = TRUE;
	effect_clear_timed_multiple(context, TMD_BLIND, TMD_CONFUSED, TMD_POISONED, TMD_STUN, TMD_CUT, TMD_AMNESIA, EFFECT_STOP);
	return TRUE;
}

bool effect_handler_CURE_FULL(effect_handler_context_t *context)
{
	int amt = (p_ptr->mhp * 35) / 100;
	if (amt < 300) amt = 300;

	if (hp_player(amt)) context->ident = TRUE;
	effect_clear_timed_multiple(context, TMD_BLIND, TMD_CONFUSED, TMD_POISONED, TMD_STUN, TMD_CUT, TMD_AMNESIA, EFFECT_STOP);
	return TRUE;
}

bool effect_handler_CURE_FULL2(effect_handler_context_t *context)
{
	if (hp_player(1200)) context->ident = TRUE;
	effect_clear_timed_multiple(context, TMD_BLIND, TMD_CONFUSED, TMD_POISONED, TMD_STUN, TMD_CUT, TMD_AMNESIA, EFFECT_STOP);
	return TRUE;
}

bool effect_handler_CURE_TEMP(effect_handler_context_t *context)
{
	effect_clear_timed_multiple(context, TMD_BLIND, TMD_CONFUSED, TMD_POISONED, TMD_STUN, TMD_CUT, EFFECT_STOP);
	return TRUE;
}

bool effect_handler_HEAL1(effect_handler_context_t *context)
{
	if (hp_player(500)) context->ident = TRUE;
	effect_clear_timed_one(context, TMD_CUT);
	return TRUE;
}

bool effect_handler_HEAL2(effect_handler_context_t *context)
{
	if (hp_player(1000)) context->ident = TRUE;
	effect_clear_timed_one(context, TMD_CUT);
	return TRUE;
}

bool effect_handler_HEAL3(effect_handler_context_t *context)
{
	if (hp_player(500)) context->ident = TRUE;
	effect_clear_timed_multiple(context, TMD_STUN, TMD_CUT, EFFECT_STOP);
	return TRUE;
}

bool effect_handler_GAIN_EXP(effect_handler_context_t *context)
{
	if (p_ptr->exp < PY_MAX_EXP) {
		msg("You feel more experienced.");
		player_exp_gain(p_ptr, 100000L);
		context->ident = TRUE;
	}
	return TRUE;
}

bool effect_handler_LOSE_EXP(effect_handler_context_t *context)
{
	if (!check_state(p_ptr, OF_HOLD_LIFE, p_ptr->state.flags) && (p_ptr->exp > 0)) {
		msg("You feel your memories fade.");
		player_exp_lose(p_ptr, p_ptr->exp / 4, FALSE);
	}
	context->ident = TRUE;
	wieldeds_notice_flag(p_ptr, OF_HOLD_LIFE);
	return TRUE;
}

bool effect_handler_RESTORE_EXP(effect_handler_context_t *context)
{
	if (restore_level()) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_RESTORE_MANA(effect_handler_context_t *context)
{
	if (p_ptr->csp < p_ptr->msp)
	{
		p_ptr->csp = p_ptr->msp;
		p_ptr->csp_frac = 0;
		msg("Your feel your head clear.");
		p_ptr->redraw |= (PR_MANA);
		context->ident = TRUE;
	}
	return TRUE;
}

bool effect_handler_GAIN_STR(effect_handler_context_t *context)
{
	return effect_stat_gain(context, A_STR);
}

bool effect_handler_GAIN_INT(effect_handler_context_t *context)
{
	return effect_stat_gain(context, A_INT);
}

bool effect_handler_GAIN_WIS(effect_handler_context_t *context)
{
	return effect_stat_gain(context, A_WIS);
}

bool effect_handler_GAIN_DEX(effect_handler_context_t *context)
{
	return effect_stat_gain(context, A_DEX);
}

bool effect_handler_GAIN_CON(effect_handler_context_t *context)
{
	return effect_stat_gain(context, A_CON);
}

bool effect_handler_GAIN_ALL(effect_handler_context_t *context)
{
	effect_stat_gain(context, A_STR);
	effect_stat_gain(context, A_INT);
	effect_stat_gain(context, A_WIS);
	effect_stat_gain(context, A_DEX);
	effect_stat_gain(context, A_CON);
	return TRUE;
}

bool effect_handler_BRAWN(effect_handler_context_t *context)
{
	/* Pick a random stat to decrease other than strength */
	int stat = randint0(A_MAX-1) + 1;

	if (do_dec_stat(stat, TRUE))
	{
		do_inc_stat(A_STR);
		context->ident = TRUE;
	}

	return TRUE;
}

bool effect_handler_INTELLECT(effect_handler_context_t *context)
{
	/* Pick a random stat to decrease other than intelligence */
	int stat = randint0(A_MAX-1);
	if (stat >= A_INT) stat++;

	if (do_dec_stat(stat, TRUE))
	{
		do_inc_stat(A_INT);
		context->ident = TRUE;
	}

	return TRUE;
}

bool effect_handler_CONTEMPLATION(effect_handler_context_t *context)
{
	/* Pick a random stat to decrease other than wisdom */
	int stat = randint0(A_MAX-1);
	if (stat >= A_WIS) stat++;

	if (do_dec_stat(stat, TRUE))
	{
		do_inc_stat(A_WIS);
		context->ident = TRUE;
	}

	return TRUE;
}

bool effect_handler_TOUGHNESS(effect_handler_context_t *context)
{
	/* Pick a random stat to decrease other than constitution */
	int stat = randint0(A_MAX-1);
	if (stat >= A_CON) stat++;

	if (do_dec_stat(stat, TRUE))
	{
		do_inc_stat(A_CON);
		context->ident = TRUE;
	}

	return TRUE;
}

bool effect_handler_NIMBLENESS(effect_handler_context_t *context)
{
	/* Pick a random stat to decrease other than dexterity */
	int stat = randint0(A_MAX-1);
	if (stat >= A_DEX) stat++;

	if (do_dec_stat(stat, TRUE))
	{
		do_inc_stat(A_DEX);
		context->ident = TRUE;
	}

	return TRUE;
}

bool effect_handler_LOSE_STR(effect_handler_context_t *context)
{
	return effect_stat_lose(context, A_STR);
}

bool effect_handler_LOSE_INT(effect_handler_context_t *context)
{
	return effect_stat_lose(context, A_INT);
}

bool effect_handler_LOSE_WIS(effect_handler_context_t *context)
{
	return effect_stat_lose(context, A_WIS);
}

bool effect_handler_LOSE_DEX(effect_handler_context_t *context)
{
	return effect_stat_lose(context, A_DEX);
}

bool effect_handler_LOSE_CON(effect_handler_context_t *context)
{
	return effect_stat_lose(context, A_CON);
}

bool effect_handler_LOSE_CON2(effect_handler_context_t *context)
{
	take_hit(p_ptr, damroll(10, 10), "poisonous food");
	(void)do_dec_stat(A_CON, FALSE);
	context->ident = TRUE;

	return TRUE;
}

bool effect_handler_RESTORE_STR(effect_handler_context_t *context)
{
	return effect_stat_restore_one(context, A_STR);
}

bool effect_handler_RESTORE_INT(effect_handler_context_t *context)
{
	return effect_stat_restore_one(context, A_INT);
}

bool effect_handler_RESTORE_WIS(effect_handler_context_t *context)
{
	return effect_stat_restore_one(context, A_WIS);
}

bool effect_handler_RESTORE_DEX(effect_handler_context_t *context)
{
	return effect_stat_restore_one(context, A_DEX);
}

bool effect_handler_RESTORE_CON(effect_handler_context_t *context)
{
	return effect_stat_restore_one(context, A_CON);
}

bool effect_handler_CURE_NONORLYBIG(effect_handler_context_t *context)
{
	msg("You feel life flow through your body!");
	restore_level();
	effect_clear_timed_multiple(context, TMD_BLIND, TMD_CONFUSED, TMD_POISONED, TMD_STUN, TMD_CUT, TMD_AMNESIA, TMD_IMAGE, EFFECT_STOP);
	effect_stat_restore_all(context);

	/* Recalculate max. hitpoints */
	update_stuff(p_ptr);

	hp_player(5000);

	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_RESTORE_ALL(effect_handler_context_t *context)
{
	/* Life, above, also gives these effects */
	effect_stat_restore_all(context);
	return TRUE;
}

bool effect_handler_RESTORE_ST_LEV(effect_handler_context_t *context)
{
	if (restore_level()) context->ident = TRUE;
	effect_stat_restore_all(context);
	return TRUE;
}

bool effect_handler_TMD_INFRA(effect_handler_context_t *context)
{
	return effect_increment_timed_normal(context, TMD_SINFRA, 100 + damroll(4, 25));
}

bool effect_handler_TMD_SINVIS(effect_handler_context_t *context)
{
	effect_clear_timed_one(context, TMD_BLIND);
	effect_increment_timed_normal(context, TMD_SINVIS, 12 + damroll(2, 6));
	return TRUE;
}

bool effect_handler_TMD_ESP(effect_handler_context_t *context)
{
	effect_clear_timed_one(context, TMD_BLIND);
	effect_increment_timed_normal(context, TMD_TELEPATHY, 24 + damroll(9, 9));
	return TRUE;
}

bool effect_handler_ENLIGHTENMENT(effect_handler_context_t *context)
{
	msg("An image of your surroundings forms in your mind...");
	wiz_light(TRUE);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_ENLIGHTENMENT2(effect_handler_context_t *context)
{
	msg("You begin to feel more enlightened...");
	message_flush();
	wiz_light(TRUE);
	(void)do_inc_stat(A_INT);
	(void)do_inc_stat(A_WIS);
	(void)detect_traps(TRUE);
	(void)detect_doorstairs(TRUE);
	(void)detect_treasure(TRUE, TRUE);
	(void)detect_monsters_entire_level();
	identify_pack();
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_HERO(effect_handler_context_t *context)
{
	int dur = randint1(25) + 25;
	if (hp_player(10)) context->ident = TRUE;
	effect_clear_timed_one(context, TMD_AFRAID);
	effect_increment_timed_normal(context, TMD_BOLD, dur);
	effect_increment_timed_normal(context, TMD_HERO, dur);
	return TRUE;
}

bool effect_handler_SHERO(effect_handler_context_t *context)
{
	int dur = randint1(25) + 25;
	if (hp_player(30)) context->ident = TRUE;
	effect_clear_timed_one(context, TMD_AFRAID);
	effect_increment_timed_normal(context, TMD_BOLD, dur);
	effect_increment_timed_normal(context, TMD_SHERO, dur);
	return TRUE;
}

bool effect_handler_RESIST_ACID(effect_handler_context_t *context)
{
	return effect_increment_timed_normal(context, TMD_OPP_ACID, randint1(10) + 10);
}

bool effect_handler_RESIST_ELEC(effect_handler_context_t *context)
{
	return effect_increment_timed_normal(context, TMD_OPP_ELEC, randint1(10) + 10);
}

bool effect_handler_RESIST_FIRE(effect_handler_context_t *context)
{
	return effect_increment_timed_normal(context, TMD_OPP_FIRE, randint1(10) + 10);
}

bool effect_handler_RESIST_COLD(effect_handler_context_t *context)
{
	return effect_increment_timed_normal(context, TMD_OPP_COLD, randint1(10) + 10);
}

bool effect_handler_RESIST_POIS(effect_handler_context_t *context)
{
	return effect_increment_timed_normal(context, TMD_OPP_POIS, randint1(10) + 10);
}

bool effect_handler_RESIST_ALL(effect_handler_context_t *context)
{
	effect_increment_timed_normal(context, TMD_OPP_ACID, randint1(20) + 20);
	effect_increment_timed_normal(context, TMD_OPP_ELEC, randint1(20) + 20);
	effect_increment_timed_normal(context, TMD_OPP_FIRE, randint1(20) + 20);
	effect_increment_timed_normal(context, TMD_OPP_COLD, randint1(20) + 20);
	effect_increment_timed_normal(context, TMD_OPP_POIS, randint1(20) + 20);
	return TRUE;
}

bool effect_handler_DETECT_TREASURE(effect_handler_context_t *context)
{
	if (detect_treasure(context->aware, FALSE)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DETECT_TRAP(effect_handler_context_t *context)
{
	if (detect_traps(context->aware)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DETECT_DOORSTAIR(effect_handler_context_t *context)
{
	if (detect_doorstairs(context->aware)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DETECT_INVIS(effect_handler_context_t *context)
{
	if (detect_monsters_invis(context->aware)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DETECT_EVIL(effect_handler_context_t *context)
{
	if (detect_monsters_evil(context->aware)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DETECT_ALL(effect_handler_context_t *context)
{
	if (detect_all(context->aware)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_ENCHANT_TOHIT(effect_handler_context_t *context)
{
	context->ident = TRUE;
	return enchant_spell(1, 0, 0);
}

bool effect_handler_ENCHANT_TODAM(effect_handler_context_t *context)
{
	context->ident = TRUE;
	return enchant_spell(0, 1, 0);
}

bool effect_handler_ENCHANT_WEAPON(effect_handler_context_t *context)
{
	context->ident = TRUE;
	return enchant_spell(randint1(3), randint1(3), 0);
}

bool effect_handler_ENCHANT_ARMOR(effect_handler_context_t *context)
{
	context->ident = TRUE;
	return enchant_spell(0, 0, 1);
}

bool effect_handler_ENCHANT_ARMOR2(effect_handler_context_t *context)
{
	context->ident = TRUE;
	return enchant_spell(0, 0, randint1(3) + 2);
}

bool effect_handler_RESTORE_ITEM(effect_handler_context_t *context)
{
	context->ident = TRUE;
	return restore_item();
}

bool effect_handler_IDENTIFY(effect_handler_context_t *context)
{
	context->ident = TRUE;
	return ident_spell();
}

bool effect_handler_REMOVE_CURSE(effect_handler_context_t *context)
{
	if (remove_curse())
	{
		if (!p_ptr->timed[TMD_BLIND])
			msg("The air around your body glows blue for a moment...");
		else
			msg("You feel as if someone is watching over you.");

		context->ident = TRUE;
	}
	return TRUE;
}

bool effect_handler_REMOVE_CURSE2(effect_handler_context_t *context)
{
	remove_all_curse();
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_LIGHT(effect_handler_context_t *context)
{
	if (light_area(damroll(2, 8), 2)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_SUMMON_MON(effect_handler_context_t *context)
{
	int i;
	sound(MSG_SUM_MONSTER);

	for (i = 0; i < randint1(3); i++)
	{
		if (summon_specific(p_ptr->py, p_ptr->px, p_ptr->depth, 0, 1))
			context->ident = TRUE;
	}
	return TRUE;
}

bool effect_handler_SUMMON_UNDEAD(effect_handler_context_t *context)
{
	int i;
	sound(MSG_SUM_UNDEAD);

	for (i = 0; i < randint1(3); i++)
	{
		if (summon_specific(p_ptr->py, p_ptr->px, p_ptr->depth,
							S_UNDEAD, 1))
			context->ident = TRUE;
	}
	return TRUE;
}

bool effect_handler_TELE_PHASE(effect_handler_context_t *context)
{
	teleport_player(10);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_TELE_LONG(effect_handler_context_t *context)
{
	teleport_player(100);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_TELE_LEVEL(effect_handler_context_t *context)
{
	(void)teleport_player_level();
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_CONFUSING(effect_handler_context_t *context)
{
	if (p_ptr->confusing == 0)
	{
		msg("Your hands begin to glow.");
		p_ptr->confusing = TRUE;
		context->ident = TRUE;
	}
	return TRUE;
}

bool effect_handler_MAPPING(effect_handler_context_t *context)
{
	map_area();
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_RUNE(effect_handler_context_t *context)
{
	warding_glyph();
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_ACQUIRE(effect_handler_context_t *context)
{
	acquirement(p_ptr->py, p_ptr->px, p_ptr->depth, 1, TRUE);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_ACQUIRE2(effect_handler_context_t *context)
{
	acquirement(p_ptr->py, p_ptr->px, p_ptr->depth, randint1(2) + 1, TRUE);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_ANNOY_MON(effect_handler_context_t *context)
{
	msg("There is a high pitched humming noise.");
	aggravate_monsters(0);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_CREATE_TRAP(effect_handler_context_t *context)
{
	/* Hack -- no traps in the town */
	if (p_ptr->depth == 0)
		return TRUE;

	trap_creation();
	msg("You hear a low-pitched whistling sound.");
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DESTROY_TDOORS(effect_handler_context_t *context)
{
	if (destroy_doors_touch()) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_RECHARGE(effect_handler_context_t *context)
{
	context->ident = TRUE;
	return recharge(60);
}

bool effect_handler_BANISHMENT(effect_handler_context_t *context)
{
	context->ident = TRUE;
	return banishment();
}

bool effect_handler_DARKNESS(effect_handler_context_t *context)
{
	if (!check_state(p_ptr, OF_RES_DARK, p_ptr->state.flags))
		(void)player_inc_timed(p_ptr, TMD_BLIND, 3 + randint1(5), TRUE, TRUE);
	unlight_area(10, 3);
	wieldeds_notice_flag(p_ptr, OF_RES_DARK);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_PROTEVIL(effect_handler_context_t *context)
{
	if (player_inc_timed(p_ptr, TMD_PROTEVIL, randint1(25) + 3 *
						 p_ptr->lev, TRUE, TRUE)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_SATISFY(effect_handler_context_t *context)
{
	if (player_set_food(p_ptr, PY_FOOD_MAX - 1)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_CURSE_WEAPON(effect_handler_context_t *context)
{
	if (curse_weapon()) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_CURSE_ARMOR(effect_handler_context_t *context)
{
	if (curse_armor()) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_BLESSING(effect_handler_context_t *context)
{
	return effect_increment_timed_normal(context, TMD_BLESSED, randint1(12) + 6);
}

bool effect_handler_BLESSING2(effect_handler_context_t *context)
{
	return effect_increment_timed_normal(context, TMD_BLESSED, randint1(24) + 12	);
}

bool effect_handler_BLESSING3(effect_handler_context_t *context)
{
	return effect_increment_timed_normal(context, TMD_BLESSED, randint1(48) + 24);
}

bool effect_handler_RECALL(effect_handler_context_t *context)
{
	context->ident = TRUE;
	return set_recall();
}

bool effect_handler_DEEP_DESCENT(effect_handler_context_t *context)
{
	int i, target_depth = p_ptr->max_depth;

	/* Calculate target depth */
	for (i = 5; i > 0; i--) {
		if (is_quest(target_depth)) break;
		if (target_depth >= MAX_DEPTH - 1) break;

		target_depth++;
	}

	if (target_depth > p_ptr->depth) {
		msgt(MSG_TPLEVEL, "The air around you starts to swirl...");
		p_ptr->deep_descent = 3 + randint1(4);
		context->ident = TRUE;
		return TRUE;
	} else {
		msgt(MSG_TPLEVEL, "You sense a malevolent presence blocking passage to the levels below.");
		context->ident = TRUE;
		return FALSE;
	}
}

bool effect_handler_LOSHASTE(effect_handler_context_t *context)
{
	if (speed_monsters()) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_LOSSLEEP(effect_handler_context_t *context)
{
	if (sleep_monsters(context->aware)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_LOSSLOW(effect_handler_context_t *context)
{
	if (slow_monsters()) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_LOSCONF(effect_handler_context_t *context)
{
	if (confuse_monsters(context->aware)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_LOSKILL(effect_handler_context_t *context)
{
	(void)mass_banishment();
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_EARTHQUAKES(effect_handler_context_t *context)
{
	earthquake(p_ptr->py, p_ptr->px, 10);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DESTRUCTION2(effect_handler_context_t *context)
{
	destroy_area(p_ptr->py, p_ptr->px, 15, TRUE);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_ILLUMINATION(effect_handler_context_t *context)
{
	if (light_area(damroll(2, 15), 3)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_CLAIRVOYANCE(effect_handler_context_t *context)
{
	context->ident = TRUE;
	wiz_light(FALSE);
	(void)detect_traps(TRUE);
	(void)detect_doorstairs(TRUE);
	return TRUE;
}

bool effect_handler_PROBING(effect_handler_context_t *context)
{
	context->ident = probing();
	return TRUE;
}

bool effect_handler_STONE_TO_MUD(effect_handler_context_t *context)
{
	if (wall_to_mud(context->dir)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_CONFUSE2(effect_handler_context_t *context)
{
	context->ident = TRUE;
	confuse_monster(context->dir, 20, context->aware);
	return TRUE;
}

bool effect_handler_BIZARRE(effect_handler_context_t *context)
{
	context->ident = TRUE;
	ring_of_power(context->dir);
	return TRUE;
}

bool effect_handler_STAR_BALL(effect_handler_context_t *context)
{
	int i;
	context->ident = TRUE;
	for (i = 0; i < 8; i++) fire_ball(GF_ELEC, ddd[i],
									  (150 * (100 + context->boost) / 100), 3);
	return TRUE;
}

bool effect_handler_RAGE_BLESS_RESIST(effect_handler_context_t *context)
{
	int dur = randint1(50) + 50;
	context->ident = TRUE;
	(void)hp_player(30);
	(void)player_clear_timed(p_ptr, TMD_AFRAID, TRUE);
	(void)player_inc_timed(p_ptr, TMD_BOLD, dur, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_SHERO, dur, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_BLESSED, randint1(50) + 50, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_OPP_ACID, randint1(50) + 50, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_OPP_ELEC, randint1(50) + 50, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_OPP_FIRE, randint1(50) + 50, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_OPP_COLD, randint1(50) + 50, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_OPP_POIS, randint1(50) + 50, TRUE, TRUE);
	return TRUE;
}

bool effect_handler_SLEEPII(effect_handler_context_t *context)
{
	context->ident = TRUE;
	sleep_monsters_touch(context->aware);
	return TRUE;
}

bool effect_handler_RESTORE_LIFE(effect_handler_context_t *context)
{
	context->ident = TRUE;
	restore_level();
	return TRUE;
}

bool effect_handler_MISSILE(effect_handler_context_t *context)
{
	int dam = damroll(3, 4) * (100 + context->boost) / 100;
	return effect_project_bolt_or_beam(context, GF_MISSILE, dam);
}

bool effect_handler_DISPEL_EVIL(effect_handler_context_t *context)
{
	context->ident = TRUE;
	int dam = p_ptr->lev * 5 * (100 + context->boost) / 100;
	dispel_evil(dam);
	return TRUE;
}

bool effect_handler_DISPEL_EVIL60(effect_handler_context_t *context)
{
	int dam = 60 * (100 + context->boost) / 100;
	if (dispel_evil(dam)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DISPEL_UNDEAD(effect_handler_context_t *context)
{
	int dam = 60 * (100 + context->boost) / 100;
	if (dispel_undead(dam)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DISPEL_ALL(effect_handler_context_t *context)
{
	int dam = 120 * (100 + context->boost) / 100;
	if (dispel_monsters(dam)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_HASTE(effect_handler_context_t *context)
{
	if (!p_ptr->timed[TMD_FAST])
	{
		if (player_set_timed(p_ptr, TMD_FAST, damroll(2, 10) + 20, TRUE)) context->ident = TRUE;
	}
	else
	{
		(void)player_inc_timed(p_ptr, TMD_FAST, 5, TRUE, TRUE);
	}

	return TRUE;
}

bool effect_handler_HASTE1(effect_handler_context_t *context)
{
	if (!p_ptr->timed[TMD_FAST])
	{
		if (player_set_timed(p_ptr, TMD_FAST, randint1(20) + 20, TRUE)) context->ident = TRUE;
	}
	else
	{
		(void)player_inc_timed(p_ptr, TMD_FAST, 5, TRUE, TRUE);
	}

	return TRUE;
}

bool effect_handler_HASTE2(effect_handler_context_t *context)
{
	if (!p_ptr->timed[TMD_FAST])
	{
		if (player_set_timed(p_ptr, TMD_FAST, randint1(75) + 75, TRUE)) context->ident = TRUE;
	}
	else
	{
		(void)player_inc_timed(p_ptr, TMD_FAST, 5, TRUE, TRUE);
	}

	return TRUE;
}

bool effect_handler_FIRE_BOLT(effect_handler_context_t *context)
{
	int dam = damroll(9, 8) * (100 + context->boost) / 100;
	return effect_project_bolt_only(context, GF_FIRE, dam);
}

bool effect_handler_FIRE_BOLT2(effect_handler_context_t *context)
{
	int dam = damroll(12, 8) * (100 + context->boost) / 100;
	return effect_project_bolt_or_beam(context, GF_FIRE, dam);
}

bool effect_handler_FIRE_BOLT3(effect_handler_context_t *context)
{
	int dam = damroll(16, 8) * (100 + context->boost) / 100;
	return effect_project_bolt_or_beam(context, GF_FIRE, dam);
}

bool effect_handler_FIRE_BOLT72(effect_handler_context_t *context)
{
	int dam = 72 * (100 + context->boost) / 100;
	return effect_project_ball(context, GF_FIRE, dam, 2);
}

bool effect_handler_FIRE_BALL(effect_handler_context_t *context)
{
	int dam = 144 * (100 + context->boost) / 100;
	return effect_project_ball(context, GF_FIRE, dam, 2);
}

bool effect_handler_FIRE_BALL2(effect_handler_context_t *context)
{
	int dam = 120 * (100 + context->boost) / 100;
	return effect_project_ball(context, GF_FIRE, dam, 3);
}

bool effect_handler_FIRE_BALL200(effect_handler_context_t *context)
{
	int dam = 200 * (100 + context->boost) / 100;
	return effect_project_ball(context, GF_FIRE, dam, 3);
}

bool effect_handler_COLD_BOLT(effect_handler_context_t *context)
{
	int dam = damroll(6, 8) * (100 + context->boost) / 100;
	return effect_project_bolt_or_beam(context, GF_COLD, dam);
}

bool effect_handler_COLD_BOLT2(effect_handler_context_t *context)
{
	int dam = damroll(12, 8) * (100 + context->boost) / 100;
	return effect_project_bolt_only(context, GF_COLD, dam);
}

bool effect_handler_COLD_BALL2(effect_handler_context_t *context)
{
	int dam = 200 * (100 + context->boost) / 100;
	return effect_project_ball(context, GF_COLD, dam, 3);
}

bool effect_handler_COLD_BALL50(effect_handler_context_t *context)
{
	int dam = 50 * (100 + context->boost) / 100;
	return effect_project_ball(context, GF_COLD, dam, 2);
}

bool effect_handler_COLD_BALL100(effect_handler_context_t *context)
{
	int dam = 100 * (100 + context->boost) / 100;
	return effect_project_ball(context, GF_COLD, dam, 2);
}

bool effect_handler_COLD_BALL160(effect_handler_context_t *context)
{
	int dam = 160 * (100 + context->boost) / 100;
	return effect_project_ball(context, GF_COLD, dam, 3);
}

bool effect_handler_ACID_BOLT(effect_handler_context_t *context)
{
	int dam = damroll(5, 8) * (100 + context->boost) / 100;
	return effect_project_bolt_only(context, GF_ACID, dam);
}

bool effect_handler_ACID_BOLT2(effect_handler_context_t *context)
{
	int dam = damroll(10, 8) * (100 + context->boost) / 100;
	return effect_project_bolt_or_beam(context, GF_ACID, dam);
}

bool effect_handler_ACID_BOLT3(effect_handler_context_t *context)
{
	int dam = damroll(12, 8) * (100 + context->boost) / 100;
	return effect_project_bolt_or_beam(context, GF_ACID, dam);
}

bool effect_handler_ACID_BALL(effect_handler_context_t *context)
{
	int dam = 120 * (100 + context->boost) / 100;
	return effect_project_ball(context, GF_ACID, dam, 2);
}

bool effect_handler_ELEC_BOLT(effect_handler_context_t *context)
{
	int dam = damroll(6, 6) * (100 + context->boost) / 100;
	context->ident = TRUE;
	fire_beam(GF_ELEC, context->dir, dam);
	return TRUE;
}

bool effect_handler_ELEC_BALL(effect_handler_context_t *context)
{
	int dam = 64 * (100 + context->boost) / 100;
	return effect_project_ball(context, GF_ELEC, dam, 2);
}

bool effect_handler_ELEC_BALL2(effect_handler_context_t *context)
{
	int dam = 250 * (100 + context->boost) / 100;
	return effect_project_ball(context, GF_ELEC, dam, 3);
}

bool effect_handler_ARROW(effect_handler_context_t *context)
{
	int dam = 150 * (100 + context->boost) / 100;
	return effect_project_bolt_only(context, GF_ARROW, dam);
}

bool effect_handler_REM_FEAR_POIS(effect_handler_context_t *context)
{
	context->ident = TRUE;
	(void)player_clear_timed(p_ptr, TMD_AFRAID, TRUE);
	(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
	return TRUE;
}

bool effect_handler_STINKING_CLOUD(effect_handler_context_t *context)
{
	int dam = 12 * (100 + context->boost) / 100;
	return effect_project_ball(context, GF_POIS, dam, 3);
}

bool effect_handler_DRAIN_LIFE1(effect_handler_context_t *context)
{
	int dam = 90 * (100 + context->boost) / 100;
	if (drain_life(context->dir, dam)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DRAIN_LIFE2(effect_handler_context_t *context)
{
	int dam = 120 * (100 + context->boost) / 100;
	if (drain_life(context->dir, dam)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DRAIN_LIFE3(effect_handler_context_t *context)
{
	int dam = 150 * (100 + context->boost) / 100;
	if (drain_life(context->dir, dam)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DRAIN_LIFE4(effect_handler_context_t *context)
{
	int dam = 250 * (100 + context->boost) / 100;
	if (drain_life(context->dir, dam)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_FIREBRAND(effect_handler_context_t *context)
{
	context->ident = TRUE;
	if (!brand_bolts()) return FALSE;
	return TRUE;
}

bool effect_handler_MANA_BOLT(effect_handler_context_t *context)
{
	int dam = damroll(12, 8) * (100 + context->boost) / 100;
	return effect_project_bolt_only(context, GF_MANA, dam);
}

bool effect_handler_MON_HEAL(effect_handler_context_t *context)
{
	if (heal_monster(context->dir)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_MON_HASTE(effect_handler_context_t *context)
{
	if (speed_monster(context->dir)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_MON_SLOW(effect_handler_context_t *context)
{
	if (slow_monster(context->dir)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_MON_CONFUSE(effect_handler_context_t *context)
{
	if (confuse_monster(context->dir, 10, context->aware)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_MON_SLEEP(effect_handler_context_t *context)
{
	if (sleep_monster(context->dir, context->aware)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_MON_CLONE(effect_handler_context_t *context)
{
	if (clone_monster(context->dir)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_MON_SCARE(effect_handler_context_t *context)
{
	if (fear_monster(context->dir, 10, context->aware)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_LIGHT_LINE(effect_handler_context_t *context)
{
	msg("A line of shimmering blue light appears.");
	light_line(context->dir);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_TELE_OTHER(effect_handler_context_t *context)
{
	if (teleport_monster(context->dir)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DISARMING(effect_handler_context_t *context)
{
	if (disarm_trap(context->dir)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_TDOOR_DEST(effect_handler_context_t *context)
{
	if (destroy_door(context->dir)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_POLYMORPH(effect_handler_context_t *context)
{
	if (poly_monster(context->dir)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_STARLIGHT(effect_handler_context_t *context)
{
	int i;
	if (!p_ptr->timed[TMD_BLIND])
		msg("Light shoots in all context->directions!");
	for (i = 0; i < 8; i++) light_line(ddd[i]);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_STARLIGHT2(effect_handler_context_t *context)
{
	int k;
	for (k = 0; k < 8; k++) strong_light_line(ddd[k]);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_BERSERKER(effect_handler_context_t *context)
{
	int dur = randint1(50) + 50;
	effect_increment_timed_normal(context, TMD_BOLD, dur);
	effect_increment_timed_normal(context, TMD_SHERO, dur);
	return TRUE;
}

bool effect_handler_WONDER(effect_handler_context_t *context)
{
	if (effect_wonder(context->dir, randint1(100) + p_ptr->lev / 5,
					  context->beam)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_WAND_BREATH(effect_handler_context_t *context)
{
	/* table of random ball effects and their damages */
	const int breath_types[] = {
		GF_ACID, 200,
		GF_ELEC, 160,
		GF_FIRE, 200,
		GF_COLD, 160,
		GF_POIS, 120
	};
	/* pick a random (type, damage) tuple in the table */
	int which = 2 * randint0(sizeof(breath_types) / (2 * sizeof(int)));
	fire_ball(breath_types[which], context->dir, breath_types[which + 1], 3);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_STAFF_MAGI(effect_handler_context_t *context)
{
	if (do_res_stat(A_INT)) context->ident = TRUE;
	if (p_ptr->csp < p_ptr->msp)
	{
		p_ptr->csp = p_ptr->msp;
		p_ptr->csp_frac = 0;
		context->ident = TRUE;
		msg("Your feel your head clear.");
		p_ptr->redraw |= (PR_MANA);
	}
	return TRUE;
}

bool effect_handler_STAFF_HOLY(effect_handler_context_t *context)
{
	int dam = 120 * (100 + context->boost) / 100;
	if (dispel_evil(dam)) context->ident = TRUE;
	if (hp_player(50)) context->ident = TRUE;
	effect_increment_timed_normal(context, TMD_PROTEVIL, randint1(25) + 3 * p_ptr->lev);
	effect_clear_timed_multiple(context,
								TMD_POISONED, TMD_TERROR, TMD_AFRAID,
								TMD_STUN, TMD_CUT, TMD_SLOW, TMD_BLIND,
								TMD_CONFUSED, TMD_IMAGE, TMD_AMNESIA, EFFECT_STOP);
	return TRUE;
}

bool effect_handler_DRINK_BREATH(effect_handler_context_t *context)
{
	const int breath_types[] =
	{
		GF_FIRE, 80,
		GF_COLD, 80,
	};

	int which = 2 * randint0(N_ELEMENTS(breath_types) / 2);
	fire_ball(breath_types[which], context->dir, breath_types[which + 1], 2);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DRINK_GOOD(effect_handler_context_t *context)
{
	msg("You feel less thirsty.");
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DRINK_DEATH(effect_handler_context_t *context)
{
	msg("A feeling of Death flows through your body.");
	take_hit(p_ptr, 5000, "a potion of Death");
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DRINK_RUIN(effect_handler_context_t *context)
{
	msg("Your nerves and muscles feel weak and lifeless!");
	take_hit(p_ptr, damroll(10, 10), "a potion of Ruination");
	player_stat_dec(p_ptr, A_DEX, TRUE);
	player_stat_dec(p_ptr, A_WIS, TRUE);
	player_stat_dec(p_ptr, A_CON, TRUE);
	player_stat_dec(p_ptr, A_STR, TRUE);
	player_stat_dec(p_ptr, A_INT, TRUE);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DRINK_DETONATE(effect_handler_context_t *context)
{
	msg("Massive explosions rupture your body!");
	take_hit(p_ptr, damroll(50, 20), "a potion of Detonation");
	(void)player_inc_timed(p_ptr, TMD_STUN, 75, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_CUT, 5000, TRUE, TRUE);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_DRINK_SALT(effect_handler_context_t *context)
{
	msg("The potion makes you vomit!");
	player_set_food(p_ptr, PY_FOOD_STARVE - 1);
	(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
	(void)player_inc_timed(p_ptr, TMD_PARALYZED, 4, TRUE, FALSE);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_FOOD_GOOD(effect_handler_context_t *context)
{
	msg("That tastes good.");
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_FOOD_WAYBREAD(effect_handler_context_t *context)
{
	msg("That tastes good.");
	(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
	(void)hp_player(damroll(4, 8));
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_FOOD_CRUNCH(effect_handler_context_t *context)
{
	if (one_in_(2))
		msg("It's crunchy.");
	else
		msg("It nearly breaks your tooth!");
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_FOOD_WHISKY(effect_handler_context_t *context)
{
	msg("That tastes great!");
	(void)player_inc_timed(p_ptr, TMD_CONFUSED, randint0(5), TRUE, TRUE);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_FOOD_WINE(effect_handler_context_t *context)
{
	msg("That tastes great!  A fine vintage.");
	player_set_timed(p_ptr, TMD_BOLD, rand_spread(100, 20), TRUE);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_SHROOM_EMERGENCY(effect_handler_context_t *context)
{
	(void)player_set_timed(p_ptr, TMD_IMAGE, rand_spread(250, 50), TRUE);
	(void)player_set_timed(p_ptr, TMD_OPP_FIRE, rand_spread(30, 10), TRUE);
	(void)player_set_timed(p_ptr, TMD_OPP_COLD, rand_spread(30, 10), TRUE);
	(void)hp_player(200);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_SHROOM_TERROR(effect_handler_context_t *context)
{
	if (player_set_timed(p_ptr, TMD_TERROR, rand_spread(100, 20), TRUE))
		context->ident = TRUE;
	return TRUE;
}

bool effect_handler_SHROOM_STONE(effect_handler_context_t *context)
{
	if (player_set_timed(p_ptr, TMD_STONESKIN, rand_spread(80, 20), TRUE))
		context->ident = TRUE;
	return TRUE;
}

bool effect_handler_SHROOM_DEBILITY(effect_handler_context_t *context)
{
	int stat = one_in_(2) ? A_STR : A_CON;

	if (p_ptr->csp < p_ptr->msp)
	{
		p_ptr->csp = p_ptr->msp;
		p_ptr->csp_frac = 0;
		msg("Your feel your head clear.");
		p_ptr->redraw |= (PR_MANA);
		context->ident = TRUE;
	}

	(void)do_dec_stat(stat, FALSE);

	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_SHROOM_SPRINTING(effect_handler_context_t *context)
{
	return effect_increment_timed_normal(context, TMD_SPRINT, 100);
}

bool effect_handler_SHROOM_PURGING(effect_handler_context_t *context)
{
	player_set_food(p_ptr, PY_FOOD_FAINT - 1);
	if (do_res_stat(A_STR)) context->ident = TRUE;
	if (do_res_stat(A_CON)) context->ident = TRUE;
	if (player_clear_timed(p_ptr, TMD_POISONED, TRUE)) context->ident = TRUE;
	return TRUE;
}

bool effect_handler_RING_ACID(effect_handler_context_t *context)
{
	int dam = 70 * (100 + context->boost) / 100;
	context->ident = TRUE;
	fire_ball(GF_ACID, context->dir, dam, 2);
	player_inc_timed(p_ptr, TMD_OPP_ACID, randint1(20) + 20, TRUE, TRUE);
	return TRUE;
}

bool effect_handler_RING_FLAMES(effect_handler_context_t *context)
{
	int dam = 80 * (100 + context->boost) / 100;
	context->ident = TRUE;
	fire_ball(GF_FIRE, context->dir, dam, 2);
	player_inc_timed(p_ptr, TMD_OPP_FIRE, randint1(20) + 20, TRUE, TRUE);
	return TRUE;
}

bool effect_handler_RING_ICE(effect_handler_context_t *context)
{
	int dam = 75 * (100 + context->boost) / 100;
	context->ident = TRUE;
	fire_ball(GF_COLD, context->dir, dam, 2);
	player_inc_timed(p_ptr, TMD_OPP_COLD, randint1(20) + 20, TRUE, TRUE);
	return TRUE;
}

bool effect_handler_RING_LIGHTNING(effect_handler_context_t *context)
{
	int dam = 85 * (100 + context->boost) / 100;
	context->ident = TRUE;
	fire_ball(GF_ELEC, context->dir, dam, 2);
	player_inc_timed(p_ptr, TMD_OPP_ELEC, randint1(20) + 20, TRUE, TRUE);
	return TRUE;
}

bool effect_handler_DRAGON_BLUE(effect_handler_context_t *context)
{
	int dam = 150 * (100 + context->boost) / 100;
	return effect_breathe_one(context, dam, GF_ELEC, MSG_BR_ELEC, "lightning");
}

bool effect_handler_DRAGON_GREEN(effect_handler_context_t *context)
{
	int dam = 150 * (100 + context->boost) / 100;
	return effect_breathe_one(context, dam, GF_POIS, MSG_BR_GAS, "poison gas");
}

bool effect_handler_DRAGON_RED(effect_handler_context_t *context)
{
	int dam = 200 * (100 + context->boost) / 100;
	return effect_breathe_one(context, dam, GF_FIRE, MSG_BR_FIRE, "fire");
}

bool effect_handler_DRAGON_MULTIHUED(effect_handler_context_t *context)
{
	static const effect_breath_info_t multihued[] = {
		{ MSG_BR_ELEC,	GF_ELEC,		"lightning" },
		{ MSG_BR_FROST,	GF_COLD,		"frost" },
		{ MSG_BR_ACID,	GF_ACID,		"acid" },
		{ MSG_BR_GAS,	GF_POIS,		"poison gas" },
		{ MSG_BR_FIRE,	GF_FIRE,		"fire" },
	};

	int dam = 250 * (100 + context->boost) / 100;
	return effect_breathe_random(context, dam, multihued, N_ELEMENTS(multihued));
}

bool effect_handler_DRAGON_BRONZE(effect_handler_context_t *context)
{
	int dam = 150 * (100 + context->boost) / 100;
	return effect_breathe_one(context, dam, GF_CONFU, MSG_BR_CONF, "confusion");
}

bool effect_handler_DRAGON_GOLD(effect_handler_context_t *context)
{
	int dam = 150 * (100 + context->boost) / 100;
	return effect_breathe_one(context, dam, GF_SOUND, MSG_BR_SOUND, "sound");
}

bool effect_handler_DRAGON_CHAOS(effect_handler_context_t *context)
{
	static const effect_breath_info_t chaos[] = {
		{ MSG_BR_DISEN,	GF_DISEN,	"disenchantment" },
		{ MSG_BR_CHAOS,	GF_CHAOS,	"chaos" },
	};

	int dam = 220 * (100 + context->boost) / 100;
	return effect_breathe_random(context, dam, chaos, N_ELEMENTS(chaos));
}

bool effect_handler_DRAGON_LAW(effect_handler_context_t *context)
{
	static const effect_breath_info_t law[] = {
		{ MSG_BR_SHARDS,		GF_SHARD,	"shards" },
		{ MSG_BR_SOUND,		GF_SOUND,	"sound" },
	};

	int dam = 230 * (100 + context->boost) / 100;
	return effect_breathe_random(context, dam, law, N_ELEMENTS(law));
}

bool effect_handler_DRAGON_BALANCE(effect_handler_context_t *context)
{
	static const effect_breath_info_t balance[] = {
		{ MSG_BR_DISEN,		GF_DISEN,	"disenchantment" },
		{ MSG_BR_CHAOS,		GF_CHAOS,	"chaos" },
		{ MSG_BR_SHARDS,		GF_SHARD,	"shards" },
		{ MSG_BR_SOUND,		GF_SOUND,	"sound" },
	};

	int dam = 250 * (100 + context->boost) / 100;
	return effect_breathe_random(context, dam, balance, N_ELEMENTS(balance));
}

bool effect_handler_DRAGON_SHINING(effect_handler_context_t *context)
{
	static const effect_breath_info_t shining[] = {
		{ MSG_BR_LIGHT,	GF_LIGHT,	"light" },
		{ MSG_BR_DARK,	GF_DARK,		"darkness" },
	};

	int dam = 200 * (100 + context->boost) / 100;
	return effect_breathe_random(context, dam, shining, N_ELEMENTS(shining));
}

bool effect_handler_DRAGON_POWER(effect_handler_context_t *context)
{
	int dam = 300 * (100 + context->boost) / 100;
	return effect_breathe_one(context, dam, GF_MISSILE, MSG_BR_ELEMENTS, "the elements");
}

bool effect_handler_TRAP_DOOR(effect_handler_context_t *context)
{
	msg("You fall through a trap door!");
	if (check_state(p_ptr, OF_FEATHER, p_ptr->state.flags)) {
		msg("You float gently down to the next level.");
	} else {
		take_hit(p_ptr, damroll(2, 8), "a trap");
	}
	wieldeds_notice_flag(p_ptr, OF_FEATHER);

	dungeon_change_level(p_ptr->depth + 1);
	return TRUE;
}

bool effect_handler_TRAP_PIT(effect_handler_context_t *context)
{
	msg("You fall into a pit!");
	if (check_state(p_ptr, OF_FEATHER, p_ptr->state.flags)) {
		msg("You float gently to the bottom of the pit.");
	} else {
		take_hit(p_ptr, damroll(2, 6), "a trap");
	}
	wieldeds_notice_flag(p_ptr, OF_FEATHER);
	return TRUE;
}

bool effect_handler_TRAP_PIT_SPIKES(effect_handler_context_t *context)
{
	msg("You fall into a spiked pit!");

	if (check_state(p_ptr, OF_FEATHER, p_ptr->state.flags)) {
		msg("You float gently to the floor of the pit.");
		msg("You carefully avoid touching the spikes.");
	} else {
		int dam = damroll(2, 6);

		/* Extra spike damage */
		if (one_in_(2)) {
			msg("You are impaled!");
			dam *= 2;
			(void)player_inc_timed(p_ptr, TMD_CUT, randint1(dam), TRUE, TRUE);
		}

		take_hit(p_ptr, dam, "a trap");
	}
	wieldeds_notice_flag(p_ptr, OF_FEATHER);
	return TRUE;
}

bool effect_handler_TRAP_PIT_POISON(effect_handler_context_t *context)
{
	msg("You fall into a spiked pit!");

	if (check_state(p_ptr, OF_FEATHER, p_ptr->state.flags)) {
		msg("You float gently to the floor of the pit.");
		msg("You carefully avoid touching the spikes.");
	} else {
		int dam = damroll(2, 6);

		/* Extra spike damage */
		if (one_in_(2)) {
			msg("You are impaled on poisonous spikes!");
			(void)player_inc_timed(p_ptr, TMD_CUT, randint1(dam * 2), TRUE, TRUE);
			(void)player_inc_timed(p_ptr, TMD_POISONED, randint1(dam * 4), TRUE, TRUE);
		}

		take_hit(p_ptr, dam, "a trap");
	}
	wieldeds_notice_flag(p_ptr, OF_FEATHER);
	return TRUE;
}

bool effect_handler_TRAP_RUNE_SUMMON(effect_handler_context_t *context)
{
	int i;
	int num = 2 + randint1(3);

	msgt(MSG_SUM_MONSTER, "You are enveloped in a cloud of smoke!");

	/* Remove trap */
	sqinfo_off(cave->info[p_ptr->py][p_ptr->px], SQUARE_MARK);
	square_destroy_trap(cave, p_ptr->py, p_ptr->px);

	for (i = 0; i < num; i++)
		(void)summon_specific(p_ptr->py, p_ptr->px, p_ptr->depth, 0, 1);

	return TRUE;
}

bool effect_handler_TRAP_RUNE_TELEPORT(effect_handler_context_t *context)
{
	msg("You hit a teleport trap!");
	teleport_player(100);
	return TRUE;
}

bool effect_handler_TRAP_SPOT_FIRE(effect_handler_context_t *context)
{
	msg("You are enveloped in flames!");
	int dam = damroll(4, 6);
	dam = adjust_dam(p_ptr, GF_FIRE, dam, RANDOMISE,
					 check_for_resist(p_ptr, GF_FIRE, p_ptr->state.flags, TRUE));
	if (dam) {
		take_hit(p_ptr, dam, "a fire trap");
		inven_damage(p_ptr, GF_FIRE, MIN(dam * 5, 300));
	}
	return TRUE;
}

bool effect_handler_TRAP_SPOT_ACID(effect_handler_context_t *context)
{
	msg("You are splashed with acid!");
	int dam = damroll(4, 6);
	dam = adjust_dam(p_ptr, GF_ACID, dam, RANDOMISE,
					 check_for_resist(p_ptr, GF_ACID, p_ptr->state.flags, TRUE));
	if (dam) {
		take_hit(p_ptr, dam, "an acid trap");
		inven_damage(p_ptr, GF_ACID, MIN(dam * 5, 300));
	}
	return TRUE;
}

bool effect_handler_TRAP_DART_SLOW(effect_handler_context_t *context)
{
	if (trap_check_hit(125)) {
		msg("A small dart hits you!");
		take_hit(p_ptr, damroll(1, 4), "a trap");
		(void)player_inc_timed(p_ptr, TMD_SLOW, randint0(20) + 20, TRUE, FALSE);
	} else {
		msg("A small dart barely misses you.");
	}
	return TRUE;
}

bool effect_handler_TRAP_DART_LOSE_STR(effect_handler_context_t *context)
{
	if (trap_check_hit(125)) {
		msg("A small dart hits you!");
		take_hit(p_ptr, damroll(1, 4), "a trap");
		(void)do_dec_stat(A_STR, FALSE);
	} else {
		msg("A small dart barely misses you.");
	}
	return TRUE;
}

bool effect_handler_TRAP_DART_LOSE_DEX(effect_handler_context_t *context)
{
	if (trap_check_hit(125)) {
		msg("A small dart hits you!");
		take_hit(p_ptr, damroll(1, 4), "a trap");
		(void)do_dec_stat(A_DEX, FALSE);
	} else {
		msg("A small dart barely misses you.");
	}
	return TRUE;
}

bool effect_handler_TRAP_DART_LOSE_CON(effect_handler_context_t *context)
{
	if (trap_check_hit(125)) {
		msg("A small dart hits you!");
		take_hit(p_ptr, damroll(1, 4), "a trap");
		(void)do_dec_stat(A_CON, FALSE);
	} else {
		msg("A small dart barely misses you.");
	}
	return TRUE;
}

bool effect_handler_TRAP_GAS_BLIND(effect_handler_context_t *context)
{
	msg("You are surrounded by a black gas!");
	(void)player_inc_timed(p_ptr, TMD_BLIND, randint0(50) + 25, TRUE, TRUE);
	return TRUE;
}

bool effect_handler_TRAP_GAS_CONFUSE(effect_handler_context_t *context)
{
	msg("You are surrounded by a gas of scintillating colors!");
	(void)player_inc_timed(p_ptr, TMD_CONFUSED, randint0(20) + 10, TRUE, TRUE);
	return TRUE;
}

bool effect_handler_TRAP_GAS_POISON(effect_handler_context_t *context)
{
	msg("You are surrounded by a pungent green gas!");
	(void)player_inc_timed(p_ptr, TMD_POISONED, randint0(20) + 10, TRUE, TRUE);
	return TRUE;
}

bool effect_handler_TRAP_GAS_SLEEP(effect_handler_context_t *context)
{
	msg("You are surrounded by a strange white mist!");
	(void)player_inc_timed(p_ptr, TMD_PARALYZED, randint0(10) + 5, TRUE, TRUE);
	return TRUE;
}





typedef bool (*effect_handler_f)(effect_handler_context_t *);

effect_handler_f effect_handler_for_effect(effect_type effect)
{
	/* Effect handler table. Terminator is {(effect_type)-1, NULL}. */
	static const struct effect_handler_s {
		effect_type effect;
		effect_handler_f handler;
	} effect_handlers[] = {
		{ EF_POISON, effect_handler_POISON },
		{ EF_BLIND, effect_handler_BLIND },
		{ EF_SCARE, effect_handler_SCARE },
		{ EF_CONFUSE, effect_handler_CONFUSE },
		{ EF_HALLUC, effect_handler_HALLUC },
		{ EF_PARALYZE, effect_handler_PARALYZE },
		{ EF_SLOW, effect_handler_SLOW },
		{ EF_CURE_POISON, effect_handler_CURE_POISON },
		{ EF_CURE_BLINDNESS, effect_handler_CURE_BLINDNESS },
		{ EF_CURE_PARANOIA, effect_handler_CURE_PARANOIA },
		{ EF_CURE_CONFUSION, effect_handler_CURE_CONFUSION },
		{ EF_CURE_MIND, effect_handler_CURE_MIND },
		{ EF_CURE_BODY, effect_handler_CURE_BODY },
		{ EF_CURE_LIGHT, effect_handler_CURE_LIGHT },
		{ EF_CURE_SERIOUS, effect_handler_CURE_SERIOUS },
		{ EF_CURE_CRITICAL, effect_handler_CURE_CRITICAL },
		{ EF_CURE_FULL, effect_handler_CURE_FULL },
		{ EF_CURE_FULL2, effect_handler_CURE_FULL2 },
		{ EF_CURE_TEMP, effect_handler_CURE_TEMP },
		{ EF_HEAL1, effect_handler_HEAL1 },
		{ EF_HEAL2, effect_handler_HEAL2 },
		{ EF_HEAL3, effect_handler_HEAL3 },
		{ EF_GAIN_EXP, effect_handler_GAIN_EXP },
		{ EF_LOSE_EXP, effect_handler_LOSE_EXP },
		{ EF_RESTORE_EXP, effect_handler_RESTORE_EXP },
		{ EF_RESTORE_MANA, effect_handler_RESTORE_MANA },
		{ EF_GAIN_STR, effect_handler_GAIN_STR },
		{ EF_GAIN_INT, effect_handler_GAIN_INT },
		{ EF_GAIN_WIS, effect_handler_GAIN_WIS },
		{ EF_GAIN_DEX, effect_handler_GAIN_DEX },
		{ EF_GAIN_CON, effect_handler_GAIN_CON },
		{ EF_GAIN_ALL, effect_handler_GAIN_ALL },
		{ EF_BRAWN, effect_handler_BRAWN },
		{ EF_INTELLECT, effect_handler_INTELLECT },
		{ EF_CONTEMPLATION, effect_handler_CONTEMPLATION },
		{ EF_TOUGHNESS, effect_handler_TOUGHNESS },
		{ EF_NIMBLENESS, effect_handler_NIMBLENESS },
		{ EF_LOSE_STR, effect_handler_LOSE_STR },
		{ EF_LOSE_INT, effect_handler_LOSE_INT },
		{ EF_LOSE_WIS, effect_handler_LOSE_WIS },
		{ EF_LOSE_DEX, effect_handler_LOSE_DEX },
		{ EF_LOSE_CON, effect_handler_LOSE_CON },
		{ EF_LOSE_CON2, effect_handler_LOSE_CON2 },
		{ EF_RESTORE_STR, effect_handler_RESTORE_STR },
		{ EF_RESTORE_INT, effect_handler_RESTORE_INT },
		{ EF_RESTORE_WIS, effect_handler_RESTORE_WIS },
		{ EF_RESTORE_DEX, effect_handler_RESTORE_DEX },
		{ EF_RESTORE_CON, effect_handler_RESTORE_CON },
		{ EF_CURE_NONORLYBIG, effect_handler_CURE_NONORLYBIG },
		{ EF_RESTORE_ALL, effect_handler_RESTORE_ALL },
		{ EF_RESTORE_ST_LEV, effect_handler_RESTORE_ST_LEV },
		{ EF_TMD_INFRA, effect_handler_TMD_INFRA },
		{ EF_TMD_SINVIS, effect_handler_TMD_SINVIS },
		{ EF_TMD_ESP, effect_handler_TMD_ESP },
		{ EF_ENLIGHTENMENT, effect_handler_ENLIGHTENMENT },
		{ EF_ENLIGHTENMENT2, effect_handler_ENLIGHTENMENT2 },
		{ EF_HERO, effect_handler_HERO },
		{ EF_SHERO, effect_handler_SHERO },
		{ EF_RESIST_ACID, effect_handler_RESIST_ACID },
		{ EF_RESIST_ELEC, effect_handler_RESIST_ELEC },
		{ EF_RESIST_FIRE, effect_handler_RESIST_FIRE },
		{ EF_RESIST_COLD, effect_handler_RESIST_COLD },
		{ EF_RESIST_POIS, effect_handler_RESIST_POIS },
		{ EF_RESIST_ALL, effect_handler_RESIST_ALL },
		{ EF_DETECT_TREASURE, effect_handler_DETECT_TREASURE },
		{ EF_DETECT_TRAP, effect_handler_DETECT_TRAP },
		{ EF_DETECT_DOORSTAIR, effect_handler_DETECT_DOORSTAIR },
		{ EF_DETECT_INVIS, effect_handler_DETECT_INVIS },
		{ EF_DETECT_EVIL, effect_handler_DETECT_EVIL },
		{ EF_DETECT_ALL, effect_handler_DETECT_ALL },
		{ EF_ENCHANT_TOHIT, effect_handler_ENCHANT_TOHIT },
		{ EF_ENCHANT_TODAM, effect_handler_ENCHANT_TODAM },
		{ EF_ENCHANT_WEAPON, effect_handler_ENCHANT_WEAPON },
		{ EF_ENCHANT_ARMOR, effect_handler_ENCHANT_ARMOR },
		{ EF_ENCHANT_ARMOR2, effect_handler_ENCHANT_ARMOR2 },
		{ EF_RESTORE_ITEM, effect_handler_RESTORE_ITEM },
		{ EF_IDENTIFY, effect_handler_IDENTIFY },
		{ EF_REMOVE_CURSE, effect_handler_REMOVE_CURSE },
		{ EF_REMOVE_CURSE2, effect_handler_REMOVE_CURSE2 },
		{ EF_LIGHT, effect_handler_LIGHT },
		{ EF_SUMMON_MON, effect_handler_SUMMON_MON },
		{ EF_SUMMON_UNDEAD, effect_handler_SUMMON_UNDEAD },
		{ EF_TELE_PHASE, effect_handler_TELE_PHASE },
		{ EF_TELE_LONG, effect_handler_TELE_LONG },
		{ EF_TELE_LEVEL, effect_handler_TELE_LEVEL },
		{ EF_CONFUSING, effect_handler_CONFUSING },
		{ EF_MAPPING, effect_handler_MAPPING },
		{ EF_RUNE, effect_handler_RUNE },
		{ EF_ACQUIRE, effect_handler_ACQUIRE },
		{ EF_ACQUIRE2, effect_handler_ACQUIRE2 },
		{ EF_ANNOY_MON, effect_handler_ANNOY_MON },
		{ EF_CREATE_TRAP, effect_handler_CREATE_TRAP },
		{ EF_DESTROY_TDOORS, effect_handler_DESTROY_TDOORS },
		{ EF_RECHARGE, effect_handler_RECHARGE },
		{ EF_BANISHMENT, effect_handler_BANISHMENT },
		{ EF_DARKNESS, effect_handler_DARKNESS },
		{ EF_PROTEVIL, effect_handler_PROTEVIL },
		{ EF_SATISFY, effect_handler_SATISFY },
		{ EF_CURSE_WEAPON, effect_handler_CURSE_WEAPON },
		{ EF_CURSE_ARMOR, effect_handler_CURSE_ARMOR },
		{ EF_BLESSING, effect_handler_BLESSING },
		{ EF_BLESSING2, effect_handler_BLESSING2 },
		{ EF_BLESSING3, effect_handler_BLESSING3 },
		{ EF_RECALL, effect_handler_RECALL },
		{ EF_DEEP_DESCENT, effect_handler_DEEP_DESCENT },
		{ EF_LOSHASTE, effect_handler_LOSHASTE },
		{ EF_LOSSLEEP, effect_handler_LOSSLEEP },
		{ EF_LOSSLOW, effect_handler_LOSSLOW },
		{ EF_LOSCONF, effect_handler_LOSCONF },
		{ EF_LOSKILL, effect_handler_LOSKILL },
		{ EF_EARTHQUAKES, effect_handler_EARTHQUAKES },
		{ EF_DESTRUCTION2, effect_handler_DESTRUCTION2 },
		{ EF_ILLUMINATION, effect_handler_ILLUMINATION },
		{ EF_CLAIRVOYANCE, effect_handler_CLAIRVOYANCE },
		{ EF_PROBING, effect_handler_PROBING },
		{ EF_STONE_TO_MUD, effect_handler_STONE_TO_MUD },
		{ EF_CONFUSE2, effect_handler_CONFUSE2 },
		{ EF_BIZARRE, effect_handler_BIZARRE },
		{ EF_STAR_BALL, effect_handler_STAR_BALL },
		{ EF_RAGE_BLESS_RESIST, effect_handler_RAGE_BLESS_RESIST },
		{ EF_SLEEPII, effect_handler_SLEEPII },
		{ EF_RESTORE_LIFE, effect_handler_RESTORE_LIFE },
		{ EF_MISSILE, effect_handler_MISSILE },
		{ EF_DISPEL_EVIL, effect_handler_DISPEL_EVIL },
		{ EF_DISPEL_EVIL60, effect_handler_DISPEL_EVIL60 },
		{ EF_DISPEL_UNDEAD, effect_handler_DISPEL_UNDEAD },
		{ EF_DISPEL_ALL, effect_handler_DISPEL_ALL },
		{ EF_HASTE, effect_handler_HASTE },
		{ EF_HASTE1, effect_handler_HASTE1 },
		{ EF_HASTE2, effect_handler_HASTE2 },
		{ EF_FIRE_BOLT, effect_handler_FIRE_BOLT },
		{ EF_FIRE_BOLT2, effect_handler_FIRE_BOLT2 },
		{ EF_FIRE_BOLT3, effect_handler_FIRE_BOLT3 },
		{ EF_FIRE_BOLT72, effect_handler_FIRE_BOLT72 },
		{ EF_FIRE_BALL, effect_handler_FIRE_BALL },
		{ EF_FIRE_BALL2, effect_handler_FIRE_BALL2 },
		{ EF_FIRE_BALL200, effect_handler_FIRE_BALL200 },
		{ EF_COLD_BOLT, effect_handler_COLD_BOLT },
		{ EF_COLD_BOLT2, effect_handler_COLD_BOLT2 },
		{ EF_COLD_BALL2, effect_handler_COLD_BALL2 },
		{ EF_COLD_BALL50, effect_handler_COLD_BALL50 },
		{ EF_COLD_BALL100, effect_handler_COLD_BALL100 },
		{ EF_COLD_BALL160, effect_handler_COLD_BALL160 },
		{ EF_ACID_BOLT, effect_handler_ACID_BOLT },
		{ EF_ACID_BOLT2, effect_handler_ACID_BOLT2 },
		{ EF_ACID_BOLT3, effect_handler_ACID_BOLT3 },
		{ EF_ACID_BALL, effect_handler_ACID_BALL },
		{ EF_ELEC_BOLT, effect_handler_ELEC_BOLT },
		{ EF_ELEC_BALL, effect_handler_ELEC_BALL },
		{ EF_ELEC_BALL2, effect_handler_ELEC_BALL2 },
		{ EF_ARROW, effect_handler_ARROW },
		{ EF_REM_FEAR_POIS, effect_handler_REM_FEAR_POIS },
		{ EF_STINKING_CLOUD, effect_handler_STINKING_CLOUD },
		{ EF_DRAIN_LIFE1, effect_handler_DRAIN_LIFE1 },
		{ EF_DRAIN_LIFE2, effect_handler_DRAIN_LIFE2 },
		{ EF_DRAIN_LIFE3, effect_handler_DRAIN_LIFE3 },
		{ EF_DRAIN_LIFE4, effect_handler_DRAIN_LIFE4 },
		{ EF_FIREBRAND, effect_handler_FIREBRAND },
		{ EF_MANA_BOLT, effect_handler_MANA_BOLT },
		{ EF_MON_HEAL, effect_handler_MON_HEAL },
		{ EF_MON_HASTE, effect_handler_MON_HASTE },
		{ EF_MON_SLOW, effect_handler_MON_SLOW },
		{ EF_MON_CONFUSE, effect_handler_MON_CONFUSE },
		{ EF_MON_SLEEP, effect_handler_MON_SLEEP },
		{ EF_MON_CLONE, effect_handler_MON_CLONE },
		{ EF_MON_SCARE, effect_handler_MON_SCARE },
		{ EF_LIGHT_LINE, effect_handler_LIGHT_LINE },
		{ EF_TELE_OTHER, effect_handler_TELE_OTHER },
		{ EF_DISARMING, effect_handler_DISARMING },
		{ EF_TDOOR_DEST, effect_handler_TDOOR_DEST },
		{ EF_POLYMORPH, effect_handler_POLYMORPH },
		{ EF_STARLIGHT, effect_handler_STARLIGHT },
		{ EF_STARLIGHT2, effect_handler_STARLIGHT2 },
		{ EF_BERSERKER, effect_handler_BERSERKER },
		{ EF_WONDER, effect_handler_WONDER },
		{ EF_WAND_BREATH, effect_handler_WAND_BREATH },
		{ EF_STAFF_MAGI, effect_handler_STAFF_MAGI },
		{ EF_STAFF_HOLY, effect_handler_STAFF_HOLY },
		{ EF_DRINK_BREATH, effect_handler_DRINK_BREATH },
		{ EF_DRINK_GOOD, effect_handler_DRINK_GOOD },
		{ EF_DRINK_DEATH, effect_handler_DRINK_DEATH },
		{ EF_DRINK_RUIN, effect_handler_DRINK_RUIN },
		{ EF_DRINK_DETONATE, effect_handler_DRINK_DETONATE },
		{ EF_DRINK_SALT, effect_handler_DRINK_SALT },
		{ EF_FOOD_GOOD, effect_handler_FOOD_GOOD },
		{ EF_FOOD_WAYBREAD, effect_handler_FOOD_WAYBREAD },
		{ EF_FOOD_CRUNCH, effect_handler_FOOD_CRUNCH },
		{ EF_FOOD_WHISKY, effect_handler_FOOD_WHISKY },
		{ EF_FOOD_WINE, effect_handler_FOOD_WINE },
		{ EF_SHROOM_EMERGENCY, effect_handler_SHROOM_EMERGENCY },
		{ EF_SHROOM_TERROR, effect_handler_SHROOM_TERROR },
		{ EF_SHROOM_STONE, effect_handler_SHROOM_STONE },
		{ EF_SHROOM_DEBILITY, effect_handler_SHROOM_DEBILITY },
		{ EF_SHROOM_SPRINTING, effect_handler_SHROOM_SPRINTING },
		{ EF_SHROOM_PURGING, effect_handler_SHROOM_PURGING },
		{ EF_RING_ACID, effect_handler_RING_ACID },
		{ EF_RING_FLAMES, effect_handler_RING_FLAMES },
		{ EF_RING_ICE, effect_handler_RING_ICE },
		{ EF_RING_LIGHTNING, effect_handler_RING_LIGHTNING },
		{ EF_DRAGON_BLUE, effect_handler_DRAGON_BLUE },
		{ EF_DRAGON_GREEN, effect_handler_DRAGON_GREEN },
		{ EF_DRAGON_RED, effect_handler_DRAGON_RED },
		{ EF_DRAGON_MULTIHUED, effect_handler_DRAGON_MULTIHUED },
		{ EF_DRAGON_BRONZE, effect_handler_DRAGON_BRONZE },
		{ EF_DRAGON_GOLD, effect_handler_DRAGON_GOLD },
		{ EF_DRAGON_CHAOS, effect_handler_DRAGON_CHAOS },
		{ EF_DRAGON_LAW, effect_handler_DRAGON_LAW },
		{ EF_DRAGON_BALANCE, effect_handler_DRAGON_BALANCE },
		{ EF_DRAGON_SHINING, effect_handler_DRAGON_SHINING },
		{ EF_DRAGON_POWER, effect_handler_DRAGON_POWER },
		{ EF_TRAP_DOOR, effect_handler_TRAP_DOOR },
		{ EF_TRAP_PIT, effect_handler_TRAP_PIT },
		{ EF_TRAP_PIT_SPIKES, effect_handler_TRAP_PIT_SPIKES },
		{ EF_TRAP_PIT_POISON, effect_handler_TRAP_PIT_POISON },
		{ EF_TRAP_RUNE_SUMMON, effect_handler_TRAP_RUNE_SUMMON },
		{ EF_TRAP_RUNE_TELEPORT, effect_handler_TRAP_RUNE_TELEPORT },
		{ EF_TRAP_SPOT_FIRE, effect_handler_TRAP_SPOT_FIRE },
		{ EF_TRAP_SPOT_ACID, effect_handler_TRAP_SPOT_ACID },
		{ EF_TRAP_DART_SLOW, effect_handler_TRAP_DART_SLOW },
		{ EF_TRAP_DART_LOSE_STR, effect_handler_TRAP_DART_LOSE_STR },
		{ EF_TRAP_DART_LOSE_DEX, effect_handler_TRAP_DART_LOSE_DEX },
		{ EF_TRAP_DART_LOSE_CON, effect_handler_TRAP_DART_LOSE_CON },
		{ EF_TRAP_GAS_BLIND, effect_handler_TRAP_GAS_BLIND },
		{ EF_TRAP_GAS_CONFUSE, effect_handler_TRAP_GAS_CONFUSE },
		{ EF_TRAP_GAS_POISON, effect_handler_TRAP_GAS_POISON },
		{ EF_TRAP_GAS_SLEEP, effect_handler_TRAP_GAS_SLEEP },
		{ EF_XXX, NULL },
		{ (effect_type)-1, NULL },
	};
	const struct effect_handler_s *current = effect_handlers;

	if (effect < 1 || effect >= EF_MAX)
		return NULL;

	while (current->effect != (effect_type)-1 && current->handler != NULL) {
		if (effect == current->effect)
			return current->handler;

		current++;
	}

	return NULL;
}


/*
 * Do an effect, given an object.
 * Boost is the extent to which skill surpasses difficulty, used as % boost. It
 * ranges from 0 to 138.
 */
bool effect_do(effect_type effect, bool *ident, bool aware, int dir, int beam, int boost)
{
	bool handled = FALSE;

	if (effect < 1 || effect > EF_MAX) {
		msg("Bad effect passed to do_effect().  Please report this bug.");
		return FALSE;
	}

	effect_handler_f effect_handler = effect_handler_for_effect(effect);

	if (effect_handler != NULL) {
		effect_handler_context_t context = {
			effect,
			aware,
			dir,
			beam,
			boost,
			*ident,
		};

		handled = effect_handler(&context);
		*ident = context.ident;
	}

	if (!handled)
		msg("Effect not handled.");

	return handled;
}
