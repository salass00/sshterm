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

#ifndef SSHTERM_H
#define SSHTERM_H

#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <proto/z.h>
#include <proto/bsdsocket.h>
#include <proto/amissl.h>
#include <proto/diskfont.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/keymap.h>
#include <proto/layers.h>
#include <proto/icon.h>

#include <libssh2.h>

#include <stdlib.h>
#include <string.h>

extern Class *MenuClass;
extern Class *WindowClass;
extern Class *LabelClass;
extern Class *LayoutClass;
extern Class *ButtonClass;
extern Class *TermClass;

extern struct DiskObject *AppIcon;

extern ULONG AboutWindowPID;

struct Interface *open_interface(CONST_STRPTR name, int version);
void close_interface(struct Interface *interface);

int sshterm(int argc, char **argv);

struct TermWindow *termwin_open(struct Screen *screen, ULONG max_sb);
void termwin_close(struct TermWindow *tw);
void termwin_set_max_sb(struct TermWindow *tw, ULONG max_sb);
void termwin_write(struct TermWindow *tw, const char *buffer, size_t len);
void termwin_refresh(struct TermWindow *tw);
ULONG termwin_get_signals(struct TermWindow *tw);
BOOL termwin_handle_input(struct TermWindow *tw);
size_t termwin_poll(struct TermWindow *tw);
ssize_t termwin_read(struct TermWindow *tw, char *buffer, size_t len);
BOOL termwin_poll_new_size(struct TermWindow *tw);
void termwin_get_size(struct TermWindow *tw, UWORD *columns, UWORD *rows);

BOOL aboutwin_open(struct Screen *screen);
void aboutwin_close(void);

BOOL signal_pid(ULONG pid, ULONG sigmask);
BOOL find_pid(ULONG pid);

#endif /* SSHTERM_H */
