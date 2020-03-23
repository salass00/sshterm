/*
 * SSHTerm - SSH2 shell client
 *
 * Copyright (C) 2019-2020 Fredrik Wikstrom <fredrik@a500.org>
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

#ifndef TERM_GC_H
#define TERM_GC_H

#include <intuition/classes.h>
#include <intuition/cghooks.h>

#define TERM_Dummy          (0x80840000)
#define TERM_UserHook       (TERM_Dummy + 1)
#define TERM_Columns        (TERM_Dummy + 2)
#define TERM_Rows           (TERM_Dummy + 3)
#define TERM_Font           (TERM_Dummy + 4)
#define TERM_MaxScrollback  (TERM_Dummy + 5)
#define TERM_Scroller       (TERM_Dummy + 6)
#define TERM_SBTop          (TERM_Dummy + 7)
#define TERM_SBVisible      (TERM_Dummy + 8)
#define TERM_SBTotal        (TERM_Dummy + 9)
#define TERM_BuiltInPalette (TERM_Dummy + 10)

#define TM_DUMMY           (0x840000)
#define TM_INPUT           (TM_DUMMY + 1)
#define TM_HANDLEKEYBOARD  (TM_DUMMY + 2)
#define TM_HANDLEMOUSE     (TM_DUMMY + 3)
#define TM_COPY            (TM_DUMMY + 4)
#define TM_PASTE           (TM_DUMMY + 5)
#define TM_CLEARSB         (TM_DUMMY + 6)
#define TM_COPYALL         (TM_DUMMY + 7)

struct tpInput
{
	ULONG              MethodID;
	struct GadgetInfo *tpi_GInfo;
	CONST_APTR         tpi_Data;
	ULONG              tpi_Length;
};

struct tpKeyboard
{
	ULONG              MethodID;
	struct GadgetInfo *tpk_GInfo;
	struct InputEvent  tpk_IEvent;
};

struct tpMouse
{
	ULONG              MethodID;
	struct GadgetInfo *tpm_GInfo;
	UWORD              tpm_Button;
	WORD               tpm_MouseX;
	WORD               tpm_MouseY;
	struct TimeVal     tpm_Time;
};

struct tpGeneric
{
	ULONG              MethodID;
	struct GadgetInfo *tpg_GInfo;
};

#define THM_DUMMY  (0x0)
#define THM_OUTPUT (THM_DUMMY + 1)
#define THM_RESIZE (THM_DUMMY + 2)
#define THM_BELL   (THM_DUMMY + 3)

struct TermHookMsg
{
	ULONG MethodID;
	union
	{
		/* THM_OUTPUT */
		struct
		{
			CONST_STRPTR tohm_Data;
			ULONG        tohm_Length;
		};
		/* THM_RESIZE */
		struct
		{
			UWORD        trhm_Columns;
			UWORD        trhm_Rows;
		};
	};
};

Class *init_term_gc(void);
void cleanup_term_gc(Class *cl);

#endif /* TERM_GC_H */

