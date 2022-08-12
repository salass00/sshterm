/*
 * libtsm - VT Emulator
 *
 * Copyright (c) 2018 Aetf <aetf@unlimitedcodeworks.xyz>
 * Copyright (c) 2011-2013 David Herrmann <dh.herrmann@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <xkbcommon/xkbcommon-keysyms.h>

SHL_EXPORT
bool tsm_vte_handle_keyboard(struct tsm_vte *vte, uint32_t keysym,
                             uint32_t ascii, unsigned int mods,
                             uint32_t unicode)
{
	char val, u8[4];
	size_t len;
	uint32_t sym;

	if (!vte) {
		return false;
	}

	/* MOD1 (mostly labeled 'Alt') prepends an escape character to every
	 * input that is sent by a key.
	 * TODO: Transform this huge handler into a lookup table to save a lot
	 * of code and make such modifiers easier to implement.
	 * Also check whether altSendsEscape should be the default (xterm
	 * disables this by default, why?) and whether we should implement the
	 * fallback shifting that xterm does. */
	if (mods & TSM_ALT_MASK)
		vte->flags |= FLAG_PREPEND_ESCAPE;

	/* A user might actually use multiple layouts for keyboard input. The
	 * @keysym variable contains the actual keysym that the user used. But
	 * if this keysym is not in the ascii range, the input handler does
	 * check all other layouts that the user specified whether one of them
	 * maps the key to some ASCII keysym and provides this via @ascii.
	 * We always use the real keysym except when handling CTRL+<XY>
	 * shortcuts we use the ascii keysym. This is for compatibility to xterm
	 * et. al. so ctrl+c always works regardless of the currently active
	 * keyboard layout.
	 * But if no ascii-sym is found, we still use the real keysym. */
	sym = ascii;
	if (sym == XKB_KEY_NoSymbol)
		sym = keysym;

	if (mods & TSM_CONTROL_MASK) {
		switch (sym) {
		case XKB_KEY_2:
		case XKB_KEY_space:
			vte_write(vte, "\x00", 1);
			return true;
		case XKB_KEY_a:
		case XKB_KEY_A:
			vte_write(vte, "\x01", 1);
			return true;
		case XKB_KEY_b:
		case XKB_KEY_B:
			vte_write(vte, "\x02", 1);
			return true;
		case XKB_KEY_c:
		case XKB_KEY_C:
			vte_write(vte, "\x03", 1);
			return true;
		case XKB_KEY_d:
		case XKB_KEY_D:
			vte_write(vte, "\x04", 1);
			return true;
		case XKB_KEY_e:
		case XKB_KEY_E:
			vte_write(vte, "\x05", 1);
			return true;
		case XKB_KEY_f:
		case XKB_KEY_F:
			vte_write(vte, "\x06", 1);
			return true;
		case XKB_KEY_g:
		case XKB_KEY_G:
			vte_write(vte, "\x07", 1);
			return true;
		case XKB_KEY_h:
		case XKB_KEY_H:
			vte_write(vte, "\x08", 1);
			return true;
		case XKB_KEY_i:
		case XKB_KEY_I:
			vte_write(vte, "\x09", 1);
			return true;
		case XKB_KEY_j:
		case XKB_KEY_J:
			vte_write(vte, "\x0a", 1);
			return true;
		case XKB_KEY_k:
		case XKB_KEY_K:
			vte_write(vte, "\x0b", 1);
			return true;
		case XKB_KEY_l:
		case XKB_KEY_L:
			vte_write(vte, "\x0c", 1);
			return true;
		case XKB_KEY_m:
		case XKB_KEY_M:
			vte_write(vte, "\x0d", 1);
			return true;
		case XKB_KEY_n:
		case XKB_KEY_N:
			vte_write(vte, "\x0e", 1);
			return true;
		case XKB_KEY_o:
		case XKB_KEY_O:
			vte_write(vte, "\x0f", 1);
			return true;
		case XKB_KEY_p:
		case XKB_KEY_P:
			vte_write(vte, "\x10", 1);
			return true;
		case XKB_KEY_q:
		case XKB_KEY_Q:
			vte_write(vte, "\x11", 1);
			return true;
		case XKB_KEY_r:
		case XKB_KEY_R:
			vte_write(vte, "\x12", 1);
			return true;
		case XKB_KEY_s:
		case XKB_KEY_S:
			vte_write(vte, "\x13", 1);
			return true;
		case XKB_KEY_t:
		case XKB_KEY_T:
			vte_write(vte, "\x14", 1);
			return true;
		case XKB_KEY_u:
		case XKB_KEY_U:
			vte_write(vte, "\x15", 1);
			return true;
		case XKB_KEY_v:
		case XKB_KEY_V:
			vte_write(vte, "\x16", 1);
			return true;
		case XKB_KEY_w:
		case XKB_KEY_W:
			vte_write(vte, "\x17", 1);
			return true;
		case XKB_KEY_x:
		case XKB_KEY_X:
			vte_write(vte, "\x18", 1);
			return true;
		case XKB_KEY_y:
		case XKB_KEY_Y:
			vte_write(vte, "\x19", 1);
			return true;
		case XKB_KEY_z:
		case XKB_KEY_Z:
			vte_write(vte, "\x1a", 1);
			return true;
		case XKB_KEY_3:
		case XKB_KEY_bracketleft:
		case XKB_KEY_braceleft:
			vte_write(vte, "\x1b", 1);
			return true;
		case XKB_KEY_4:
		case XKB_KEY_backslash:
		case XKB_KEY_bar:
			vte_write(vte, "\x1c", 1);
			return true;
		case XKB_KEY_5:
		case XKB_KEY_bracketright:
		case XKB_KEY_braceright:
			vte_write(vte, "\x1d", 1);
			return true;
		case XKB_KEY_6:
		case XKB_KEY_grave:
		case XKB_KEY_asciitilde:
			vte_write(vte, "\x1e", 1);
			return true;
		case XKB_KEY_7:
		case XKB_KEY_slash:
		case XKB_KEY_question:
			vte_write(vte, "\x1f", 1);
			return true;
		case XKB_KEY_8:
			vte_write(vte, "\x7f", 1);
			return true;
		}
	}

	switch (keysym) {
		case XKB_KEY_BackSpace:
			if (vte->backspace_sends_delete)
				vte_write(vte, "\x7f", 1);
			else
				vte_write(vte, "\x08", 1);
			return true;
		case XKB_KEY_Tab:
		case XKB_KEY_KP_Tab:
			vte_write(vte, "\x09", 1);
			return true;
		case XKB_KEY_ISO_Left_Tab:
			vte_write(vte, "\e[Z", 3);
			return true;
		case XKB_KEY_Linefeed:
			vte_write(vte, "\x0a", 1);
			return true;
		case XKB_KEY_Clear:
			vte_write(vte, "\x0b", 1);
			return true;
		/*
		 TODO: What should we do with this key? Sending XOFF is awful as
		       there is no simple way on modern keyboards to send XON
		       again. If someone wants this, we can re-eanble it and set
		       some flag.
		case XKB_KEY_Pause:
			vte_write(vte, "\x13", 1);
			return true;
		*/
		/*
		 TODO: What should we do on scroll-lock? Sending 0x14 is what
		       the specs say but it is not used today the way most
		       users would expect so we disable it. If someone wants
		       this, we can re-enable it and set some flag.
		case XKB_KEY_Scroll_Lock:
			vte_write(vte, "\x14", 1);
			return true;
		*/
		case XKB_KEY_Sys_Req:
			vte_write(vte, "\x15", 1);
			return true;
		case XKB_KEY_Escape:
			vte_write(vte, "\x1b", 1);
			return true;
		case XKB_KEY_KP_Enter:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE) {
				vte_write(vte, "\eOM", 3);
				return true;
			}
			/* fallthrough */
		case XKB_KEY_Return:
			if (vte->flags & FLAG_LINE_FEED_NEW_LINE_MODE)
				vte_write(vte, "\x0d\x0a", 2);
			else
				vte_write(vte, "\x0d", 1);
			return true;
		case XKB_KEY_Find:
			vte_write(vte, "\e[1~", 4);
			return true;
		case XKB_KEY_Insert:
			vte_write(vte, "\e[2~", 4);
			return true;
		case XKB_KEY_Delete:
			vte_write(vte, "\e[3~", 4);
			return true;
		case XKB_KEY_Select:
			vte_write(vte, "\e[4~", 4);
			return true;
		case XKB_KEY_Page_Up:
		case XKB_KEY_KP_Page_Up:
			vte_write(vte, "\e[5~", 4);
			return true;
		case XKB_KEY_KP_Page_Down:
		case XKB_KEY_Page_Down:
			vte_write(vte, "\e[6~", 4);
			return true;
		case XKB_KEY_Up:
		case XKB_KEY_KP_Up:
			if (mods & TSM_CONTROL_MASK)
				vte_write(vte, "\e[1;5A", 6);
			else if (vte->flags & FLAG_CURSOR_KEY_MODE)
				vte_write(vte, "\eOA", 3);
			else
				vte_write(vte, "\e[A", 3);
			return true;
		case XKB_KEY_Down:
		case XKB_KEY_KP_Down:
			if (mods & TSM_CONTROL_MASK) {
				vte_write(vte, "\e[1;5B", 6);
			else if (vte->flags & FLAG_CURSOR_KEY_MODE)
				vte_write(vte, "\eOB", 3);
			else
				vte_write(vte, "\e[B", 3);
			return true;
		case XKB_KEY_Right:
		case XKB_KEY_KP_Right:
			if (mods & TSM_CONTROL_MASK) {
				vte_write(vte, "\e[1;5C", 6);
			else if (vte->flags & FLAG_CURSOR_KEY_MODE)
				vte_write(vte, "\eOC", 3);
			else
				vte_write(vte, "\e[C", 3);
			return true;
		case XKB_KEY_Left:
		case XKB_KEY_KP_Left:
			if (mods & TSM_CONTROL_MASK) {
				vte_write(vte, "\e[1;5D", 6);
			else if (vte->flags & FLAG_CURSOR_KEY_MODE)
				vte_write(vte, "\eOD", 3);
			else
				vte_write(vte, "\e[D", 3);
			return true;
		case XKB_KEY_KP_Insert:
		case XKB_KEY_KP_0:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOp", 3);
			else
				vte_write(vte, "0", 1);
			return true;
		case XKB_KEY_KP_1:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOq", 3);
			else
				vte_write(vte, "1", 1);
			return true;
		case XKB_KEY_KP_2:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOr", 3);
			else
				vte_write(vte, "2", 1);
			return true;
		case XKB_KEY_KP_3:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOs", 3);
			else
				vte_write(vte, "3", 1);
			return true;
		case XKB_KEY_KP_4:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOt", 3);
			else
				vte_write(vte, "4", 1);
			return true;
		case XKB_KEY_KP_5:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOu", 3);
			else
				vte_write(vte, "5", 1);
			return true;
		case XKB_KEY_KP_6:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOv", 3);
			else
				vte_write(vte, "6", 1);
			return true;
		case XKB_KEY_KP_7:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOw", 3);
			else
				vte_write(vte, "7", 1);
			return true;
		case XKB_KEY_KP_8:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOx", 3);
			else
				vte_write(vte, "8", 1);
			return true;
		case XKB_KEY_KP_9:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOy", 3);
			else
				vte_write(vte, "9", 1);
			return true;
		case XKB_KEY_KP_Subtract:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOm", 3);
			else
				vte_write(vte, "-", 1);
			return true;
		case XKB_KEY_KP_Separator:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOl", 3);
			else
				vte_write(vte, ",", 1);
			return true;
		case XKB_KEY_KP_Delete:
		case XKB_KEY_KP_Decimal:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOn", 3);
			else
				vte_write(vte, ".", 1);
			return true;
		case XKB_KEY_KP_Equal:
		case XKB_KEY_KP_Divide:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOj", 3);
			else
				vte_write(vte, "/", 1);
			return true;
		case XKB_KEY_KP_Multiply:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOo", 3);
			else
				vte_write(vte, "*", 1);
			return true;
		case XKB_KEY_KP_Add:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE)
				vte_write(vte, "\eOk", 3);
			else
				vte_write(vte, "+", 1);
			return true;
		case XKB_KEY_Home:
		case XKB_KEY_KP_Home:
			if (mods & TSM_CONTROL_MASK)
				vte_write(vte, "\e[1;5H", 6);
			else if (vte->flags & FLAG_CURSOR_KEY_MODE)
				vte_write(vte, "\eOH", 3);
			else
				vte_write(vte, "\e[H", 3);
			return true;
		case XKB_KEY_End:
		case XKB_KEY_KP_End:
			if (mods & TSM_CONTROL_MASK)
				vte_write(vte, "\e[1;5F", 6);
			else if (vte->flags & FLAG_CURSOR_KEY_MODE)
				vte_write(vte, "\eOF", 3);
			else
				vte_write(vte, "\e[F", 3);
			return true;
		case XKB_KEY_KP_Space:
			vte_write(vte, " ", 1);
			return true;
		/* TODO: check what to transmit for functions keys when
		 * shift/ctrl etc. are pressed. Every terminal behaves
		 * differently here which is really weird.
		 * We now map F4 to F14 if shift is pressed and so on for all
		 * keys. However, such mappings should rather be done via
		 * xkb-configurations and we should instead add a flags argument
		 * to the CSIs as some of the keys here already do. */
		case XKB_KEY_F1:
		case XKB_KEY_KP_F1:
			if (mods & TSM_SHIFT_MASK)
				vte_write(vte, "\e[23~", 5);
			else
				vte_write(vte, "\eOP", 3);
			return true;
		case XKB_KEY_F2:
		case XKB_KEY_KP_F2:
			if (mods & TSM_SHIFT_MASK)
				vte_write(vte, "\e[24~", 5);
			else
				vte_write(vte, "\eOQ", 3);
			return true;
		case XKB_KEY_F3:
		case XKB_KEY_KP_F3:
			if (mods & TSM_SHIFT_MASK)
				vte_write(vte, "\e[25~", 5);
			else
				vte_write(vte, "\eOR", 3);
			return true;
		case XKB_KEY_F4:
		case XKB_KEY_KP_F4:
			if (mods & TSM_SHIFT_MASK)
				//vte_write(vte, "\e[1;2S", 6);
				vte_write(vte, "\e[26~", 5);
			else
				vte_write(vte, "\eOS", 3);
			return true;
		case XKB_KEY_F5:
			if (mods & TSM_SHIFT_MASK)
				//vte_write(vte, "\e[15;2~", 7);
				vte_write(vte, "\e[28~", 5);
			else
				vte_write(vte, "\e[15~", 5);
			return true;
		case XKB_KEY_F6:
			if (mods & TSM_SHIFT_MASK)
				//vte_write(vte, "\e[17;2~", 7);
				vte_write(vte, "\e[29~", 5);
			else
				vte_write(vte, "\e[17~", 5);
			return true;
		case XKB_KEY_F7:
			if (mods & TSM_SHIFT_MASK)
				//vte_write(vte, "\e[18;2~", 7);
				vte_write(vte, "\e[31~", 5);
			else
				vte_write(vte, "\e[18~", 5);
			return true;
		case XKB_KEY_F8:
			if (mods & TSM_SHIFT_MASK)
				//vte_write(vte, "\e[19;2~", 7);
				vte_write(vte, "\e[32~", 5);
			else
				vte_write(vte, "\e[19~", 5);
			return true;
		case XKB_KEY_F9:
			if (mods & TSM_SHIFT_MASK)
				//vte_write(vte, "\e[20;2~", 7);
				vte_write(vte, "\e[33~", 5);
			else
				vte_write(vte, "\e[20~", 5);
			return true;
		case XKB_KEY_F10:
			if (mods & TSM_SHIFT_MASK)
				//vte_write(vte, "\e[21;2~", 7);
				vte_write(vte, "\e[34~", 5);
			else
				vte_write(vte, "\e[21~", 5);
			return true;
		case XKB_KEY_F11:
			if (mods & TSM_SHIFT_MASK)
				vte_write(vte, "\e[23;2~", 7);
			else
				vte_write(vte, "\e[23~", 5);
			return true;
		case XKB_KEY_F12:
			if (mods & TSM_SHIFT_MASK)
				vte_write(vte, "\e[24;2~", 7);
			else
				vte_write(vte, "\e[24~", 5);
			return true;
		case XKB_KEY_F13:
			if (mods & TSM_SHIFT_MASK)
				vte_write(vte, "\e[25;2~", 7);
			else
				vte_write(vte, "\e[25~", 5);
			return true;
		case XKB_KEY_F14:
			if (mods & TSM_SHIFT_MASK)
				vte_write(vte, "\e[26;2~", 7);
			else
				vte_write(vte, "\e[26~", 5);
			return true;
		case XKB_KEY_F15:
			if (mods & TSM_SHIFT_MASK)
				vte_write(vte, "\e[28;2~", 7);
			else
				vte_write(vte, "\e[28~", 5);
			return true;
		case XKB_KEY_F16:
			if (mods & TSM_SHIFT_MASK)
				vte_write(vte, "\e[29;2~", 7);
			else
				vte_write(vte, "\e[29~", 5);
			return true;
		case XKB_KEY_F17:
			if (mods & TSM_SHIFT_MASK)
				vte_write(vte, "\e[31;2~", 7);
			else
				vte_write(vte, "\e[31~", 5);
			return true;
		case XKB_KEY_F18:
			if (mods & TSM_SHIFT_MASK)
				vte_write(vte, "\e[32;2~", 7);
			else
				vte_write(vte, "\e[32~", 5);
			return true;
		case XKB_KEY_F19:
			if (mods & TSM_SHIFT_MASK)
				vte_write(vte, "\e[33;2~", 7);
			else
				vte_write(vte, "\e[33~", 5);
			return true;
		case XKB_KEY_F20:
			if (mods & TSM_SHIFT_MASK)
				vte_write(vte, "\e[34;2~", 7);
			else
				vte_write(vte, "\e[34~", 5);
			return true;
	}

	if (unicode != TSM_VTE_INVALID) {
		if (vte->flags & FLAG_7BIT_MODE) {
			val = unicode;
			if (unicode & 0x80) {
				llog_debug(vte, "invalid keyboard input in 7bit mode U+%x; mapping to '?'",
					   unicode);
				val = '?';
			}
			vte_write(vte, &val, 1);
		} else if (vte->flags & FLAG_8BIT_MODE) {
			val = unicode;
			if (unicode > 0xff) {
				llog_debug(vte, "invalid keyboard input in 8bit mode U+%x; mapping to '?'",
					   unicode);
				val = '?';
			}
			vte_write_raw(vte, &val, 1);
		} else {
			len = tsm_ucs4_to_utf8(tsm_symbol_make(unicode), u8);
			vte_write_raw(vte, u8, len);
		}
		return true;
	}

	vte->flags &= ~FLAG_PREPEND_ESCAPE;
	return false;
}

