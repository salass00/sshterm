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

#include <intuition/gadgetclass.h>
#include <intuition/icclass.h>
#include <diskfont/diskfonttag.h>
#include <datatypes/textclass.h>
#include <proto/iffparse.h>

#include <gadgets/scroller.h>

#include <libtsm.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdarg.h>

#define MIN_SIZE_X 80
#define MIN_SIZE_Y 24

#define G(x) ((struct Gadget *)(x))
#define EG(x) ((struct ExtGadget *)(x))

#define GREDRAW_SCROLL 84

#define ABS(x) ((x) >= 0 ? (x) : -(x))

#ifndef ID_CSET
#define ID_CSET MAKE_ID('C','S','E','T')
#endif
#ifndef ID_UTF8
#define ID_UTF8 MAKE_ID('U','T','F','8')
#endif

#define CHARSET_UTF8 106

struct TermData
{
	Object            *td_Object;

	struct tsm_screen *td_Con;
	struct tsm_vte    *td_VTE;

	UWORD              td_Columns;
	UWORD              td_Rows;

	struct TextFont   *td_Font;
	UWORD              td_CellW;
	UWORD              td_CellH;
	UWORD              td_Baseline;

	ULONG              td_CharSet;
	const ULONG       *td_MapTable;

	struct IBox        td_IBox;
	UWORD              td_Width;
	UWORD              td_Height;
	BOOL               td_Layouted:1;
	struct RastPort   *td_RPort;
	ULONG              td_Age;

	struct Hook       *td_UserHook;

	UWORD              td_MouseMode;
	UWORD              td_NumClicks;
	struct TimeVal     td_Time;
	WORD               td_MouseX;
	WORD               td_MouseY;
	WORD               td_PrevX;
	WORD               td_PrevY;

	ULONG              td_MaxSB;
	ULONG              td_SBTop;
	ULONG              td_SBVisible;
	ULONG              td_SBTotal;
	Object            *td_Scroller;
	LONG               td_ScrollDelta;
	UWORD              td_MinY;
	UWORD              td_MaxY;

	#ifdef OFFSCREEN_BUFFER
	struct Layer_Info *td_TmpLayerInfo;
	struct Layer      *td_TmpLayer;
	struct RastPort    td_TmpRP;
	struct Screen     *td_TmpScreen;
	ULONG              td_TmpSize;
	#endif

	#ifdef DEBUG
	ULONG              td_SkipCount;
	ULONG              td_UpdateCount;
	#endif

	struct Screen     *td_Screen;
};

enum {
	MM_DEFAULT,
	MM_MOVECURSOR,
	MM_MARKTEXT
};

static ULONG TERM_dispatch(Class *cl, Object *obj, Msg msg);

Class *init_term_gc(void)
{
	Class *cl;

	cl = IIntuition->MakeClass("termgclass", "gadgetclass", NULL, sizeof(struct TermData), 0);
	if (cl != NULL)
	{
		cl->cl_Dispatcher.h_Entry = TERM_dispatch;
	}

	return cl;
}

void cleanup_term_gc(Class *cl)
{
	IIntuition->FreeClass(cl);
}

static ULONG TERM_new(Class *cl, Object *obj, struct opSet *ops);
static ULONG TERM_dispose(Class *cl, Object *obj, Msg msg);
static ULONG TERM_get(Class *cl, Object *obj, struct opGet *opg);
static ULONG TERM_set(Class *cl, Object *obj, struct opSet *ops);
static ULONG TERM_domain(Class *cl, Object *obj, struct gpDomain *gpd);
static ULONG TERM_layout(Class *cl, Object *obj, struct gpLayout *gpl);
static ULONG TERM_render(Class *cl, Object *obj, struct gpRender *gpr);
static ULONG TERM_handlescroll(Class *cl, Object *obj, struct gpInput *gpi);
static ULONG TERM_input(Class *cl, Object *obj, struct tpInput *tpi);
static ULONG TERM_handlekeyboard(Class *cl, Object *obj, struct tpKeyboard *tpk);
static ULONG TERM_handlemouse(Class *cl, Object *obj, struct tpMouse *tpm);
static ULONG TERM_copy(Class *cl, Object *obj, struct tpGeneric *tpg);
static ULONG TERM_paste(Class *cl, Object *obj, struct tpGeneric *tpg);
static ULONG TERM_clearsb(Class *cl, Object *obj, struct tpGeneric *tpg);

static ULONG TERM_dispatch(Class *cl, Object *obj, Msg msg)
{
	ULONG result;

	switch (msg->MethodID)
	{
		case OM_NEW:
			result = TERM_new(cl, obj, (struct opSet *)msg);
			break;

		case OM_DISPOSE:
			result = TERM_dispose(cl, obj, msg);
			break;

		case OM_GET:
			result = TERM_get(cl, obj, (struct opGet *)msg);
			break;

		case OM_UPDATE:
		case OM_SET:
			result = TERM_set(cl, obj, (struct opSet *)msg);
			break;

		case GM_DOMAIN:
			result = TERM_domain(cl, obj, (struct gpDomain *)msg);
			break;

		case GM_LAYOUT:
			result = TERM_layout(cl, obj, (struct gpLayout *)msg);
			break;

		case GM_EXTENT:
			result = GMR_FULLBBOX;
			break;

		case GM_RENDER:
			result = TERM_render(cl, obj, (struct gpRender *)msg);
			break;

		case GM_HITTEST:
			result = 0;
			break;

		case GM_HANDLESCROLL:
			result = TERM_handlescroll(cl, obj, (struct gpInput *)msg);
			break;

		case TM_INPUT:
			result = TERM_input(cl, obj, (struct tpInput *)msg);
			break;

		case TM_HANDLEKEYBOARD:
			result = TERM_handlekeyboard(cl, obj, (struct tpKeyboard *)msg);
			break;

		case TM_HANDLEMOUSE:
			result = TERM_handlemouse(cl, obj, (struct tpMouse *)msg);
			break;

		case TM_COPY:
		case TM_COPYALL:
			result = TERM_copy(cl, obj, (struct tpGeneric *)msg);
			break;

		case TM_PASTE:
			result = TERM_paste(cl, obj, (struct tpGeneric *)msg);
			break;

		case TM_CLEARSB:
			result = TERM_clearsb(cl, obj, (struct tpGeneric *)msg);
			break;

		default:
			result = IIntuition->IDoSuperMethodA(cl, obj, msg);
			break;
	}

	return result;
}

static ULONG call_hook(struct TermData *td, struct Hook *hook, APTR msg)
{
	ULONG result;

	result = IUtility->CallHookPkt(hook, td->td_Object, msg);

	return result;
}

static void tsm_log_cb(void *data, const char *file, int line,
                       const char *func, const char *subs,
                       unsigned int sev, const char *format,
                       va_list args)
{
	UNUSED struct TermData *td = data;

	/* FIXME: Implement me */
}

static void tsm_write_cb(struct tsm_vte *vte, const char *u8,
                         size_t len, void *data)
{
	struct TermData *td = data;

	if (td->td_UserHook != NULL)
	{
		struct TermHookMsg thm;

		thm.MethodID    = THM_OUTPUT;
		thm.tohm_Data   = u8;
		thm.tohm_Length = len;

		call_hook(td, td->td_UserHook, &thm);
	}
}

static void tsm_bell_cb(struct tsm_vte *vte, void *data)
{
	struct TermData *td = data;

	if (td->td_UserHook != NULL)
	{
		struct TermHookMsg thm;

		thm.MethodID = THM_BELL;

		call_hook(td, td->td_UserHook, &thm);
	}
}

static ULONG TERM_new(Class *cl, Object *obj, struct opSet *ops)
{
	obj = (Object *)IIntuition->IDoSuperMethodA(cl, obj, (Msg)ops);

	if (obj != NULL)
	{
		struct TermData *td = INST_DATA(cl, obj);
		int rc;

		td->td_Object = obj;

		EG(obj)->MoreFlags |= GMORE_SCROLLRASTER;

		rc = tsm_screen_new(&td->td_Con, &tsm_log_cb, td);
		if (rc < 0)
		{
			IIntuition->ICoerceMethod(cl, obj, OM_DISPOSE);
			return (ULONG)NULL;
		}

		rc = tsm_vte_new(&td->td_VTE, td->td_Con, &tsm_write_cb, td,
		                 &tsm_log_cb, td);
		if (rc < 0)
		{
			IIntuition->ICoerceMethod(cl, obj, OM_DISPOSE);
			return (ULONG)NULL;
		}

		tsm_vte_set_bell_cb(td->td_VTE, tsm_bell_cb, td);

		td->td_Columns = tsm_screen_get_width(td->td_Con);
		td->td_Rows    = tsm_screen_get_height(td->td_Con);

		td->td_Font = ((struct GfxBase *)IGraphics->Data.LibBase)->DefaultFont;

		td->td_CellW    = td->td_Font->tf_XSize + td->td_Font->tf_BoldSmear;
		td->td_CellH    = td->td_Font->tf_YSize;
		td->td_Baseline = td->td_Font->tf_Baseline;

		td->td_CharSet = IDiskfont->GetDiskFontCtrl(DFCTRL_CHARSET);

		td->td_MapTable = (const ULONG *)IDiskfont->ObtainCharsetInfo(DFCS_NUMBER, td->td_CharSet, DFCS_MAPTABLE);
		if (td->td_MapTable == NULL)
		{
			IIntuition->ICoerceMethod(cl, obj, OM_DISPOSE);
			return (ULONG)NULL;
		}

		td->td_MaxSB     = 0;
		td->td_SBTop     = 0;
		td->td_SBVisible = td->td_Rows;
		td->td_SBTotal   = td->td_Rows;

		TERM_set(cl, obj, ops);
	}

	return (ULONG)obj;
}

static ULONG TERM_dispose(Class *cl, Object *obj, Msg msg)
{
	struct TermData *td = INST_DATA(cl, obj);
	ULONG result;

	#ifdef OFFSCREEN_BUFFER
	if (td->td_TmpRP.BitMap != NULL)
	{
		ILayers->DeleteLayer(0, td->td_TmpLayer);
		ILayers->DisposeLayerInfo(td->td_TmpLayerInfo);
		IGraphics->FreeBitMap(td->td_TmpRP.BitMap);

		td->td_TmpLayer = NULL;
		td->td_TmpLayerInfo = NULL;
		td->td_TmpRP.BitMap = NULL;
	}
	#endif

	if (td->td_VTE != NULL)
	{
		tsm_vte_unref(td->td_VTE);
		td->td_VTE = NULL;
	}

	if (td->td_Con != NULL)
	{
		tsm_screen_unref(td->td_Con);
		td->td_Con = NULL;
	}

	result = IIntuition->IDoSuperMethodA(cl, obj, msg);

	return result;
}

static ULONG TERM_get(Class *cl, Object *obj, struct opGet *opg)
{
	struct TermData *td = INST_DATA(cl, obj);
	ULONG result = TRUE;

	switch (opg->opg_AttrID)
	{
		case TERM_UserHook:
			*opg->opg_Storage = (ULONG)td->td_UserHook;
			break;

		case TERM_Columns:
			*opg->opg_Storage = td->td_Columns;
			break;

		case TERM_Rows:
			*opg->opg_Storage = td->td_Rows;
			break;

		case TERM_Font:
			*opg->opg_Storage = (ULONG)td->td_Font;
			break;

		case TERM_MaxScrollback:
			*opg->opg_Storage = (ULONG)td->td_MaxSB;
			break;

		case TERM_Scroller:
			*opg->opg_Storage = (ULONG)td->td_Scroller;
			break;

		case TERM_SBTop:
			*opg->opg_Storage = td->td_SBTop;
			break;

		case TERM_SBVisible:
			*opg->opg_Storage = td->td_SBVisible;
			break;

		case TERM_SBTotal:
			*opg->opg_Storage = td->td_SBTotal;
			break;

		default:
			result = IIntuition->IDoSuperMethodA(cl, obj, (Msg)opg);
			break;
	}

	return result;
}

static ULONG VARARGS68K SetAttrsGI(Object *obj, struct GadgetInfo *ginfo, ...) {
	va_list tags;
	struct opSet ops;
	ULONG result;

	va_startlinear(tags, ginfo);

	ops.MethodID     = OM_SET;
	ops.ops_AttrList = va_getlinearva(tags, struct TagItem *);
	ops.ops_GInfo    = ginfo;

	result = IIntuition->IDoMethodA(obj, (Msg)&ops);

	va_end(tags);

	return result;
}

static ULONG TERM_set(Class *cl, Object *obj, struct opSet *ops)
{
	struct TermData *td = INST_DATA(cl, obj);
	struct TagItem *tstate = ops->ops_AttrList;
	struct TagItem *tag;
	BOOL initial = (ops->MethodID == OM_NEW) ? TRUE : FALSE;
	BOOL refresh = FALSE;
	BOOL update_scroller = FALSE;
	ULONG top, max;
	int scroll = 0;

	if (!initial && IIntuition->IDoSuperMethodA(cl, obj, (Msg)ops))
	{
		refresh = TRUE;
	}

	while ((tag = IUtility->NextTagItem(&tstate)) != NULL)
	{
		switch (tag->ti_Tag)
		{
			case TERM_UserHook:
				td->td_UserHook = (struct Hook *)tag->ti_Data;
				break;

			case TERM_Font:
				/* Only allow fixed-width fonts */
				if ((((struct TextFont *)tag->ti_Data)->tf_Flags & FPF_PROPORTIONAL) == 0)
				{
					td->td_Font = (struct TextFont *)tag->ti_Data;

					td->td_CellW    = td->td_Font->tf_XSize + td->td_Font->tf_BoldSmear;
					td->td_CellH    = td->td_Font->tf_YSize;
					td->td_Baseline = td->td_Font->tf_Baseline;
				}
				break;

			case TERM_MaxScrollback:
				if (tag->ti_Data == td->td_MaxSB)
					break;

				td->td_MaxSB = tag->ti_Data;
				tsm_screen_set_max_sb(td->td_Con, td->td_MaxSB);

				refresh = TRUE;
				update_scroller = TRUE;
				break;

			case TERM_Scroller:
				td->td_Scroller = (Object *)tag->ti_Data;
				if (td->td_Scroller != NULL)
				{
					static const struct TagItem scroller_map[] =
					{
						{ SCROLLER_Top, TERM_SBTop },
						{ TAG_END,      0          }
					};

					IIntuition->SetAttrs(td->td_Scroller,
						ICA_TARGET, obj,
						ICA_MAP,    scroller_map,
						TAG_END);

					update_scroller = TRUE;
				}
				break;

			case TERM_SBTop:
				top = tag->ti_Data;

				max = td->td_SBTotal - td->td_SBVisible;
				if ((LONG)top < 0)
					top = 0;
				else if (top > max)
					top = max;

				if (top == td->td_SBTop)
					break;

				if (top < td->td_SBTop)
					tsm_screen_sb_up(td->td_Con, td->td_SBTop - top);
				else
					tsm_screen_sb_down(td->td_Con, top - td->td_SBTop);

				scroll = top - td->td_SBTop;

				if (ABS(scroll) >= td->td_Rows)
					refresh = TRUE;

				td->td_SBTop = top;

				if (ops->MethodID != OM_UPDATE)
				{
					update_scroller = TRUE;
				}
				break;

			case TERM_BuiltInPalette:
				if (tsm_vte_set_palette(td->td_VTE, (const char *)tag->ti_Data) == 0)
				{
					refresh = TRUE;
				}
				break;
		}
	}

	if (refresh && ops->ops_GInfo != NULL)
	{
		IIntuition->DoRender(obj, ops->ops_GInfo, GREDRAW_UPDATE);
		refresh = FALSE;
		scroll = 0;
	}
	else if (scroll != 0 && ops->ops_GInfo != NULL)
	{
		td->td_ScrollDelta = scroll;
		IIntuition->DoRender(obj, ops->ops_GInfo, GREDRAW_SCROLL);
		scroll = 0;
	}

	if (update_scroller)
	{
		td->td_SBTop     = tsm_screen_get_sb_top(td->td_Con);
		td->td_SBVisible = tsm_screen_get_sb_visible(td->td_Con);
		td->td_SBTotal   = tsm_screen_get_sb_total(td->td_Con);

		if (td->td_Scroller != NULL)
		{
			SetAttrsGI(td->td_Scroller, ops->ops_GInfo,
				SCROLLER_Total,   td->td_SBTotal,
				SCROLLER_Visible, td->td_SBVisible,
				SCROLLER_Top,     td->td_SBTop,
				TAG_END);
		}
	}

	return (refresh || scroll);
}

static ULONG TERM_domain(Class *cl, Object *obj, struct gpDomain *gpd)
{
	struct TermData *td = INST_DATA(cl, obj);

	if (gpd->gpd_Which == GDOMAIN_MAXIMUM)
	{
		gpd->gpd_Domain.Width  = 8192;
		gpd->gpd_Domain.Height = 8192;
	}
	else
	{
		gpd->gpd_Domain.Width  = MIN_SIZE_X * td->td_CellW;
		gpd->gpd_Domain.Height = MIN_SIZE_Y * td->td_CellH;
	}

	return 1;
}

static ULONG TERM_layout(Class *cl, Object *obj, struct gpLayout *gpl)
{
	struct TermData *td = INST_DATA(cl, obj);
	UWORD rows, columns;

	IIntuition->GadgetBox(G(obj), gpl->gpl_GInfo, GBD_GINFO, 0, &td->td_IBox);

	#ifdef DEBUG
	IExec->DebugPrintF("GM_LAYOUT width: %u height: %u\n", td->td_IBox.Width, td->td_IBox.Height);
	#endif

	columns = td->td_IBox.Width / td->td_CellW;
	rows = td->td_IBox.Height / td->td_CellH;

	if (columns < MIN_SIZE_X)
		columns = MIN_SIZE_X;
	if (rows < MIN_SIZE_Y)
		rows = MIN_SIZE_Y;

	if (columns != td->td_Columns || rows != td->td_Rows)
	{
		UNUSED int r;
		ULONG top, visible, total;

		r = tsm_screen_resize(td->td_Con, columns, rows);
		#ifdef DEBUG
		if (r < 0)
			IExec->DebugPrintF("tsm_screen_resize: %d\n", r);
		}
		#endif

		td->td_Columns = tsm_screen_get_width(td->td_Con);
		td->td_Rows = tsm_screen_get_height(td->td_Con);

		if (td->td_UserHook != NULL)
		{
			struct TermHookMsg thm;

			thm.MethodID     = THM_RESIZE;
			thm.trhm_Columns = td->td_Columns;
			thm.trhm_Rows    = td->td_Rows;

			call_hook(td, td->td_UserHook, &thm);
		}

		top     = tsm_screen_get_sb_top(td->td_Con);
		visible = tsm_screen_get_sb_visible(td->td_Con);
		total   = tsm_screen_get_sb_total(td->td_Con);

		if ((top + visible) > total)
		{
			ULONG newtop = total - visible;
			tsm_screen_sb_up(td->td_Con, top - newtop);
			top = newtop;
		}

		if (top     != td->td_SBTop ||
			visible != td->td_SBVisible ||
			total   != td->td_SBTotal)
		{
			td->td_SBTop     = top;
			td->td_SBVisible = visible;
			td->td_SBTotal   = total;

			if (td->td_Scroller != NULL)
			{
				SetAttrsGI(td->td_Scroller, gpl->gpl_GInfo,
					SCROLLER_Total,   td->td_SBTotal,
					SCROLLER_Visible, td->td_SBVisible,
					SCROLLER_Top,     td->td_SBTop,
					TAG_END);
			}
		}
	}

	td->td_Width = td->td_Columns * td->td_CellW;
	td->td_Height = td->td_Rows * td->td_CellH;

	if (td->td_Width > td->td_IBox.Width)
		td->td_Width = td->td_IBox.Width;
	if (td->td_Height > td->td_IBox.Height)
		td->td_Height = td->td_IBox.Height;

	td->td_Layouted = TRUE;

	return 1;
}

static TEXT map_unicode(const ULONG *maptable, ULONG unicode)
{
	TEXT ch = '?';

	if (unicode >= 128)
	{
		ULONG i;

		for (i = 128; i < 256; i++)
		{
			if (maptable[i] == unicode)
			{
				ch = (TEXT)i;
				break;
			}
		}
	}
	else
		ch = (TEXT)unicode;

	return ch;
}

static int tsm_draw_cb(struct tsm_screen *con, uint64_t id,
                       const uint32_t *ch, size_t len, unsigned int width,
                       unsigned int posx, unsigned int posy,
                       const struct tsm_screen_attr *attr, tsm_age_t age,
                       void *data)
{
	struct TermData *td = data;
	struct RastPort *rp;
	ULONG f_argb, b_argb;
	UWORD cellw, cellh;
	UWORD x, y;
	ULONG style;
	TEXT tmp[1];

	if (age != 0 && age <= td->td_Age)
	{
		#ifdef DEBUG
		td->td_SkipCount++;
		#endif
		return 0;
	}

	if (posy < td->td_MinY || posy > td->td_MaxY)
	{
		#ifdef DEBUG
		td->td_SkipCount++;
		#endif
		return 0;
	}

	#ifdef DEBUG
	td->td_UpdateCount++;
	#endif

	cellw = td->td_CellW;
	cellh = td->td_CellH;

	#ifdef OFFSCREEN_BUFFER
	if (len && td->td_TmpRP.BitMap != NULL)
	{
		rp = &td->td_TmpRP;
		x = y = 0;
	}
	else
	#endif
	{
		rp = td->td_RPort;
		x = td->td_IBox.Left + posx * cellw;
		y = td->td_IBox.Top + posy * cellh;
	}

	if (attr->inverse)
	{
		f_argb = *(ULONG *)(&attr->br - 1) | 0xff000000;
		b_argb = *(ULONG *)(&attr->fr - 1) | 0xff000000;
	}
	else
	{
		f_argb = *(ULONG *)(&attr->fr - 1) | 0xff000000;
		b_argb = *(ULONG *)(&attr->br - 1) | 0xff000000;
	}

	IGraphics->SetRPAttrs(rp,
		RPTAG_APenColor, b_argb,
		TAG_END);

	IGraphics->RectFill(rp, x, y, x + cellw - 1, y + cellh - 1);

	if (len)
	{
		style = 0;

		if (attr->bold)
		{
			style |= FSF_BOLD;
		}

		if (attr->underline)
		{
			style |= FSF_UNDERLINED;
		}

		IGraphics->SetSoftStyle(rp, style, FSF_BOLD | FSF_UNDERLINED);

		IGraphics->SetRPAttrs(rp,
			RPTAG_APenColor, f_argb,
			TAG_END);

		IGraphics->Move(rp, x, y + td->td_Baseline);

		tmp[0] = map_unicode(td->td_MapTable, ch[0]);

		#ifdef OFFSCREEN_BUFFER
		if (rp == &td->td_TmpRP)
		{
			/* Enable clipping */
			rp->Layer = td->td_TmpLayer;
		}
		#endif

		IGraphics->Text(rp, tmp, 1);

		#ifdef OFFSCREEN_BUFFER
		if (rp == &td->td_TmpRP)
		{
			/* Disable clipping */
			rp->Layer = NULL;
		}
		#endif
	}

	#ifdef OFFSCREEN_BUFFER
	if (rp == &td->td_TmpRP)
	{
		IGraphics->BltBitMapTags(
			BLITA_SrcType,  BLITT_BITMAP,
			BLITA_Source,   td->td_TmpRP.BitMap,
			BLITA_DestType, BLITT_RASTPORT,
			BLITA_Dest,     td->td_RPort,
			BLITA_DestX,    td->td_IBox.Left + posx * cellw,
			BLITA_DestY,    td->td_IBox.Top + posy * cellh,
			BLITA_Width,    cellw,
			BLITA_Height,   cellh,
			TAG_END);
	}
	#endif

	return 0;
}

static void render_cells(struct TermData *td, struct RastPort *rp, UWORD miny, UWORD maxy, BOOL domargins)
{
	td->td_RPort = rp;

	#ifdef OFFSCREEN_BUFFER
	if (td->td_TmpRP.BitMap != NULL)
	{
		IGraphics->SetDrMd(&td->td_TmpRP, JAM1);
		IGraphics->SetFont(&td->td_TmpRP, td->td_Font);
	}
	else
	#endif
	{
		IGraphics->SetDrMd(rp, JAM1);
		IGraphics->SetFont(rp, td->td_Font);
	}

	#ifdef DEBUG
	td->td_SkipCount = 0;
	td->td_UpdateCount = 0;
	#endif

	td->td_MinY = miny;
	td->td_MaxY = maxy;

	td->td_Age = tsm_screen_draw(td->td_Con, &tsm_draw_cb, td);

	#ifdef DEBUG
	IExec->DebugPrintF("GM_RENDER cells rendered: %lu skipped: %lu\n",
	                   td->td_UpdateCount, td->td_SkipCount);
	#endif

	td->td_RPort = NULL;

	if (domargins)
	{
		/* Fill margins with the VTE default background color */
		struct tsm_screen_attr attr;

		if (td->td_IBox.Width > td->td_Width || td->td_IBox.Height > td->td_Height)
		{
			tsm_vte_get_def_attr(td->td_VTE, &attr);

			IGraphics->SetRPAttrs(rp,
				RPTAG_APenColor, *(ULONG *)(&attr.br - 1) | 0xff000000,
				TAG_END);

			if (td->td_IBox.Width > td->td_Width)
			{
				IGraphics->RectFill(rp,
					                td->td_IBox.Left + td->td_Width,
					                td->td_IBox.Top,
					                td->td_IBox.Left + td->td_IBox.Width - 1,
					                td->td_IBox.Top + td->td_Height - 1);
			}

			if (td->td_IBox.Height > td->td_Height)
			{
				IGraphics->RectFill(rp,
					                td->td_IBox.Left,
					                td->td_IBox.Top + td->td_Height,
					                td->td_IBox.Left + td->td_IBox.Width - 1,
					                td->td_IBox.Top + td->td_IBox.Height - 1);
			}
		}
	}
}

static ULONG TERM_render(Class *cl, Object *obj, struct gpRender *gpr)
{
	struct TermData *td = INST_DATA(cl, obj);
	struct RastPort *rp = gpr->gpr_RPort;

	#ifdef OFFSCREEN_BUFFER
	struct Screen *screen = gpr->gpr_GInfo->gi_Screen;
	ULONG cellw = td->td_CellW;
	ULONG cellh = td->td_CellH;
	if (td->td_TmpRP.BitMap == NULL ||
		td->td_TmpScreen != screen ||
		td->td_TmpSize != ((cellw << 16) | cellh))
	{
		struct BitMap *bitmap;
		struct Layer_Info *li;
		struct Layer *layer;

		if (td->td_TmpRP.BitMap != NULL)
		{
			ILayers->DeleteLayer(0, td->td_TmpLayer);
			ILayers->DisposeLayerInfo(td->td_TmpLayerInfo);
			IGraphics->FreeBitMap(td->td_TmpRP.BitMap);

			td->td_TmpLayer = NULL;
			td->td_TmpLayerInfo = NULL;
			td->td_TmpRP.BitMap = NULL;
		}

		const struct TagItem bmtags[] =
		{
			{ BMATags_Friend,      (ULONG)screen->RastPort.BitMap },
			{ BMATags_UserPrivate, TRUE                           },
			{ TAG_END,             0                              }
		};

		if (LIB_IS_AT_LEAST(IGraphics->Data.LibBase, 53, 7))
			bitmap = IGraphics->AllocBitMapTagList(cellw, cellh, 24, bmtags);
		else
			bitmap = IGraphics->AllocBitMap(cellw, cellh, 24, BMF_CHECKVALUE, (struct BitMap *)bmtags);

		if (bitmap != NULL)
		{
			li = ILayers->NewLayerInfo();
			if (li != NULL)
			{
				layer = ILayers->CreateUpfrontHookLayer(li, bitmap, 0, 0, cellw - 1, cellh - 1,
				                                        0, LAYERS_NOBACKFILL, NULL);
				if (layer != NULL)
				{
					td->td_TmpLayerInfo = li;
					td->td_TmpLayer = layer;

					IGraphics->InitRastPort(&td->td_TmpRP);
					td->td_TmpRP.BitMap = bitmap;

					td->td_TmpScreen = screen;
					td->td_TmpSize = (cellw << 16) | cellh;
				}
				else
				{
					ILayers->DisposeLayerInfo(li);
					IGraphics->FreeBitMap(bitmap);
				}
			}
			else
			{
				IGraphics->FreeBitMap(bitmap);
			}
		}
	}
	#endif

	if (gpr->gpr_Redraw == GREDRAW_SCROLL)
	{
		struct Layer *layer = gpr->gpr_GInfo->gi_Layer;
		struct Hook *bfh;
		LONG scroll = td->td_ScrollDelta;
		BOOL scroll_damage = FALSE;

		td->td_ScrollDelta = 0;

		bfh = ILayers->InstallLayerHook(layer, LAYERS_NOBACKFILL);

		IGraphics->ScrollRasterBF(rp, 0, scroll * td->td_CellH,
		                          td->td_IBox.Left,
		                          td->td_IBox.Top,
		                          td->td_IBox.Left + td->td_Width - 1,
		                          td->td_IBox.Top + td->td_Height - 1);

		if (layer->Flags & LAYERREFRESH)
			scroll_damage = TRUE;

		ILayers->InstallLayerHook(layer, bfh);

		if (scroll < 0)
			render_cells(td, rp, 0, 1 - scroll, FALSE);
		else
			render_cells(td, rp, td->td_Rows - scroll, td->td_Rows - 1, FALSE);

		if (scroll_damage)
		{
			td->td_Age = 0;

			ILayers->BeginUpdate(layer);

			if (scroll < 0)
				render_cells(td, rp, -scroll, td->td_Rows - 1, TRUE);
			else
				render_cells(td, rp, 0, td->td_Rows - scroll - 1, TRUE);

			ILayers->EndUpdate(layer, FALSE);
		}
	}
	else
	{
		if (gpr->gpr_Redraw == GREDRAW_REDRAW)
		{
			if (!td->td_Layouted)
			{
				IIntuition->ICoerceMethod(cl, obj, GM_LAYOUT, gpr->gpr_GInfo, 1);
				/* td->td_Layouted = FALSE; */
			}

			td->td_Age = 0;
		}

		render_cells(td, rp, 0, td->td_Rows - 1, TRUE);
	}

	return 1;
}

static ULONG TERM_handlescroll(Class *cl, Object *obj, struct gpInput *gpi)
{
	struct TermData *td = INST_DATA(cl, obj);
	struct InputEvent *ie = gpi->gpi_IEvent;
	ULONG result = GMR_NOREUSE;

	if (ie != NULL && ie->ie_Class == IECLASS_MOUSEWHEEL && ie->ie_Code == IECODE_NOBUTTON)
	{
		ULONG max = td->td_SBTotal - td->td_SBVisible;
		ULONG top = td->td_SBTop;

		if (ie->ie_Y > 0)
		{
			top += ie->ie_Y;
			if (top > max)
				top = max;
		}
		else if (ie->ie_Y < 0)
		{
			top += ie->ie_Y;
			if ((LONG)top < 0)
				top = 0;
		}

		if (top != td->td_SBTop)
		{
			SetAttrsGI(obj, gpi->gpi_GInfo,
				TERM_SBTop, top,
				TAG_END);

			*gpi->gpi_Termination = td->td_SBTop;
			gpi->gpi_TabletData = (APTR)obj;

			result = GMR_NOREUSE|GMR_VERIFY;
		}
	}

	return result;
}

static ULONG TERM_input(Class *cl, Object *obj, struct tpInput *tpi)
{
	struct TermData *td = INST_DATA(cl, obj);
	ULONG top, visible, total;

	tsm_vte_input(td->td_VTE, tpi->tpi_Data, tpi->tpi_Length);

	if (tpi->tpi_GInfo != NULL)
	{
		top     = tsm_screen_get_sb_top(td->td_Con);
		visible = tsm_screen_get_sb_visible(td->td_Con);
		total   = tsm_screen_get_sb_total(td->td_Con);

		if (top     != td->td_SBTop ||
			visible != td->td_SBVisible ||
			total   != td->td_SBTotal)
		{
			td->td_SBTop     = top;
			td->td_SBVisible = visible;
			td->td_SBTotal   = total;

			if (td->td_Scroller != NULL)
			{
				SetAttrsGI(td->td_Scroller, tpi->tpi_GInfo,
					SCROLLER_Total,   td->td_SBTotal,
					SCROLLER_Visible, td->td_SBVisible,
					SCROLLER_Top,     td->td_SBTop,
					TAG_END);
			}
		}

		IIntuition->DoRender(obj, tpi->tpi_GInfo, GREDRAW_UPDATE);
	}

	return 1;
}

static ULONG TERM_handlekeyboard(Class *cl, Object *obj, struct tpKeyboard *tpk)
{
	struct TermData *td = INST_DATA(cl, obj);
	struct InputEvent *ie = &tpk->tpk_IEvent;
	TEXT vanilla;
	ULONG ucs4 = TSM_VTE_INVALID;
	int n;

	/* Ignore non-rawkey events */
	if ((ie->ie_Class != IECLASS_RAWKEY) && (ie->ie_Class != IECLASS_EXTENDEDRAWKEY))
		return 0;

	/* Ignore key up events */
	if (ie->ie_Code & IECODE_UP_PREFIX)
		return 0;

	n = IKeymap->MapRawKey(ie, &vanilla, 1, NULL);
	if (n == 1)
	{
		ucs4 = td->td_MapTable[(UBYTE)vanilla];
	}

	if (tsm_vte_handle_keyboard_amiga(td->td_VTE, ie->ie_Code, ie->ie_Qualifier, ucs4))
	{
		tsm_screen_sb_reset(td->td_Con);
		return 1;
	}

	return 0;
}

static int doubleclick(const struct TimeVal *start, const struct TimeVal *current)
{
	return IIntuition->DoubleClick(start->Seconds, start->Microseconds,
	                               current->Seconds, current->Microseconds);
}

static ULONG TERM_handlemouse(Class *cl, Object *obj, struct tpMouse *tpm)
{
	struct TermData *td = INST_DATA(cl, obj);
	WORD x, y;
	BOOL refresh = FALSE;

	if (!td->td_Layouted)
	{
		IIntuition->ICoerceMethod(cl, obj, GM_LAYOUT, tpm->tpm_GInfo, 1);
	}

	/* Convert to gadget relative coords */
	x = tpm->tpm_MouseX - td->td_IBox.Left;
	y = tpm->tpm_MouseY - td->td_IBox.Top;

	/* Check if we are in the gadget */
	if ((x >= 0 && x < td->td_Width) && (y >= 0 && y < td->td_Height))
	{
		if (tpm->tpm_Button == SELECTDOWN)
		{
			if (td->td_NumClicks != 0 &&
				(x / td->td_CellW) == (td->td_MouseX / td->td_CellW) &&
				(y / td->td_CellH) == (td->td_MouseY / td->td_CellH) &&
				doubleclick(&td->td_Time, &tpm->tpm_Time))
			{
				if (td->td_NumClicks == 1)
				{
					td->td_NumClicks = 2;

					tsm_screen_selection_word(td->td_Con,
					                          td->td_MouseX / td->td_CellW,
					                          td->td_MouseY / td->td_CellH);

					refresh = TRUE;
				}
				else
				{
					td->td_NumClicks = 0;

					tsm_screen_selection_line(td->td_Con,
					                          td->td_MouseY / td->td_CellH);

					refresh = TRUE;
				}

				td->td_Time = tpm->tpm_Time;
			}
			else
			{
				td->td_MouseMode = MM_MOVECURSOR;
				td->td_NumClicks = 1;
				td->td_Time      = tpm->tpm_Time;
				td->td_MouseX    = x;
				td->td_MouseY    = y;

				tsm_screen_selection_reset(td->td_Con);

				/* FIXME: Move text cursor if applicable */

				refresh = TRUE;
			}
		}
		else if (tpm->tpm_Button == SELECTUP)
		{
			td->td_MouseMode = MM_DEFAULT;
		}
		else if (tpm->tpm_Button == 0)
		{
			if (td->td_MouseMode == MM_MOVECURSOR)
			{
				if (ABS(td->td_MouseX - x) > 3 || ABS(td->td_MouseY - y) > 3)
				{
					td->td_MouseMode = MM_MARKTEXT;

					tsm_screen_selection_start(td->td_Con,
					                           td->td_MouseX / td->td_CellW,
					                           td->td_MouseY / td->td_CellH);

					td->td_PrevX = td->td_MouseX;
					td->td_PrevY = td->td_MouseY;

					refresh = TRUE;
				}
			}
			else if (td->td_MouseMode == MM_MARKTEXT)
			{
				if ((x / td->td_CellW) != (td->td_PrevX / td->td_CellW) ||
					(y / td->td_CellH) != (td->td_PrevY / td->td_CellH))
				{
					tsm_screen_selection_target(td->td_Con,
						                        x / td->td_CellW,
						                        y / td->td_CellH);

					td->td_PrevX = x;
					td->td_PrevY = y;

					refresh = TRUE;
				}
			}
		}
	}

	if (refresh && tpm->tpm_GInfo != NULL)
	{
		IIntuition->DoRender(obj, tpm->tpm_GInfo, GREDRAW_UPDATE);
	}

	return 0;
}

static ULONG convert_from_utf8(STRPTR dst, CONST_STRPTR src, ULONG srclen, const ULONG *maptable)
{
	STRPTR d = dst;
	CONST_STRPTR s = src;
	CONST_STRPTR srcend = src + srclen;
	mbstate_t ps;
	LONG mblen;
	wchar_t ucs4;

	memset(&ps, 0, sizeof(ps));

	while ((mblen = mbrtowc(&ucs4, s, srcend - s, &ps)) > 0)
	{
		*d++ = map_unicode(maptable, ucs4);
		s += mblen;
	}

	return d - dst;
}

static BOOL write_clip(ULONG unit, CONST_STRPTR utf8, ULONG utf8_len)
{
	struct IFFParseIFace *IIFFParse;
	struct IFFHandle *iff = NULL;
	LONG error;
	ULONG charset;
	STRPTR text = NULL;
	ULONG len = 0;
	BOOL result = FALSE;
	APTR window;

	/* Disable DOS requesters as they might cause a deadlock if we are called via
	 * DoGadgetMethod(). */
	window = IDOS->SetProcWindow((APTR)-1);

	IIFFParse = (struct IFFParseIFace *)open_interface("iffparse.library", 53);
	if (IIFFParse == NULL)
		goto cleanup;

	charset = IDiskfont->GetDiskFontCtrl(DFCTRL_CHARSET);

	if (charset != CHARSET_UTF8)
	{
		const ULONG *maptable;

		maptable = (const ULONG *)IDiskfont->ObtainCharsetInfo(DFCS_NUMBER, charset, DFCS_MAPTABLE);
		if (maptable == NULL)
			goto cleanup;

		text = malloc(utf8_len + 1);
		if (text == NULL)
			goto cleanup;

		len = convert_from_utf8(text, utf8, utf8_len, maptable);
	}

	iff = IIFFParse->AllocIFF();
	if (iff == NULL)
		goto cleanup;

	iff->iff_Stream = (ULONG)IIFFParse->OpenClipboard(unit);
	if (iff->iff_Stream == (ULONG)NULL)
		goto cleanup;

	IIFFParse->InitIFFasClip(iff);

	error = IIFFParse->OpenIFF(iff, IFFF_WRITE);
	if (error)
		goto cleanup;

	error = IIFFParse->PushChunk(iff, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN);
	if (error)
		goto cleanup;

	error = IIFFParse->PushChunk(iff, 0, ID_CSET, IFFSIZE_UNKNOWN);
	if (error)
		goto cleanup;
	if (IIFFParse->WriteChunkBytes(iff, &charset, sizeof(charset)) != sizeof(charset))
		goto cleanup;
	error = IIFFParse->PopChunk(iff);
	if (error)
		goto cleanup;

	if (charset == CHARSET_UTF8)
	{
		error = IIFFParse->PushChunk(iff, 0, ID_CHRS, IFFSIZE_UNKNOWN);
		if (error)
			goto cleanup;
		if (IIFFParse->WriteChunkBytes(iff, utf8, utf8_len) != utf8_len)
			goto cleanup;
		error = IIFFParse->PopChunk(iff);
		if (error)
			goto cleanup;
	}
	else
	{
		error = IIFFParse->PushChunk(iff, 0, ID_CHRS, IFFSIZE_UNKNOWN);
		if (error)
			goto cleanup;
		if (IIFFParse->WriteChunkBytes(iff, text, len) != len)
			goto cleanup;
		error = IIFFParse->PopChunk(iff);
		if (error)
			goto cleanup;

		error = IIFFParse->PushChunk(iff, 0, ID_UTF8, IFFSIZE_UNKNOWN);
		if (error)
			goto cleanup;
		if (IIFFParse->WriteChunkBytes(iff, utf8, utf8_len) != utf8_len)
			goto cleanup;
		error = IIFFParse->PopChunk(iff);
		if (error)
			goto cleanup;
	}

	error = IIFFParse->PopChunk(iff);
	if (error)
		goto cleanup;

cleanup:
	if (iff != NULL)
	{
		IIFFParse->CloseIFF(iff);

		if (iff->iff_Stream != (ULONG)NULL)
			IIFFParse->CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);

		IIFFParse->FreeIFF(iff);
	}

	if (text != NULL)
		free(text);

	if (IIFFParse != NULL)
		close_interface((struct Interface *)IIFFParse);

	/* Re-enable DOS requesters. */
	IDOS->SetProcWindow(window);

	return result;
}

static ULONG TERM_copy(Class *cl, Object *obj, struct tpGeneric *tpg)
{
	struct TermData *td = INST_DATA(cl, obj);
	STRPTR utf8;
	ULONG utf8_len;
	ULONG result = 0;

	if (tpg->MethodID == TM_COPY)
		utf8_len = tsm_screen_selection_copy(td->td_Con, &utf8);
	else
		utf8_len = tsm_screen_copy_all(td->td_Con, &utf8);

	if (utf8_len > 0)
	{
		result = write_clip(PRIMARY_CLIP, utf8, utf8_len);

		free(utf8);
	}

	return result;
}

static BOOL read_clip(ULONG unit, STRPTR *utf8, ULONG *utf8_len)
{
	struct IFFParseIFace *IIFFParse;
	struct IFFHandle *iff = NULL;
	struct ContextNode *cn;
	LONG error;
	ULONG charset;
	STRPTR text = NULL;
	ULONG len = 0;
	BOOL ignore_chrs = FALSE;
	BOOL result = FALSE;
	APTR window;

	/* Disable DOS requesters as they might cause a deadlock if we are called via
	 * DoGadgetMethod(). */
	window = IDOS->SetProcWindow((APTR)-1);

	IIFFParse = (struct IFFParseIFace *)open_interface("iffparse.library", 53);
	if (IIFFParse == NULL)
		goto cleanup;

	charset = IDiskfont->GetDiskFontCtrl(DFCTRL_CHARSET);

	iff = IIFFParse->AllocIFF();
	if (iff == NULL)
		goto cleanup;

	iff->iff_Stream = (ULONG)IIFFParse->OpenClipboard(unit);
	if (iff->iff_Stream == (ULONG)NULL)
		goto cleanup;

	IIFFParse->InitIFFasClip(iff);

	error = IIFFParse->OpenIFF(iff, IFFF_READ);
	if (error)
		goto cleanup;

	error = IIFFParse->StopChunk(iff, ID_FTXT, ID_CSET);
	if (error)
		goto cleanup;

	error = IIFFParse->StopChunk(iff, ID_FTXT, ID_CHRS);
	if (error)
		goto cleanup;

	error = IIFFParse->StopChunk(iff, ID_FTXT, ID_UTF8);
	if (error)
		goto cleanup;

	while (TRUE)
	{
		error = IIFFParse->ParseIFF(iff, IFFPARSE_SCAN);
		if (error)
		{
			if (error == IFFERR_EOC)
				continue;

			break;
		}

		cn = IIFFParse->CurrentChunk(iff);
		if (cn != NULL && cn->cn_Type == ID_FTXT)
		{
			if (!ignore_chrs)
			{
				if (cn->cn_ID == ID_CSET && cn->cn_Size >= 4)
				{
					if (IIFFParse->ReadChunkBytes(iff, &charset, sizeof(charset)) != sizeof(charset))
						goto cleanup;
				}
				else if (cn->cn_ID == ID_CHRS && cn->cn_Size > 0)
				{
					text = realloc(text, len + cn->cn_Size);
					if (text == NULL)
						goto cleanup;

					if (IIFFParse->ReadChunkBytes(iff, &text[len], cn->cn_Size) != cn->cn_Size)
						goto cleanup;

					len += cn->cn_Size;
				}
			}

			if (cn->cn_ID == ID_UTF8 && cn->cn_Size > 0)
			{
				if (!ignore_chrs)
				{
					if (text != NULL)
					{
						free(text);
						text = NULL;
						len = 0;
					}

					charset = CHARSET_UTF8;
					ignore_chrs = TRUE;
				}

				text = realloc(text, len + cn->cn_Size);
				if (text == NULL)
					goto cleanup;

				if (IIFFParse->ReadChunkBytes(iff, &text[len], cn->cn_Size) != cn->cn_Size)
					goto cleanup;

				len += cn->cn_Size;
			}
		}
	}

	if (text != NULL && len > 0)
	{
		if (charset == CHARSET_UTF8)
		{
			*utf8 = text;
			*utf8_len = len;
		}
		else
		{
			const ULONG *maptable;
			TEXT ch;
			STRPTR u8;
			ULONG i, u8_len;

			maptable = (const ULONG *)IDiskfont->ObtainCharsetInfo(DFCS_NUMBER, charset, DFCS_MAPTABLE);
			if (maptable == NULL)
				goto cleanup;

			u8_len = 0;
			for (i = 0; i < len; i++)
			{
				ch = text[i];
				if (ch >= 128)
				{
					u8_len += tsm_ucs4_get_len(maptable[(UBYTE)ch]);
				}
				else
				{
					u8_len++;
				}
			}

			u8 = malloc(u8_len + 1);
			if (u8 == NULL)
				goto cleanup;

			u8_len = 0;
			for (i = 0; i < len; i++)
			{
				ch = text[i];
				if (ch >= 128)
				{
					u8_len += tsm_ucs4_to_utf8(maptable[(UBYTE)ch], &u8[u8_len]);
				}
				else
				{
					u8[u8_len++] = ch;
				}
			}

			free(text);

			*utf8 = u8;
			*utf8_len = u8_len;
		}

		result = TRUE;
	}

cleanup:
	if (iff != NULL)
	{
		IIFFParse->CloseIFF(iff);

		if (iff->iff_Stream != (ULONG)NULL)
			IIFFParse->CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);

		IIFFParse->FreeIFF(iff);
	}

	if (IIFFParse != NULL)
		close_interface((struct Interface *)IIFFParse);

	if (!result && text != NULL)
		free(text);

	/* Re-enable DOS requesters. */
	IDOS->SetProcWindow(window);

	return result;
}

static ULONG TERM_paste(Class *cl, Object *obj, struct tpGeneric *tpg)
{
	struct TermData *td = INST_DATA(cl, obj);
	ULONG result = 0;
	STRPTR utf8 = NULL;
	ULONG utf8_len = 0;

	result = read_clip(PRIMARY_CLIP, &utf8, &utf8_len);
	if (result)
	{
		CONST_STRPTR u8 = utf8;
		CONST_STRPTR u8end = utf8 + utf8_len;
		mbstate_t ps;
		LONG mblen;
		wchar_t ucs4;

		memset(&ps, 0, sizeof(ps));

		while ((mblen = mbrtowc(&ucs4, u8, u8end - u8, &ps)) > 0)
		{
			UWORD code = (ucs4 == '\n') ? RAWKEY_RETURN : 0;

			tsm_vte_handle_keyboard_amiga(td->td_VTE, code, 0, ucs4);

			u8 += mblen;
		}

		free(utf8);

		if (tpg->tpg_GInfo != NULL)
		{
			IIntuition->DoRender(obj, tpg->tpg_GInfo, GREDRAW_UPDATE);
		}
	}

	return result;
}

static ULONG TERM_clearsb(Class *cl, Object *obj, struct tpGeneric *tpg)
{
	struct TermData *td = INST_DATA(cl, obj);

	tsm_screen_clear_sb(td->td_Con);

	if (tpg->tpg_GInfo != NULL)
	{
		td->td_SBTop     = tsm_screen_get_sb_top(td->td_Con);
		td->td_SBVisible = tsm_screen_get_sb_visible(td->td_Con);
		td->td_SBTotal   = tsm_screen_get_sb_total(td->td_Con);

		if (td->td_Scroller != NULL)
		{
			SetAttrsGI(td->td_Scroller, tpg->tpg_GInfo,
				SCROLLER_Total,   td->td_SBTotal,
				SCROLLER_Visible, td->td_SBVisible,
				SCROLLER_Top,     td->td_SBTop,
				TAG_END);
		}

		IIntuition->DoRender(obj, tpg->tpg_GInfo, GREDRAW_UPDATE);
	}

	return TRUE;
}

