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

#include "sshterm.h"
#include "menus.h"
#include "term-gc.h"

#include <intuition/menuclass.h>
#include <diskfont/diskfonttag.h>
#include <classes/window.h>
#include <gadgets/layout.h>

#include <shl-ring.h>

#include "SSHTerm_rev.h"

struct TermWindow {
	struct Screen         *Screen;
	APTR                   VisualInfo;
	APTR                   MenuStrip;
	struct MsgPort        *AppPort;
	Object                *Window;
	Object                *Layout;
	Object                *Term;
	struct Hook            IDCMPHook;
	struct Hook            TermHook;
	struct shl_ring        RingBuffer;
	UWORD                  Columns;
	UWORD                  Rows;
	BOOL                   NewSize:1;
};

enum {
	MID_INVALID,
	MID_PROJECT_MENU,
	MID_PROJECT_ICONIFY,
	MID_PROJECT_ABOUT,
	MID_PROJECT_CLEARSB,
	MID_PROJECT_CLOSE,
	MID_EDIT_MENU,
	MID_EDIT_COPY,
	MID_EDIT_COPYALL,
	MID_EDIT_PASTE,
	MID_PALETTE_MENU,
	MID_PALETTE_DEFAULT,
	MID_PALETTE_SOLARIZED,
	MID_PALETTE_SOLARIZED_BLACK,
	MID_PALETTE_SOLARIZED_WHITE,
	MID_PALETTE_SOFT_BLACK,
	MID_PALETTE_BASE16_DARK,
	MID_PALETTE_BASE16_LIGHT
};

static inline ULONG GET(Object *obj, ULONG attr)
{
	ULONG result = 0;

	IIntuition->GetAttr(attr, obj, &result);

	return result;
}

static inline ULONG DGM(Object *obj, Object *winobj, Msg msg)
{
	struct Window *window;

	window = (struct Window *)GET(winobj, WINDOW_Window);

	return IIntuition->DoGadgetMethodA((struct Gadget *)obj, window, NULL, msg);
}

static ULONG term_hook_cb(struct Hook *hook, Object *term, struct TermHookMsg *thm);
static ULONG term_idcmp_cb(struct Hook *hook, Object *winobj, struct IntuiMessage *imsg);

static const struct NewMenu newmenus[] =
{
	{ NM_TITLE, "Project", NULL, 0, 0, (APTR)MID_PROJECT_MENU },
	{ NM_ITEM, "Iconify", "I", 0, 0, (APTR)MID_PROJECT_ICONIFY },
	{ NM_ITEM, "About...", "?", 0, 0, (APTR)MID_PROJECT_ABOUT },
	{ NM_ITEM, NM_BARLABEL, NULL, 0, 0, NULL },
	{ NM_ITEM, "Clear Scrollback", NULL, 0, 0, (APTR)MID_PROJECT_CLEARSB },
	{ NM_ITEM, NM_BARLABEL, NULL, 0, 0, NULL },
	{ NM_ITEM, "Close", "K", 0, 0, (APTR)MID_PROJECT_CLOSE },
	{ NM_TITLE, "Edit", NULL, 0, 0, (APTR)MID_EDIT_MENU },
	{ NM_ITEM, "Copy", "C", 0, 0, (APTR)MID_EDIT_COPY },
	{ NM_ITEM, "Copy All", NULL, 0, 0, (APTR)MID_EDIT_COPYALL },
	{ NM_ITEM, "Paste", "V", 0, 0, (APTR)MID_EDIT_PASTE },
	{ NM_TITLE, "Palette", NULL, 0, 0, (APTR)MID_PALETTE_MENU },
	{ NM_ITEM, "Default", NULL, CHECKIT | CHECKED, ~1, (APTR)MID_PALETTE_DEFAULT },
	{ NM_ITEM, "Solarized", NULL, CHECKIT, ~2, (APTR)MID_PALETTE_SOLARIZED },
	{ NM_ITEM, "Solarized Black", NULL, CHECKIT, ~4, (APTR)MID_PALETTE_SOLARIZED_BLACK },
	{ NM_ITEM, "Solarized White", NULL, CHECKIT, ~8, (APTR)MID_PALETTE_SOLARIZED_WHITE },
	{ NM_ITEM, "Soft Black", NULL, CHECKIT, ~16, (APTR)MID_PALETTE_SOFT_BLACK },
	{ NM_ITEM, "Base16 Dark", NULL, CHECKIT, ~32, (APTR)MID_PALETTE_BASE16_DARK },
	{ NM_ITEM, "Base16 Light", NULL, CHECKIT, ~64, (APTR)MID_PALETTE_BASE16_LIGHT },
	{ NM_END, NULL, NULL, 0, 0, NULL }
};

struct TermWindow *termwin_open(struct Screen *screen, ULONG max_sb)
{
	struct TermWindow *tw;
	Object *scroller;

	if (screen == NULL)
		return NULL;

	tw = malloc(sizeof(*tw));
	if (tw == NULL)
		return NULL;

	memset(tw, 0, sizeof(*tw));

	tw->Screen = screen;

	if (MenuClass == NULL)
	{
		tw->VisualInfo = IGadTools->GetVisualInfoA(tw->Screen, NULL);
	}

	tw->MenuStrip = create_menu(newmenus, tw->VisualInfo,
		NM_Menu, "Project", MA_ID, MID_PROJECT_MENU,
		NM_Item, "Iconify", MA_ID, MID_PROJECT_ICONIFY, MA_Key, "I",
		NM_Item, "About...", MA_ID, MID_PROJECT_ABOUT, MA_Key, "?",
		NM_Item, ML_SEPARATOR,
		NM_Item, "Clear Scrollback", MA_ID, MID_PROJECT_CLEARSB,
		NM_Item, ML_SEPARATOR,
		NM_Item, "Close", MA_ID, MID_PROJECT_CLOSE, MA_Key, "K",
		NM_Menu, "Edit", MA_ID, MID_EDIT_MENU,
		NM_Item, "Copy", MA_ID, MID_EDIT_COPY, MA_Key, "C",
		NM_Item, "Copy All", MA_ID, MID_EDIT_COPYALL,
		NM_Item, "Paste", MA_ID, MID_EDIT_PASTE, MA_Key, "V",
		NM_Menu, "Palette", MA_ID, MID_PALETTE_MENU,
		NM_Item, "Default", MA_ID, MID_PALETTE_DEFAULT, MA_MX, ~1, MA_Selected, TRUE,
		NM_Item, "Solarized", MA_ID, MID_PALETTE_SOLARIZED, MA_MX, ~2,
		NM_Item, "Solarized Black", MA_ID, MID_PALETTE_SOLARIZED_BLACK, MA_MX, ~4,
		NM_Item, "Solarized White", MA_ID, MID_PALETTE_SOLARIZED_WHITE, MA_MX, ~8,
		NM_Item, "Soft Black", MA_ID, MID_PALETTE_SOFT_BLACK, MA_MX, ~16,
		NM_Item, "Base16 Dark", MA_ID, MID_PALETTE_BASE16_DARK, MA_MX, ~32,
		NM_Item, "Base16 Light", MA_ID, MID_PALETTE_BASE16_LIGHT, MA_MX, ~64,
		TAG_END);
	if (tw->MenuStrip == NULL)
	{
		termwin_close(tw);
		return NULL;
	}

	tw->AppPort = IExec->AllocSysObject(ASOT_PORT, NULL);
	if (tw->AppPort == NULL)
	{
		termwin_close(tw);
		return NULL;
	}

	memset(&tw->TermHook, 0, sizeof(tw->TermHook));
	tw->TermHook.h_Entry = (HOOKFUNC)term_hook_cb;
	tw->TermHook.h_Data  = tw;

	tw->Term = IIntuition->NewObject(TermClass, NULL,
		TERM_UserHook, &tw->TermHook,
		TAG_END);

	tw->Layout = IIntuition->NewObject(LayoutClass, NULL,
		LAYOUT_DeferLayout, TRUE,
		LAYOUT_SpaceOuter,  FALSE,
		LAYOUT_AddChild,    tw->Term,
		TAG_END);

	memset(&tw->IDCMPHook, 0, sizeof(tw->IDCMPHook));
	tw->IDCMPHook.h_Entry = (HOOKFUNC)term_idcmp_cb;
	tw->IDCMPHook.h_Data  = tw;

	tw->Window = IIntuition->NewObject(WindowClass, NULL,
		WA_PubScreen,         tw->Screen,
		WA_Title,             VERS,
		WA_Flags,             WFLG_ACTIVATE | WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET |
		                      WFLG_SIZEGADGET | WFLG_NEWLOOKMENUS | WFLG_NOCAREREFRESH,
		WA_IDCMP,             IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_RAWKEY | IDCMP_MOUSEMOVE |
		                      IDCMP_MOUSEBUTTONS | IDCMP_EXTENDEDMOUSE,
		WINDOW_Position,      WPOS_CENTERSCREEN,
		WINDOW_BuiltInScroll, TRUE,
		WINDOW_VertProp,      TRUE,
		WINDOW_AppPort,       tw->AppPort,
		WINDOW_Icon,          AppIcon,
		WINDOW_IconNoDispose, TRUE,
		WINDOW_IconTitle,     "SSHTerm",
		WINDOW_IconifyGadget, TRUE,
		WINDOW_MenuStrip,     tw->MenuStrip,
		WINDOW_Layout,        tw->Layout,
		WINDOW_IDCMPHook,     &tw->IDCMPHook,
		WINDOW_IDCMPHookBits, IDCMP_MOUSEMOVE | IDCMP_MOUSEBUTTONS,
		TAG_END);

	if (tw->Window == NULL)
	{
		termwin_close(tw);
		return NULL;
	}

	scroller = (Object *)GET(tw->Window, WINDOW_VertObject);
	IIntuition->SetAttrs(tw->Term,
		TERM_MaxScrollback, max_sb,
		TERM_Scroller,      scroller,
		TAG_END);

	tw->Columns = (UWORD)GET(tw->Term, TERM_Columns);
	tw->Rows    = (UWORD)GET(tw->Term, TERM_Rows);

	if ((struct Window *)IIntuition->IDoMethod(tw->Window, WM_OPEN, NULL) == NULL)
	{
		termwin_close(tw);
		return NULL;
	}

	return tw;
}

void termwin_close(struct TermWindow *tw)
{
	if (tw != NULL)
	{
		if (tw->Window != NULL)
		{
			IIntuition->DisposeObject(tw->Window);
			tw->Window = NULL;
			tw->Layout = NULL;
			tw->Term   = NULL;
		}

		if (tw->AppPort != NULL)
		{
			IExec->FreeSysObject(ASOT_PORT, tw->AppPort);
			tw->AppPort = NULL;
		}

		if (tw->MenuStrip != NULL)
		{
			delete_menu(tw->MenuStrip);
			tw->MenuStrip = NULL;
		}

		if (tw->VisualInfo != NULL)
		{
			IGadTools->FreeVisualInfo(tw->VisualInfo);
			tw->VisualInfo = NULL;
		}

		free(tw);
	}
}

void termwin_set_max_sb(struct TermWindow *tw, ULONG max_sb)
{
	struct Window *window;

	window = (struct Window *)GET(tw->Window, WINDOW_Window);

	IIntuition->SetGadgetAttrs((struct Gadget *)tw->Term, window, NULL,
		TERM_MaxScrollback, max_sb,
		TAG_END);
}

void termwin_set_palette(struct TermWindow *tw, const char *palette)
{
	struct Window *window;

	window = (struct Window *)GET(tw->Window, WINDOW_Window);

	IIntuition->SetGadgetAttrs((struct Gadget *)tw->Term, window, NULL,
		TERM_BuiltInPalette, palette,
		TAG_END);
}

void termwin_write(struct TermWindow *tw, const char *buffer, size_t len)
{
	struct tpInput tpi;

	tpi.MethodID   = TM_INPUT;
	tpi.tpi_GInfo  = NULL;
	tpi.tpi_Data   = buffer;
	tpi.tpi_Length = len;

	//DGM(tw->Term, tw->Window, (Msg)&tpi);
	IIntuition->IDoMethodA(tw->Term, (Msg)&tpi);
}

void termwin_refresh(struct TermWindow *tw)
{
	struct tpInput tpi;

	tpi.MethodID   = TM_INPUT;
	tpi.tpi_GInfo  = NULL;
	tpi.tpi_Data   = NULL;
	tpi.tpi_Length = 0;

	DGM(tw->Term, tw->Window, (Msg)&tpi);
}

ULONG termwin_get_signals(struct TermWindow *tw)
{
	return GET(tw->Window, WINDOW_SigMask);
}

static ULONG term_hook_cb(struct Hook *hook, Object *term, struct TermHookMsg *thm)
{
	struct TermWindow *tw = hook->h_Data;
	int r;

	switch (thm->MethodID)
	{
		case THM_OUTPUT:
			r = shl_ring_push(&tw->RingBuffer, thm->tohm_Data, thm->tohm_Length);
			if (r < 0)
			{
				IExec->DebugPrintF("shl_ring_push: %d\n", r);
			}
			break;

		case THM_RESIZE:
			IExec->Forbid();
			tw->Columns = thm->trhm_Columns;
			tw->Rows    = thm->trhm_Rows;
			tw->NewSize = TRUE;
			IExec->Permit();
			break;

		case THM_BELL:
			IIntuition->DisplayBeep(tw->Screen);
			break;
	}

	return 0;
}

static ULONG term_idcmp_cb(struct Hook *hook, Object *winobj, struct IntuiMessage *imsg)
{
	struct TermWindow *tw = hook->h_Data;
	struct tpMouse tpm;

	tpm.MethodID   = TM_HANDLEMOUSE;
	tpm.tpm_GInfo  = NULL;
	tpm.tpm_MouseX = imsg->MouseX;
	tpm.tpm_MouseY = imsg->MouseY;

	tpm.tpm_Time.Seconds      = imsg->Seconds;
	tpm.tpm_Time.Microseconds = imsg->Micros;

	if (imsg->Class == IDCMP_MOUSEBUTTONS)
	{
		tpm.tpm_Button = imsg->Code;

		if (tpm.tpm_Button == SELECTDOWN)
			IIntuition->SetAttrs(winobj, WA_ReportMouse, TRUE, TAG_END);
		else if (tpm.tpm_Button == SELECTUP)
			IIntuition->SetAttrs(winobj, WA_ReportMouse, FALSE, TAG_END);
	}
	else
	{
		tpm.tpm_Button = 0;
	}

	DGM(tw->Term, tw->Window, (Msg)&tpm);

	return 0;
}

static BOOL termwin_iconify(struct TermWindow *tw)
{
	BOOL result = FALSE;

	if (IIntuition->IDoMethod(tw->Window, WM_ICONIFY, NULL))
	{
		result = TRUE;
	}

	return result;
}

static BOOL termwin_uniconify(struct TermWindow *tw)
{
	struct Window *window;
	BOOL result = FALSE;

	window = (struct Window *)IIntuition->IDoMethod(tw->Window, WM_OPEN, NULL);
	if (window != NULL)
	{
		result = TRUE;
	}

	return result;
}

BOOL termwin_handle_input(struct TermWindow *tw)
{
	ULONG result;
	UWORD code;
	struct MenuInputData mstate;
	ULONG mid;
	struct InputEvent *ie;
	struct tpKeyboard tpk;
	struct tpGeneric tpg;
	BOOL done = FALSE;

	static const char *const palette_name[] =
	{
		NULL,
		"solarized",
		"solarized-black",
		"solarized-white",
		"soft-black",
		"base16-dark",
		"base16-light"
	};

	while ((result = IIntuition->IDoMethod(tw->Window, WM_HANDLEINPUT, &code)) != WMHI_LASTMSG)
	{
		switch (result & WMHI_CLASSMASK)
		{
			case WMHI_CLOSEWINDOW:
				done = TRUE;
				break;

			case WMHI_ICONIFY:
				termwin_iconify(tw);
				break;

			case WMHI_UNICONIFY:
				termwin_uniconify(tw);
				break;

			case WMHI_MENUPICK:
				start_menu_input(tw->MenuStrip, &mstate, code);
				while ((mid = handle_menu_input(&mstate)) != NO_MENU_ID)
				{
					switch (mid)
					{
						case MID_PROJECT_ICONIFY:
							termwin_iconify(tw);
							break;

						case MID_PROJECT_ABOUT:
							aboutwin_open(tw->Screen);
							break;

						case MID_PROJECT_CLEARSB:
							tpg.MethodID  = TM_CLEARSB;
							tpg.tpg_GInfo = NULL;

							DGM(tw->Term, tw->Window, (Msg)&tpg);
							break;

						case MID_PROJECT_CLOSE:
							done = TRUE;
							break;

						case MID_EDIT_COPY:
							tpg.MethodID  = TM_COPY;
							tpg.tpg_GInfo = NULL;

							IIntuition->IDoMethodA(tw->Term, (Msg)&tpg);
							break;

						case MID_EDIT_COPYALL:
							tpg.MethodID  = TM_COPYALL;
							tpg.tpg_GInfo = NULL;

							IIntuition->IDoMethodA(tw->Term, (Msg)&tpg);
							break;

						case MID_EDIT_PASTE:
							tpg.MethodID  = TM_PASTE;
							tpg.tpg_GInfo = NULL;

							DGM(tw->Term, tw->Window, (Msg)&tpg);
							break;

						case MID_PALETTE_DEFAULT:
						case MID_PALETTE_SOLARIZED:
						case MID_PALETTE_SOLARIZED_BLACK:
						case MID_PALETTE_SOLARIZED_WHITE:
						case MID_PALETTE_SOFT_BLACK:
						case MID_PALETTE_BASE16_DARK:
						case MID_PALETTE_BASE16_LIGHT:
							termwin_set_palette(tw, palette_name[mid - MID_PALETTE_DEFAULT]);
							break;
					}
				}
				break;

			case WMHI_RAWKEY:
				ie = (struct InputEvent *)GET(tw->Window, WINDOW_InputEvent);

				tpk.MethodID   = TM_HANDLEKEYBOARD;
				tpk.tpk_GInfo  = NULL;
				tpk.tpk_IEvent = *ie;

				DGM(tw->Term, tw->Window, (Msg)&tpk);
				break;
		}
	}

	return done;
}

size_t termwin_poll(struct TermWindow *tw)
{
	return tw->RingBuffer.used;
}

ssize_t termwin_read(struct TermWindow *tw, char *buffer, size_t len)
{
	size_t n;

	n = shl_ring_copy(&tw->RingBuffer, buffer, len);
	if (n > 0)
	{
		shl_ring_pull(&tw->RingBuffer, n);
	}

	return n;
}

BOOL termwin_poll_new_size(struct TermWindow *tw)
{
	return tw->NewSize;
}

void termwin_get_size(struct TermWindow *tw, UWORD *columns, UWORD *rows)
{
	IExec->Forbid();
	tw->NewSize = FALSE;
	*columns = tw->Columns;
	*rows    = tw->Rows;
	IExec->Permit();
}

