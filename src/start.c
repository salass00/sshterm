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
#include "term-gc.h"

#include <workbench/startup.h>
#include <workbench/icon.h>
#include <libraries/amisslmaster.h>
#include <proto/amisslmaster.h>

#include <signal.h>
#include <locale.h>

#include "SSHTerm_rev.h"

static const TEXT USED verstag[] = VERSTAG;

#ifdef __CLIB2__
struct UtilityIFace        *IUtility;
#endif
struct DiskfontIFace       *IDiskfont;
struct GraphicsIFace       *IGraphics;
struct IntuitionIFace      *IIntuition;
struct AslIFace            *IAsl;
struct KeymapIFace         *IKeymap;
struct LayersIFace         *ILayers;
struct IconIFace           *IIcon;
struct ZIFace              *IZ;
struct SocketIFace         *ISocket;
struct AmiSSLMasterIFace   *IAmiSSLMaster;
struct AmiSSLIFace         *IAmiSSL;

Class                      *MenuClass;
struct GadToolsIFace       *IGadTools;
static struct ClassLibrary *WindowBase;
Class                      *WindowClass;
static struct ClassLibrary *LabelBase;
Class                      *LabelClass;
static struct ClassLibrary *LayoutBase;
Class                      *LayoutClass;
static struct ClassLibrary *ButtonBase;
Class                      *ButtonClass;
Class                      *TermClass;

struct DiskObject          *AppIcon;

#ifdef __CLIB2__
int errno;
int h_errno;
#define __errno() &errno
#define __h_errno() &h_errno
#endif

int main(int argc, char **argv)
{
	struct Library *AmiSSLBase = NULL;
	int             rc         = RETURN_ERROR;

	/* Disable newlib CTRL-C handling. */
	signal(SIGINT, SIG_IGN);

	/* Enable UTF-8 support in newlib. */
	setlocale(LC_CTYPE, "C-UTF-8");

	do
	{
		/* Open libraries, BOOPSI classes, etc. */

		#ifdef __CLIB2__
		IUtility = (struct UtilityIFace *)open_interface("utility.library", 53);
		if (IUtility == NULL)
		{
			break;
		}
		#endif

		IDiskfont = (struct DiskfontIFace *)open_interface("diskfont.library", 53);
		if (IDiskfont == NULL)
		{
			break;
		}

		IGraphics = (struct GraphicsIFace *)open_interface("graphics.library", 53);
		if (IGraphics == NULL)
		{
			break;
		}

		IIntuition = (struct IntuitionIFace *)open_interface("intuition.library", 53);
		if (IIntuition == NULL)
		{
			break;
		}

		IAsl = (struct AslIFace *)open_interface("asl.library", 53);
		if (IAsl == NULL)
		{
			break;
		}

		IKeymap = (struct KeymapIFace *)open_interface("keymap.library", 53);
		if (IKeymap == NULL)
		{
			break;
		}

		ILayers = (struct LayersIFace *)open_interface("layers.library", 53);
		if (ILayers == NULL)
		{
			break;
		}

		IIcon = (struct IconIFace *)open_interface("icon.library", 53);
		if (IIcon == NULL)
		{
			break;
		}

		IZ = (struct ZIFace *)open_interface("z.library", 53);
		if (IZ == NULL || !LIB_IS_AT_LEAST(IZ->Data.LibBase, 53, 5))
		{
			break;
		}

		ISocket = (struct SocketIFace *)open_interface("bsdsocket.library", 4);
		if (ISocket == NULL)
		{
			break;
		}

		if (ISocket->SocketBaseTags(
			SBTM_SETVAL(SBTC_ERRNOLONGPTR),            __errno(),
			SBTM_SETVAL(SBTC_HERRNOLONGPTR),           __h_errno(),
			SBTM_SETVAL(SBTC_CAN_SHARE_LIBRARY_BASES), TRUE,
			TAG_END))
		{
			break;
		}

		IAmiSSLMaster = (struct AmiSSLMasterIFace *)open_interface("amisslmaster.library", AMISSLMASTER_MIN_VERSION);
		if (IAmiSSLMaster == NULL)
		{
			break;
		}

		if (!IAmiSSLMaster->InitAmiSSLMaster(AMISSL_CURRENT_VERSION, TRUE))
		{
			break;
		}

		AmiSSLBase = IAmiSSLMaster->OpenAmiSSL();
		if (AmiSSLBase == NULL)
		{
			break;
		}

		IAmiSSL = (struct AmiSSLIFace *)IExec->GetInterface(AmiSSLBase, "main", 1, NULL);
		if (IAmiSSL == NULL)
		{
			break;
		}

		if (IAmiSSL->InitAmiSSL(
			AmiSSL_ErrNoPtr, __errno(),
			AmiSSL_ISocket,  ISocket,
			TAG_END))
		{
			break;
		}

		if (libssh2_init(0) != 0)
		{
			break;
		}

		MenuClass = IIntuition->FindClass("menuclass");
		if (MenuClass == NULL)
		{
			IGadTools = (struct GadToolsIFace *)open_interface("gadtools.library", 53);
			if (IGadTools == NULL)
			{
				break;
			}
		}

		WindowBase = IIntuition->OpenClass("window.class", 53, &WindowClass);
		if (WindowBase == NULL)
		{
			break;
		}

		LabelBase = IIntuition->OpenClass("images/label.image", 53, &LabelClass);
		if (LabelBase == NULL)
		{
			break;
		}

		LayoutBase = IIntuition->OpenClass("gadgets/layout.gadget", 53, &LayoutClass);
		if (LayoutBase == NULL)
		{
			break;
		}

		ButtonBase = IIntuition->OpenClass("gadgets/button.gadget", 53, &ButtonClass);
		if (ButtonBase == NULL)
		{
			break;
		}

		TermClass = init_term_gc();
		if (TermClass == NULL)
		{
			break;
		}

		/* Get icon for iconification */

		if (argc == 0)
		{
			struct WBStartup *wbsm = (struct WBStartup *)argv;
			BPTR cd;

			cd = IDOS->SetCurrentDir(wbsm->sm_ArgList[0].wa_Lock);
			AppIcon = IIcon->GetIconTags(wbsm->sm_ArgList[0].wa_Name,
				ICONGETA_FailIfUnavailable, TRUE,
				TAG_END);
			IDOS->SetCurrentDir(cd);
		}
		else
		{
			BPTR cd;

			cd = IDOS->SetCurrentDir(IDOS->GetProgramDir());
			AppIcon = IIcon->GetIconTags(IDOS->FilePart(argv[0]),
				ICONGETA_FailIfUnavailable, TRUE,
				TAG_END);
			IDOS->SetCurrentDir(cd);
		}

		if (AppIcon == NULL)
		{
			AppIcon = IIcon->GetIconTags(NULL,
				ICONGETA_GetDefaultName, "iconify",
				TAG_END);
			if (AppIcon == NULL)
			{
				AppIcon = IIcon->GetIconTags(NULL,
					ICONGETA_GetDefaultType, WBTOOL,
					TAG_END);
				if (AppIcon == NULL)
					break;
			}
		}

		/* Launch SSHTerm */

		rc = sshterm(argc, argv);
	}
	while (FALSE);

	/* Cleanup resources and exit */

	if (AppIcon != NULL)
	{
		IIcon->FreeDiskObject(AppIcon);
		AppIcon = NULL;
	}

	if (TermClass != NULL)
	{
		cleanup_term_gc(TermClass);
		TermClass = NULL;
	}

	if (ButtonBase != NULL)
	{
		IIntuition->CloseClass(ButtonBase);
		ButtonBase  = NULL;
		ButtonClass = NULL;
	}

	if (LayoutBase != NULL)
	{
		IIntuition->CloseClass(LayoutBase);
		LayoutBase  = NULL;
		LayoutClass = NULL;
	}

	if (LabelBase != NULL)
	{
		IIntuition->CloseClass(LabelBase);
		LabelBase  = NULL;
		LabelClass = NULL;
	}

	if (WindowBase != NULL)
	{
		IIntuition->CloseClass(WindowBase);
		WindowBase  = NULL;
		WindowClass = NULL;
	}

	if (IGadTools != NULL)
	{
		close_interface((struct Interface *)IGadTools);
		IGadTools = NULL;
	}

	if (IAmiSSL != NULL)
	{
		IAmiSSL->CleanupAmiSSL(TAG_END);
		IExec->DropInterface((struct Interface *)IAmiSSL);
		IAmiSSL = NULL;
	}

	libssh2_exit();

	if (AmiSSLBase != NULL)
	{
		IAmiSSLMaster->CloseAmiSSL();
		AmiSSLBase = NULL;
	}

	if (IAmiSSLMaster != NULL)
	{
		close_interface((struct Interface *)IAmiSSLMaster);
		IAmiSSLMaster = NULL;
	}

	if (ISocket != NULL)
	{
		close_interface((struct Interface *)ISocket);
		ISocket = NULL;
	}

	if (IZ != NULL)
	{
		close_interface((struct Interface *)IZ);
		IZ = NULL;
	}

	if (IIcon != NULL)
	{
		close_interface((struct Interface *)IIcon);
		IIcon = NULL;
	}

	if (ILayers != NULL)
	{
		close_interface((struct Interface *)ILayers);
		ILayers = NULL;
	}

	if (IKeymap != NULL)
	{
		close_interface((struct Interface *)IKeymap);
		IKeymap = NULL;
	}

	if (IAsl != NULL)
	{
		close_interface((struct Interface *)IAsl);
		IAsl = NULL;
	}

	if (IIntuition != NULL)
	{
		close_interface((struct Interface *)IIntuition);
		IIntuition = NULL;
	}

	if (IGraphics != NULL)
	{
		close_interface((struct Interface *)IGraphics);
		IGraphics = NULL;
	}

	if (IDiskfont != NULL)
	{
		close_interface((struct Interface *)IDiskfont);
		IDiskfont = NULL;
	}

	#ifdef __CLIB2__
	if (IUtility != NULL)
	{
		close_interface((struct Interface *)IUtility);
		IUtility = NULL;
	}
	#endif

	return rc;
}

struct Interface *open_interface(CONST_STRPTR name, int version)
{
	struct Library   *base;
	struct Interface *interface;

	base = IExec->OpenLibrary(name, version);
	if (base == NULL)
	{
		return NULL;
	}

	interface = IExec->GetInterface(base, "main", 1, NULL);
	if (interface == NULL)
	{
		IExec->CloseLibrary(base);
		return NULL;
	}

	return interface;
}

void close_interface(struct Interface *interface)
{
	if (interface != NULL)
	{
		struct Library *base = interface->Data.LibBase;

		IExec->DropInterface(interface);
		IExec->CloseLibrary(base);
	}
}

