/*
 * libtsm - VT Emulator
 *
 * Copyright (c) 2019-2020 Fredrik Wikstrom <fredrik@a500.org>
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

#include <devices/inputevent.h>
#include <libraries/keymap.h>

#include <xkbcommon/xkbcommon-keysyms.h>

SHL_EXPORT
bool tsm_vte_handle_keyboard_amiga(struct tsm_vte *vte, uint16_t code,
                                   uint16_t qualifier, uint32_t unicode)
{
	char val, u8[4];
	size_t len;

	if (!vte) {
		return false;
	}

	if (qualifier & IEQUALIFIER_LALT)
		vte->flags |= FLAG_PREPEND_ESCAPE;

	if (qualifier & IEQUALIFIER_CONTROL) {
		switch (unicode) {
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

	switch (code) {
		case RAWKEY_BACKSPACE:
			vte_write(vte, "\x08", 1);
			return true;
		case RAWKEY_TAB:
			vte_write(vte, "\x09", 1);
			return true;
		/*
		 TODO: What should we do with this key? Sending XOFF is awful as
		       there is no simple way on modern keyboards to send XON
		       again. If someone wants this, we can re-eanble it and set
		       some flag.
		case RAWKEY_BREAK:
			vte_write(vte, "\x13", 1);
			return true;
		*/
		case RAWKEY_PRINTSCR:
			vte_write(vte, "\x15", 1);
			return true;
		case RAWKEY_ESC:
			vte_write(vte, "\x1b", 1);
			return true;
		case RAWKEY_ENTER:
			if (vte->flags & FLAG_KEYPAD_APPLICATION_MODE) {
				vte_write(vte, "\eOM", 3);
				return true;
			}
			/* fallthrough */
		case RAWKEY_RETURN:
			if (vte->flags & FLAG_LINE_FEED_NEW_LINE_MODE)
				vte_write(vte, "\x0d\x0a", 2);
			else
				vte_write(vte, "\x0d", 1);
			return true;
		case RAWKEY_INSERT:
			vte_write(vte, "\e[2~", 4);
			return true;
		case RAWKEY_DEL:
			vte_write(vte, "\e[3~", 4);
			return true;
		case RAWKEY_PAGEUP:
			vte_write(vte, "\e[5~", 4);
			return true;
		case RAWKEY_PAGEDOWN:
			vte_write(vte, "\e[6~", 4);
			return true;
		case RAWKEY_CRSRUP:
			if (qualifier & IEQUALIFIER_CONTROL)
				vte_write(vte, "\e[1;5A", 6);
			else if (vte->flags & FLAG_CURSOR_KEY_MODE)
				vte_write(vte, "\eOA", 3);
			else
				vte_write(vte, "\e[A", 3);
			return true;
		case RAWKEY_CRSRDOWN:
			if (qualifier & IEQUALIFIER_CONTROL)
				vte_write(vte, "\e[1;5B", 6);
			else if (vte->flags & FLAG_CURSOR_KEY_MODE)
				vte_write(vte, "\eOB", 3);
			else
				vte_write(vte, "\e[B", 3);
			return true;
		case RAWKEY_CRSRRIGHT:
			if (qualifier & IEQUALIFIER_CONTROL)
				vte_write(vte, "\e[1;5C", 6);
			else if (vte->flags & FLAG_CURSOR_KEY_MODE)
				vte_write(vte, "\eOC", 3);
			else
				vte_write(vte, "\e[C", 3);
			return true;
		case RAWKEY_CRSRLEFT:
			if (qualifier & IEQUALIFIER_CONTROL)
				vte_write(vte, "\e[1;5D", 6);
			else if (vte->flags & FLAG_CURSOR_KEY_MODE)
				vte_write(vte, "\eOD", 3);
			else
				vte_write(vte, "\e[D", 3);
			return true;
		case RAWKEY_HOME:
			if (qualifier & IEQUALIFIER_CONTROL)
				vte_write(vte, "\e[1;5H", 6);
			else if (vte->flags & FLAG_CURSOR_KEY_MODE)
				vte_write(vte, "\eOH", 3);
			else
				vte_write(vte, "\e[H", 3);
			return true;
		case RAWKEY_END:
			if (qualifier & IEQUALIFIER_CONTROL)
				vte_write(vte, "\e[1;5F", 6);
			else if (vte->flags & FLAG_CURSOR_KEY_MODE)
				vte_write(vte, "\eOF", 3);
			else
				vte_write(vte, "\e[F", 3);
			return true;
		/* TODO: check what to transmit for functions keys when
		 * shift/ctrl etc. are pressed. Every terminal behaves
		 * differently here which is really weird.
		 * We now map F4 to F14 if shift is pressed and so on for all
		 * keys. However, such mappings should rather be done via
		 * xkb-configurations and we should instead add a flags argument
		 * to the CSIs as some of the keys here already do. */
		case RAWKEY_F1:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				vte_write(vte, "\e[23~", 5);
			else
				vte_write(vte, "\eOP", 3);
			return true;
		case RAWKEY_F2:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				vte_write(vte, "\e[24~", 5);
			else
				vte_write(vte, "\eOQ", 3);
			return true;
		case RAWKEY_F3:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				vte_write(vte, "\e[25~", 5);
			else
				vte_write(vte, "\eOR", 3);
			return true;
		case RAWKEY_F4:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				//vte_write(vte, "\e[1;2S", 6);
				vte_write(vte, "\e[26~", 5);
			else
				vte_write(vte, "\eOS", 3);
			return true;
		case RAWKEY_F5:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				//vte_write(vte, "\e[15;2~", 7);
				vte_write(vte, "\e[28~", 5);
			else
				vte_write(vte, "\e[15~", 5);
			return true;
		case RAWKEY_F6:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				//vte_write(vte, "\e[17;2~", 7);
				vte_write(vte, "\e[29~", 5);
			else
				vte_write(vte, "\e[17~", 5);
			return true;
		case RAWKEY_F7:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				//vte_write(vte, "\e[18;2~", 7);
				vte_write(vte, "\e[31~", 5);
			else
				vte_write(vte, "\e[18~", 5);
			return true;
		case RAWKEY_F8:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				//vte_write(vte, "\e[19;2~", 7);
				vte_write(vte, "\e[32~", 5);
			else
				vte_write(vte, "\e[19~", 5);
			return true;
		case RAWKEY_F9:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				//vte_write(vte, "\e[20;2~", 7);
				vte_write(vte, "\e[33~", 5);
			else
				vte_write(vte, "\e[20~", 5);
			return true;
		case RAWKEY_F10:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				//vte_write(vte, "\e[21;2~", 7);
				vte_write(vte, "\e[34~", 5);
			else
				vte_write(vte, "\e[21~", 5);
			return true;
		case RAWKEY_F11:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				vte_write(vte, "\e[23;2~", 7);
			else
				vte_write(vte, "\e[23~", 5);
			return true;
		case RAWKEY_F12:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				vte_write(vte, "\e[24;2~", 7);
			else
				vte_write(vte, "\e[24~", 5);
			return true;
		case RAWKEY_F13:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				vte_write(vte, "\e[25;2~", 7);
			else
				vte_write(vte, "\e[25~", 5);
			return true;
		case RAWKEY_F14:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				vte_write(vte, "\e[26;2~", 7);
			else
				vte_write(vte, "\e[26~", 5);
			return true;
		case RAWKEY_F15:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				vte_write(vte, "\e[28;2~", 7);
			else
				vte_write(vte, "\e[28~", 5);
			return true;
		/*
		case RAWKEY_F16:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				vte_write(vte, "\e[29;2~", 7);
			else
				vte_write(vte, "\e[29~", 5);
			return true;
		case RAWKEY_F17:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				vte_write(vte, "\e[31;2~", 7);
			else
				vte_write(vte, "\e[31~", 5);
			return true;
		case RAWKEY_F18:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				vte_write(vte, "\e[32;2~", 7);
			else
				vte_write(vte, "\e[32~", 5);
			return true;
		case RAWKEY_F19:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				vte_write(vte, "\e[33;2~", 7);
			else
				vte_write(vte, "\e[33~", 5);
			return true;
		case RAWKEY_F20:
			if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
				vte_write(vte, "\e[34;2~", 7);
			else
				vte_write(vte, "\e[34~", 5);
			return true;
		*/
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

