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

#include "timer.h"
#include <proto/exec.h>

struct TimeRequest *timer_open(ULONG unit)
{
	struct MsgPort *mp;
	struct TimeRequest *tr;
	LONG error;

	mp = IExec->AllocSysObject(ASOT_PORT, NULL);
	if (mp == NULL)
		return NULL;

	tr = IExec->AllocSysObjectTags(ASOT_IOREQUEST,
		ASOIOR_ReplyPort, mp,
		ASOIOR_Size,      sizeof(*tr),
		TAG_END);
	if (tr == NULL)
	{
		IExec->FreeSysObject(ASOT_PORT, mp);
		return NULL;
	}

	error = IExec->OpenDevice("timer.device", unit, &tr->Request, 0);
	if (error != IOERR_SUCCESS)
	{
		IExec->FreeSysObject(ASOT_IOREQUEST, tr);
		IExec->FreeSysObject(ASOT_PORT, mp);
		return NULL;
	}

	return tr;
}

void timer_close(struct TimeRequest *tr)
{
	if (tr != NULL)
	{
		struct MsgPort *mp = tr->Request.io_Message.mn_ReplyPort;

		IExec->FreeSysObject(ASOT_IOREQUEST, tr);
		IExec->FreeSysObject(ASOT_PORT, mp);
	}
}

ULONG timer_signal(const struct TimeRequest *tr)
{
	struct MsgPort *mp = tr->Request.io_Message.mn_ReplyPort;

	return (1 << mp->mp_SigBit);
}

void timer_start(struct TimeRequest *tr, ULONG msec)
{
	tr->Request.io_Command = TR_ADDREQUEST;
	tr->Time.Seconds       = msec / 1000;
	tr->Time.Microseconds  = msec % 1000;

	IExec->SendIO(&tr->Request);
}

void timer_end(struct TimeRequest *tr)
{
	IExec->WaitIO(&tr->Request);
}

void timer_abort(struct TimeRequest *tr)
{
	if (IExec->CheckIO(&tr->Request) == NULL)
		IExec->AbortIO(&tr->Request);

	IExec->WaitIO(&tr->Request);
}

