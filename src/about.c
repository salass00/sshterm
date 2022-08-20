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

#include "sshterm.h"

#include <classes/window.h>
#include <gadgets/layout.h>
#include <gadgets/button.h>
#include <images/label.h>

#include "SSHTerm_rev.h"

enum {
	GID_DUMMY,
	GID_OK
};

ULONG AboutWindowPID = 0;

const TEXT gpl_license[] =
"This program is free software; you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation; either version 2 of the License, or\n"
"(at your option) any later version.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program; if not, write to the Free Software Foundation,\n"
"Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA";

static LONG aboutwin_procentry(void)
{
	TEXT              about_text[1024];
	struct TextFont  *font;
	struct TTextAttr *tta;
	struct Screen    *screen;
	Object           *label;
	Object           *labellayout;
	Object           *button;
	Object           *buttonlayout;
	Object           *layout;
	Object           *winobj;
	struct Window    *window;
	ULONG             sigmask, signals;
	BOOL              done;

	screen = (IExec->FindTask(NULL))->tc_UserData;

	font = ((struct GfxBase *)IGraphics->Data.LibBase)->DefaultFont;

	tta = IDiskfont->ObtainTTextAttr(font);
	if (tta == NULL)
	{
		return RETURN_ERROR;
	}

	snprintf(about_text, sizeof(about_text),
		"%s (%s)\n\n"
		"Copyright (C) 2019-2022 Fredrik Wikstrom <fredrik@a500.org>\n\n"
		"%s",
		VERS, DATE,
		gpl_license);

	label = IIntuition->NewObject(LabelClass, NULL,
		LABEL_Text,            about_text,
		LABEL_Justification,   LJ_CENTER,
		TAG_END);

	labellayout = IIntuition->NewObject(LayoutClass, NULL,
		GA_BackFill,           NULL,
		LAYOUT_SpaceOuter,     TRUE,
		LAYOUT_VertAlignment,  LALIGN_CENTER,
		LAYOUT_HorizAlignment, LALIGN_CENTER,
		LAYOUT_BevelStyle,     BVS_FIELD,
		LAYOUT_AddImage,       label,
		TAG_END);

	button = IIntuition->NewObject(ButtonClass, NULL,
		GA_ID,                 GID_OK,
		GA_RelVerify,          TRUE,
		GA_Text,               "OK",
		BUTTON_Justification,  BCJ_CENTER,
		TAG_END);

	buttonlayout = IIntuition->NewObject(LayoutClass, NULL,
		LAYOUT_HorizAlignment, LALIGN_CENTER,
		LAYOUT_AddChild,       button,
		CHILD_WeightedWidth,   0,
		TAG_END);

	layout = IIntuition->NewObject(LayoutClass, NULL,
		LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
		LAYOUT_AddChild,       labellayout,
		LAYOUT_AddChild,       buttonlayout,
		CHILD_WeightedHeight,  0,
		TAG_END);

	winobj = IIntuition->NewObject(WindowClass, NULL,
		WA_Title,         "About - SSHTerm",
		WA_PubScreen,     screen,
		WA_Activate,      TRUE,
		WA_CloseGadget,   TRUE,
		WA_DragBar,       TRUE,
		WA_DepthGadget,   TRUE,
		WA_SizeGadget,    TRUE,
		WA_NoCareRefresh, TRUE,
		WA_IDCMP,         IDCMP_CLOSEWINDOW | IDCMP_GADGETUP,
		WINDOW_Position,  WPOS_CENTERSCREEN,
		WINDOW_Layout,    layout,
		TAG_END);
	if (winobj == NULL)
	{
		IDiskfont->FreeTTextAttr(tta);
		return RETURN_ERROR;
	}

	window = (struct Window *)IIntuition->IDoMethod(winobj, WM_OPEN, NULL);
	if (window == NULL)
	{
		IIntuition->DisposeObject(winobj);
		IDiskfont->FreeTTextAttr(tta);
		return RETURN_ERROR;
	}

	done = FALSE;

	while (!done)
	{
		IIntuition->GetAttr(WINDOW_SigMask, winobj, &sigmask);
		signals = IExec->Wait(sigmask | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);

		if (signals & SIGBREAKF_CTRL_C)
			done = TRUE;

		if (signals & SIGBREAKF_CTRL_F)
		{
			window = (struct Window *)IIntuition->IDoMethod(winobj, WM_OPEN, NULL);
			if (window != NULL)
				IIntuition->ScreenToFront(window->WScreen);
		}

		if (signals & sigmask)
		{
			ULONG result;
			UWORD code;

			while ((result = IIntuition->IDoMethod(winobj, WM_HANDLEINPUT, &code)) != WMHI_LASTMSG)
			{
				switch (result & WMHI_CLASSMASK)
				{
					case WMHI_GADGETUP:
						if ((result & WMHI_GADGETMASK) == GID_OK)
							done = TRUE;
						break;

					case WMHI_CLOSEWINDOW:
						done = TRUE;
						break;
				}
			}
		}
	}

	IIntuition->DisposeObject(winobj);
	IDiskfont->FreeTTextAttr(tta);

	return RETURN_OK;
}

BOOL aboutwin_open(struct Screen *screen)
{
	ULONG pid;
	struct Process *proc;

	pid = AboutWindowPID;
	if (pid != 0)
	{
		/* Try to signal about window process */
		if (signal_pid(pid, SIGBREAKF_CTRL_F))
			return TRUE;

		/* About window process must have quit */
		AboutWindowPID = 0;
	}

	if (screen == NULL)
		return FALSE;

	proc = IDOS->CreateNewProcTags(
		NP_Name,                   "SSHTerm:About",
		NP_Entry,                  aboutwin_procentry,
		NP_Priority,               0,
		NP_Child,                  TRUE,
		NP_UserData,               screen,
		NP_CurrentDir,             ZERO,
		NP_Path,                   ZERO,
		NP_CopyVars,               FALSE,
		NP_Input,                  ZERO,
		NP_Output,                 ZERO,
		NP_Error,                  ZERO,
		NP_CloseInput,             FALSE,
		NP_CloseOutput,            FALSE,
		NP_CloseError,             FALSE,
		NP_NotifyOnDeathSigTask,   NULL,
		NP_NotifyOnDeathSignalBit, SIGB_CHILD,
		TAG_END);
	if (proc == NULL)
		return FALSE;

	/* IoErr() returns PID on CreateNewProc() success */
	AboutWindowPID = IDOS->IoErr();

	return TRUE;
}

void aboutwin_close(void)
{
	ULONG pid;

	pid = AboutWindowPID;
	if (pid != 0)
	{
		if (!signal_pid(pid, SIGBREAKF_CTRL_C))
			AboutWindowPID = 0;
	}
}

