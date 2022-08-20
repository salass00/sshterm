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

#include "menus.h"

#include <intuition/menuclass.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>

#include <stdarg.h>

extern Class *MenuClass;

APTR VARARGS68K create_menu(const struct NewMenu *nm, APTR vi, ...)
{
	APTR menustrip;
	BOOL success = FALSE;

	if (MenuClass != NULL)
	{
		menustrip = IIntuition->NewObject(MenuClass, NULL,
			MA_Type,        T_ROOT,
			MA_EmbeddedKey, FALSE,
			MA_FreeImage,   TRUE,
			TAG_END);
		if (menustrip != NULL)
		{
			struct mpNewMenu mpnm;
			va_list tags;

			va_startlinear(tags, vi);

			mpnm.MethodID      = MM_NEWMENU;
			mpnm.mpnm_Reserved = 0;
			mpnm.mpnm_AttrList = va_getlinearva(tags, struct TagItem *);

			success = IIntuition->IDoMethodA(menustrip, (Msg)&mpnm);

			va_end(tags);
		}
	}
	else
	{
		if (vi == NULL)
			return NULL;

		menustrip = IGadTools->CreateMenus(nm,
			GTMN_FullMenu, TRUE,
			TAG_END);
		if (menustrip != NULL)
		{
			success = IGadTools->LayoutMenus(menustrip, vi,
				GTMN_NewLookMenus, TRUE,
				TAG_END);
		}
	}

	if (!success)
	{
		delete_menu(menustrip);
		return NULL;
	}

	return menustrip;
}

void delete_menu(APTR menustrip)
{
	if (menustrip != NULL)
	{
		if (MenuClass != NULL)
			IIntuition->DisposeObject(menustrip);
		else
			IGadTools->FreeMenus(menustrip);
	}
}

void start_menu_input(APTR menu, struct MenuInputData *mid, UWORD code)
{
	mid->MenuStrip = menu;
	mid->MenuID    = NO_MENU_ID;
	mid->Code      = code;
}

ULONG handle_menu_input(struct MenuInputData *mid)
{
	if (MenuClass != NULL)
	{
		struct mpNextSelect mpns;

		mpns.MethodID       = MM_NEXTSELECT;
		mpns.mpns_Reserved  = 0;
		mpns.mpns_CurrentID = mid->MenuID;

		mid->MenuID = IIntuition->IDoMethodA(mid->MenuStrip, (Msg)&mpns);
	}
	else
	{
		struct MenuItem *item;

		item = IIntuition->ItemAddress(mid->MenuStrip, mid->Code);
		if (item != NULL)
		{
			mid->MenuID = (ULONG)GTMENUITEM_USERDATA(item);
			mid->Code   = item->NextSelect;
		}
		else
		{
			mid->MenuID = NO_MENU_ID;
			mid->Code   = MENUNULL;
		}
	}

	return mid->MenuID;
}

