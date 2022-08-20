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

static ULONG signal_pid_func(const struct Hook *hook, ULONG pid, struct Process *proc) {
	ULONG sigmask = (ULONG)hook->h_Data;
	BOOL  result  = FALSE;

	if (proc->pr_ProcessID == pid) {
		if (sigmask != 0)
			IExec->Signal(&proc->pr_Task, sigmask);

		result = TRUE;
	}

	return result;
}

BOOL signal_pid(ULONG pid, ULONG sigmask) {
	struct Hook hook;
	BOOL        result = FALSE;

	if (pid != 0)
	{
		memset(&hook, 0, sizeof(hook));

		hook.h_Entry = (HOOKFUNC)signal_pid_func;
		hook.h_Data  = (APTR)sigmask;

		if (IDOS->ProcessScan(&hook, (APTR)pid, 0))
			result = TRUE;
	}

	return result;
}

BOOL find_pid(ULONG pid) {
	return signal_pid(pid, 0);
}

