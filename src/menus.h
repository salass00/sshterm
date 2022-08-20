/*
 * SSHTerm - SSH2 shell client
 *
 * Copyright (C) 2019-2022 Fredrik Wikstrom <fredrik@a500.org>
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the SSHTerm
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MENUS_H
#define MENUS_H

#include <libraries/gadtools.h>

struct MenuInputData {
	APTR  MenuStrip;
	ULONG MenuID;
	UWORD Code;
};

APTR VARARGS68K create_menu(const struct NewMenu *nm, APTR vi, ...);
void delete_menu(APTR menustrip);
void start_menu_input(APTR menu, struct MenuInputData *mid, UWORD code);
ULONG handle_menu_input(struct MenuInputData *mid);

#endif /* MENUS_H */

