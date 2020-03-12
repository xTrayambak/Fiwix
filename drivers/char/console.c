/*
 * fiwix/drivers/char/console.c
 *
 * Copyright 2018, Jordi Sanfeliu. All rights reserved.
 * Distributed under the terms of the Fiwix License.
 */

#include <fiwix/asm.h>
#include <fiwix/kernel.h>
#include <fiwix/ctype.h>
#include <fiwix/console.h>
#include <fiwix/devices.h>
#include <fiwix/tty.h>
#include <fiwix/keyboard.h>
#include <fiwix/sleep.h>
#include <fiwix/pit.h>
#include <fiwix/timer.h>
#include <fiwix/process.h>
#include <fiwix/sched.h>
#include <fiwix/kd.h>
#include <fiwix/stdio.h>
#include <fiwix/string.h>

#define CSI_J_CUR2END	0	/* clear from cursor to end of screen */
#define CSI_J_STA2CUR	1	/* clear from start of screen to cursor */
#define CSI_J_SCREEN	2	/* clear entire screen */

#define CSI_K_CUR2END	0	/* clear from cursor to end of line */
#define CSI_K_STA2CUR	1	/* clear from start of line to cursor */
#define CSI_K_LINE	2	/* clear entire line */

#define CSE		vc->esc = 0	/* Code Set End */

#define ON		1
#define OFF		0

#define SCROLL_UP	1
#define SCROLL_DOWN	2

/* VT100 ID string generated by <ESC>Z or <ESC>[c */
#define VT100ID		"\033[?1;2c"

/* VT100 report status generated by <ESC>[5n */
#define DEVICE_OK	"\033[0n"
#define DEVICE_NOT_OK	"\033[3n"

/* ISO/IEC 8859-1:1998 (aka latin1, IBM819, CP819), same as in Linux */
static const char *iso8859 =
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	" !\"#$%&'()*+,-./0123456789:;<=>?"
	"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
	"`abcdefghijklmnopqrstuvwxyz{|}~\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\377\255\233\234\376\235\174\025\376\376\246\256\252\055\376\376"
	"\370\361\375\376\376\346\024\371\376\376\247\257\254\253\376\250"
	"\376\376\376\376\216\217\222\200\376\220\376\376\376\376\376\376"
	"\376\245\376\376\376\376\231\376\350\376\376\376\232\376\376\341"
	"\205\240\203\376\204\206\221\207\212\202\210\211\215\241\214\213"
	"\376\244\225\242\223\376\224\366\355\227\243\226\201\376\376\230"
;

unsigned short int *video_base_address;
short int current_cons;
unsigned char screen_is_off = 0;
int buf_y, buf_top;

struct vconsole vc[NR_VCONSOLES + 1];
unsigned short int vcbuf[VC_BUF_SIZE];

static struct fs_operations tty_driver_fsop = {
	0,
	0,

	tty_open,
	tty_close,
	tty_read,
	tty_write,
	tty_ioctl,
	tty_lseek,
	NULL,			/* readdir */
	NULL,			/* mmap */
	tty_select,

	NULL,			/* readlink */
	NULL,			/* followlink */
	NULL,			/* bmap */
	NULL,			/* lookup */
	NULL,			/* rmdir */
	NULL,			/* link */
	NULL,			/* unlink */
	NULL,			/* symlink */
	NULL,			/* mkdir */
	NULL,			/* mknod */
	NULL,			/* truncate */
	NULL,			/* create */
	NULL,			/* rename */

	NULL,			/* read_block */
	NULL,			/* write_block */

	NULL,			/* read_inode */
	NULL,			/* write_inode */
	NULL,			/* ialloc */
	NULL,			/* ifree */
	NULL,			/* statfs */
	NULL,			/* read_superblock */
	NULL,			/* remount_fs */
	NULL,			/* write_superblock */
	NULL			/* release_superblock */
};

static struct device tty_device = {
	"vconsole",
	KEYBOARD_IRQ,
	VCONSOLES_MAJOR,
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	0,
	NULL,
	&tty_driver_fsop,
};

static struct device console_device = {
	"console",
	KEYBOARD_IRQ,
	SYSCON_MAJOR,
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	0,
	NULL,
	&tty_driver_fsop,
};

unsigned short int ansi_color_table[] = {
	COLOR_BLACK,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BROWN,
	COLOR_BLUE,
	COLOR_MAGENTA,
	COLOR_CYAN,
	COLOR_WHITE
};

static void update_curpos(struct vconsole *vc)
{
	unsigned short int curpos;

	if(vc->has_focus) {
		curpos = (vc->y * vc->columns) + vc->x;
		outport_b(video_port + CRT_INDEX, CRT_CURSOR_POS_HI);
		outport_b(video_port + CRT_DATA, (curpos >> 8) & 0xFF);
		outport_b(video_port + CRT_INDEX, CRT_CURSOR_POS_LO);
		outport_b(video_port + CRT_DATA, (curpos & 0xFF));
	}
}

static void show_cursor(int mode)
{
	int status;

	switch(mode) {
		case ON:
			outport_b(video_port + CRT_INDEX, CRT_CURSOR_STR);
			status = inport_b(video_port + CRT_DATA);
			outport_b(video_port + CRT_DATA, status & CURSOR_MASK);
			break;
		case OFF:
			outport_b(video_port + CRT_INDEX, CRT_CURSOR_STR);
			status = inport_b(video_port + CRT_DATA);
			outport_b(video_port + CRT_DATA, status | CURSOR_DISABLE);
			break;
	}
}

static void get_curpos(struct vconsole *vc)
{
	unsigned short int curpos;

	outport_b(video_port + CRT_INDEX, CRT_CURSOR_POS_HI);
	curpos = inport_b(video_port + CRT_DATA) << 8;
	outport_b(video_port + CRT_INDEX, CRT_CURSOR_POS_LO);
	curpos |= inport_b(video_port + CRT_DATA);

	vc->x = curpos % vc->columns;
	vc->y = curpos / vc->columns;
}	

static void adjust(struct vconsole *vc, int x, int y)
{
	if(x < 0) {
		x = 0;
	}
	if(x >= vc->columns) {
		x = vc->columns - 1;
	}
	if(y < 0) {
		y = 0;
	}
	if(y >= vc->bottom) {
		y = vc->bottom - 1;
	}
	vc->x = x;
	vc->y = y;
}

static void delete_char(struct vconsole *vc)
{
	int n, from;

	from = (vc->y * vc->columns) + vc->x;
	n = vc->x;
	while(++n < vc->columns) {
		memcpy_w(vc->vidmem + from, vc->vidmem + from + 1, 1);
		from++;
	}
	memset_w(vc->vidmem + from, BLANK_MEM, 1);
}

static void insert_char(struct vconsole *vc)
{
	int n, from;
	unsigned short int tmp, last_char;

	from = (vc->y * vc->columns) + vc->x;
	n = vc->x + 1;
	last_char = BLANK_MEM;
	while(++n < vc->columns) {
		memcpy_w(&tmp, vc->vidmem + from, 1);
		memset_w(vc->vidmem + from, last_char, 1);
		last_char = tmp;
		from++;
	}
}

static void scroll_screen(struct vconsole *vc, int top, int mode)
{
	int n, count, from;

	if(!top) {
		top = vc->top;
	}
	switch(mode) {
		case SCROLL_UP:
			count = (vc->columns * (vc->bottom - top - 1)) * 2;
			from = top * vc->columns;
			top = (top + 1) * vc->columns;
			memcpy_b(vc->vidmem + from, vc->vidmem + top, count);
			memset_w(vc->vidmem + from + (count / 2), BLANK_MEM, (top * 2) / sizeof(unsigned short int));
			break;
		case SCROLL_DOWN:
			count = vc->columns * 2;
			for(n = vc->bottom - 1; n >= top; n--) {
				memcpy_b(vc->vidmem + (vc->columns * (n + 1)), vc->vidmem + (vc->columns * n), count);
			}
			memset_w(vc->vidmem + (top * vc->columns), BLANK_MEM, count / sizeof(unsigned short int));
			break;
	}
	return;
}

static void cr(struct vconsole *vc)
{
	vc->x = 0;
}

static void lf(struct vconsole *vc)
{
	if(vc->y == vc->bottom) {
		scroll_screen(vc, 0, SCROLL_UP);
	} else {
		vc->y++;
	}
}

static void ri(struct vconsole *vc)
{
	if(vc->y == 0) {
		scroll_screen(vc, vc->y, SCROLL_DOWN);
	} else {
		vc->y--;
	}
}

static void csi_J(struct vconsole *vc, int mode)
{
	int from, count;

	switch(mode) {
		case CSI_J_CUR2END:	/* Erase Down <ESC>[J */
			from = (vc->y * vc->columns) + vc->x;
			count = (SCREEN_SIZE - from) / sizeof(unsigned short int);
			break;
		case CSI_J_STA2CUR:	/* Erase Up <ESC>[1J */
			from = 0;
			count = (((vc->y * vc->columns) + vc->x) * 2) / sizeof(unsigned short int);
			break;
		case CSI_J_SCREEN:	/* Erase Screen <ESC>[2J */
			from = 0;
			count = SCREEN_SIZE / sizeof(unsigned short int);
			break;
		default:
			return;
	}
	memset_w(vc->vidmem + from, vc->color_attr, count);
}

static void csi_K(struct vconsole *vc, int mode)
{
	int from, count;

	switch(mode) {
		case CSI_K_CUR2END:	/* Erase End of Line <ESC>[K */
			from = (vc->y * vc->columns) + vc->x;
			count = ((vc->columns - vc->x) * 2) / sizeof(unsigned short int);
			break;
		case CSI_K_STA2CUR:	/* Erase Start of Line <ESC>[1K */
			from = vc->y * vc->columns;
			count = (vc->x * 2) / sizeof(unsigned short int);
			break;
		case CSI_K_LINE:	/* Erase Line <ESC>[2K */
			from = vc->y * vc->columns;
			count = (vc->columns * 2) / sizeof(unsigned short int);
			break;
		default:
			return;
	}
	memset_w(vc->vidmem + from, vc->color_attr, count);
}

static void csi_L(struct vconsole *vc, int count)
{
	if(count > (vc->bottom - vc->top)) {
		count = vc->bottom - vc->top;
	}
	while(count--) {
		scroll_screen(vc, vc->y, SCROLL_DOWN);
	}
}

static void csi_M(struct vconsole *vc, int count)
{
	if(count > (vc->bottom - vc->top)) {
		count = vc->bottom - vc->top;
	}
	while(count--) {
		scroll_screen(vc, vc->y, SCROLL_UP);
	}
}

static void csi_P(struct vconsole *vc, int count)
{
	if(count > vc->columns) {
		count = vc->columns;
	}
	while(count--) {
		delete_char(vc);
	}
}

static void csi_at(struct vconsole *vc, int count)
{
	if(count > vc->columns) {
		count = vc->columns;
	}
	while(count--) {
		insert_char(vc);
	}
}

static void default_color_attr(struct vconsole *vc)
{
	vc->color_attr = DEF_MODE;
	vc->bold = 0;
	vc->underline = 0;
	vc->blink = 0;
	vc->reverse = 0;
}

static void csi_m(struct vconsole *vc)
{
	if(vc->reverse) {
		vc->color_attr = ((vc->color_attr & 0x7000) >> 4) | ((vc->color_attr & 0x0700) << 4) | (vc->color_attr & 0x8800); 
	}

	switch(vc->parmv1) {
		case COLOR_NORMAL:
			default_color_attr(vc);
			break;
		case COLOR_BOLD:
			vc->bold = 1;
			break;
		case COLOR_BOLD_OFF:
			vc->bold = 0;
			break;
		case COLOR_BLINK:
			vc->blink = 1;
			break;
		case COLOR_REVERSE:
			vc->reverse = 1;
			break;
		case 21:
		case 22:
			vc->bold = 1;
			break;
		case 25:
			vc->blink = 0;
			break;
		case 27:
			vc->reverse = 0;
			break;
	}
	if(vc->parmv1 >= 30 && vc->parmv1 <= 37) {
		vc->color_attr = (vc->color_attr & 0xF8FF) | (ansi_color_table[vc->parmv1 - 30]);
	}
	if(vc->parmv1 >= 40 && vc->parmv1 <= 47) {
		vc->color_attr = (vc->color_attr & 0x8FFF) | ((ansi_color_table[vc->parmv1 - 40]) << 4);
	}
	if(vc->parmv2 >= 30 && vc->parmv2 <= 37) {
		vc->color_attr = (vc->color_attr & 0xF8FF) | (ansi_color_table[vc->parmv2 - 30]);
	}
	if(vc->parmv2 >= 40 && vc->parmv2 <= 47) {
		vc->color_attr = (vc->color_attr & 0x8FFF) | ((ansi_color_table[vc->parmv2 - 40]) << 4);
	}
	if(vc->bold) {
		vc->color_attr |= 0x0800;
	}
	if(vc->blink) {
		vc->color_attr |= 0x8000;
	}
	if(vc->reverse) {
		vc->color_attr = ((vc->color_attr & 0x7000) >> 4) | ((vc->color_attr & 0x0700) << 4) | (vc->color_attr & 0x8800); 
	}
}

static void init_vt(struct vconsole *vc)
{
	vc->vt_mode.mode = VT_AUTO;
	vc->vt_mode.waitv = 0;
	vc->vt_mode.relsig = 0;
	vc->vt_mode.acqsig = 0;
	vc->vt_mode.frsig = 0;
	vc->vc_mode = KD_TEXT;
	vc->tty->pid = 0;
	vc->switchto_tty = -1;
}

static void insert_seq(struct tty *tty, char *buf, int count)
{
	while(count--) {
		tty_queue_putchar(tty, &tty->read_q, *(buf++));
	}
	tty->input(tty);
}

static void echo_char(struct vconsole *vc, unsigned char *buf, unsigned int count)
{
	int n;
	unsigned char ch;

	if(vc->has_focus) {
		if(buf_top) {
			vconsole_restore(vc);
			show_cursor(ON);
			buf_top = 0;
		}
	}

	while(count--) {
		ch = *buf++;
		if(ch == NULL) {
			continue;

		} else if(ch == '\b') {
			if(vc->x) {
				vc->x--;
			}

		} else if(ch == '\a') {
			vconsole_beep();

		} else if(ch == '\r') {
			cr(vc);

		} else if(ch == '\n') {
			cr(vc);
			vc->y++;
			if(vc->has_focus) {
				buf_y++;
			}

		} else if(ch == '\t') {
			while(vc->x < (vc->columns - 1)) {
				if(vc->tab_stop[++vc->x]) {
					break;
				}
			}
/*			vc->x += TAB_SIZE - (vc->x % TAB_SIZE); */
			vc->check_x = 1;

		} else {
			if((vc->x == vc->columns - 1) && vc->check_x) {
				vc->x = 0;
				vc->y++;
				if(vc->has_focus) {
					buf_y++;
				}
			}
			if(vc->y >= vc->bottom) {
				scroll_screen(vc, 0, SCROLL_UP);
				vc->y--;
			}
			ch = iso8859[ch];
			vc->vidmem[(vc->y * vc->columns) + vc->x] = vc->color_attr | ch;
			if(vc->has_focus) {
				vcbuf[(buf_y * vc->columns) + vc->x] = vc->color_attr | ch;
			}
			if(vc->x < vc->columns - 1) {
				vc->check_x = 0;
				vc->x++;
			} else {
				vc->check_x = 1;
			}
		}
		if(vc->y >= vc->bottom) {
			scroll_screen(vc, 0, SCROLL_UP);
			vc->y--;
		}
		if(vc->has_focus) {
			if(buf_y >= VC_BUF_LINES) {
				memcpy_b(vcbuf, vcbuf + SCREEN_COLS, VC_BUF_SIZE - (SCREEN_COLS * 2));
				for(n = (SCREEN_COLS * (VC_BUF_LINES - 1)); n < (SCREEN_COLS * VC_BUF_LINES); n++) {
					vcbuf[n] = BLANK_MEM;
				}
				buf_y--;
			}
		}
	}
	update_curpos(vc);
}

void vconsole_reset(struct tty *tty)
{
	int n;
	struct vconsole *vc;

	vc = (struct vconsole *)tty->driver_data;

	vc->top = 0;
	vc->bottom = SCREEN_LINES;
	vc->columns = SCREEN_COLS;
	vc->check_x = 0;
	vc->led_status = 0;
	set_leds(vc->led_status);
	vc->scrlock = vc->numlock = vc->capslock = 0;
	vc->esc = vc->sbracket = vc->semicolon = vc->question = 0;
	vc->parmv1 = vc->parmv2 = 0;
	default_color_attr(vc);
	vc->insert_mode = 0;
	vc->saved_x = vc->saved_y = 0;

	for(n = 0; n < MAX_TAB_COLS; n++) {
		if(!(n % TAB_SIZE)) {
			vc->tab_stop[n] = 1;
		} else {
			vc->tab_stop[n] = 0;
		}
	}

	termios_reset(tty);
	vc->tty->winsize.ws_row = vc->bottom - vc->top;
	vc->tty->winsize.ws_col = vc->columns;
	vc->tty->winsize.ws_xpixel = 0;
	vc->tty->winsize.ws_ypixel = 0;
	vc->tty->lnext = 0;

	init_vt(vc);
	vc->blanked = 0;
	update_curpos(vc);
}

void vconsole_write(struct tty *tty)
{
	int n;
	unsigned char ch;
	int numeric;
	struct vconsole *vc;

	vc = (struct vconsole *)tty->driver_data;

	if(buf_top) {
		vconsole_restore(vc);
		buf_top = 0;
		show_cursor(ON);
		update_curpos(vc);
	}

	numeric = 0;

	while(!vc->scrlock && tty->write_q.count > 0) {
		ch = tty_queue_getchar(&tty->write_q);

		if(vc->esc) {
			if(vc->sbracket) {
				if(IS_NUMERIC(ch)) {
					numeric = 1;
					if(vc->semicolon) {
						vc->parmv2 *= 10;
						vc->parmv2 += ch - '0';
					} else {
						vc->parmv1 *= 10;
						vc->parmv1 += ch - '0';
					}
					continue;
				}
				switch(ch) {
					case ';':
						vc->semicolon = 1;
						vc->parmv2 = 0;
						continue;
					case '?':
						vc->question = 1;
						continue;
					case 'A':	/* Cursor Up <ESC>[{COUNT}A */
						vc->parmv1 = !vc->parmv1 ? 1 : vc->parmv1;
						adjust(vc, vc->x, vc->y - vc->parmv1);
						CSE;
						continue;
					case 'B':	/* Cursor Down <ESC>[{COUNT}B */
						vc->parmv1 = !vc->parmv1 ? 1 : vc->parmv1;
						adjust(vc, vc->x, vc->y + vc->parmv1);
						CSE;
						continue;
					case 'C':	/* Cursor Forward <ESC>[{COUNT}C */
						vc->parmv1 = !vc->parmv1 ? 1 : vc->parmv1;
						adjust(vc, vc->x + vc->parmv1, vc->y);
						CSE;
						continue;
					case 'D':	/* Cursor Backward <ESC>[{COUNT}D */
						vc->parmv1 = !vc->parmv1 ? 1 : vc->parmv1;
						adjust(vc, vc->x - vc->parmv1, vc->y);
						CSE;
						continue;
					case 'E':	/* Cursor Next Line(s) <ESC>[{COUNT}E */
						vc->parmv1 = !vc->parmv1 ? 1 : vc->parmv1;
						adjust(vc, 0, vc->y + vc->parmv1);
						CSE;
						continue;
					case 'F':	/* Cursor Previous Line(s) <ESC>[{COUNT}F */
						vc->parmv1 = !vc->parmv1 ? 1 : vc->parmv1;
						adjust(vc, 0, vc->y - vc->parmv1);
						CSE;
						continue;
					case 'G':	/* Cursor Horizontal Position <ESC>[{NUM1}G */
					case '`':
						vc->parmv1 = vc->parmv1 ? vc->parmv1 - 1 : vc->parmv1;
						adjust(vc, vc->parmv1, vc->y);
						CSE;
						continue;
					case 'H':	/* Cursor Home <ESC>[{ROW};{COLUMN}H */
					case 'f':	/* Force Cursor Position <ESC>[{ROW};{COLUMN}f */
						vc->parmv1 = vc->parmv1 ? vc->parmv1 - 1 : vc->parmv1;
						vc->parmv2 = vc->parmv2 ? vc->parmv2 - 1 : vc->parmv2;
						adjust(vc, vc->parmv2, vc->parmv1);
						CSE;
						continue;
					case 'J':	/* Erase (Down/Up/Screen) <ESC>[J */
						csi_J(vc, vc->parmv1);
						CSE;
						continue;
					case 'K':	/* Erase (End of/Start of/) Line <ESC>[K */
						csi_K(vc, vc->parmv1);
						CSE;
						continue;
					case 'L':	/* Insert Line(s) <ESC>[{COUNT}L */
						vc->parmv1 = !vc->parmv1 ? 1 : vc->parmv1;
						csi_L(vc, vc->parmv1);
						CSE;
						continue;
					case 'M':	/* Delete Line(s) <ESC>[{COUNT}M */
						vc->parmv1 = !vc->parmv1 ? 1 : vc->parmv1;
						csi_M(vc, vc->parmv1);
						CSE;
						continue;
					case 'P':	/* Delete Character(s) <ESC>[{COUNT}P */
						vc->parmv1 = !vc->parmv1 ? 1 : vc->parmv1;
						csi_P(vc, vc->parmv1);
						CSE;
						continue;
					case '@':	/* Insert Character(s) <ESC>[{COUNT}@ */
						vc->parmv1 = !vc->parmv1 ? 1 : vc->parmv1;
						csi_at(vc, vc->parmv1);
						CSE;
						continue;
					case 'c':	/* Query Device Code <ESC>[c */
						if(!numeric) {
							insert_seq(tty, VT100ID, 7);
						}
						CSE;
						continue;
					case 'd':	/* Cursor Vertical Position <ESC>[{NUM1}d */
						vc->parmv1 = vc->parmv1 ? vc->parmv1 - 1 : vc->parmv1;
						adjust(vc, vc->x, vc->parmv1);
						CSE;
						continue;
					case 'g':
						switch(vc->parmv1) {
							case 0:	/* Clear Tab <ESC>[g */
								vc->tab_stop[vc->x] = 0;
								break;
							case 3:	/* Clear All Tabs <ESC>[3g */
								for(n = 0; n < MAX_TAB_COLS; n++)
									vc->tab_stop[n] = 0;
								break;
						}
						CSE;
						continue;
					case 'h':
						if(vc->question) {
							switch(vc->parmv1) {
								/* DEC modes */
								case 25: /* Switch Cursor Visible <ESC>[?25h */
									show_cursor(ON);
									break;
								case 4:
									vc->insert_mode = ON; /* not used */
									break;
							}
						}
						CSE;
						continue;
					case 'l':
						if(vc->question) {
							switch(vc->parmv1) {
								/* DEC modes */
								case 25: /* Switch Cursor Invisible <ESC>[?25l */
									show_cursor(OFF);
									break;
								case 4:
									vc->insert_mode = OFF; /* not used */
									break;
							}
						}
						CSE;
						continue;
					case 'm':	/* Character Attributes <ESC>{NUM1}{NUM2}m */
						csi_m(vc);
						CSE;
						continue;
					case 'n':
						if(!vc->question) {
							switch(vc->parmv1) {
								case 5:	/* Query Device Status <ESC>[5n */
									insert_seq(tty, DEVICE_OK, 4);
									break;
								case 6:	/* Query Cursor Position <ESC>[6n */
									{
										char curpos[8];
										char len;
										len = sprintk(curpos, "\033[%d;%dR", vc->y, vc->x);
										insert_seq(tty, curpos, len);
									}
									break;
							}
						}
						CSE;
						continue;
					case 'r':	/* Top and Bottom Margins <ESC>[r  / <ESC>[{start};{end}r */
						if(!vc->parmv1) {
							vc->parmv1++;
						}
						if(!vc->parmv2) {
							vc->parmv2 = SCREEN_LINES;
						}
						if(vc->parmv1 < vc->parmv2 && vc->parmv2 <= SCREEN_LINES) {
							vc->top = vc->parmv1 - 1;
							vc->bottom = vc->parmv2;
							adjust(vc, 0, 0);
						}
						CSE;
						continue;
					case 's':	/* Save Cursor <ESC>[s */
						vc->saved_x = vc->x;
						vc->saved_y = vc->y;
						CSE;
						continue;
					case 'u':	/* Restore Cursor <ESC>[u */
						vc->x = vc->saved_x;
						vc->y = vc->saved_y;
						CSE;
						continue;
					default:
						CSE;
						break;
				}
			} else {
				switch(ch) {
					case '[':
						vc->sbracket = 1;
						vc->semicolon = 0;
						vc->question = 0;
						vc->parmv1 = vc->parmv2 = 0;
						continue;
					case '7':	/* Save Cursor & Attrs <ESC>7 */
						vc->saved_x = vc->x;
						vc->saved_y = vc->y;
						CSE;
						continue;
					case '8':	/* Restore Cursor & Attrs <ESC>8 */
						vc->x = vc->saved_x;
						vc->y = vc->saved_y;
						CSE;
						continue;
					case 'D':	/* Scroll Down <ESC>D */
						lf(vc);
						CSE;
						continue;
					case 'E':	/* Move To Next Line <ESC>E */
						cr(vc);
						lf(vc);
						CSE;
						continue;
					case 'H':	/* Set Tab <ESC>H */
						vc->tab_stop[vc->x] = 1;
						CSE;
						continue;
					case 'M':	/* Scroll Up <ESC>M */
						ri(vc);
						CSE;
						continue;
					case 'Z':	/* Identify Terminal <ESC>Z */
						insert_seq(tty, VT100ID, 7);
						CSE;
						continue;
					case 'c':	/* Reset Device <ESC>c */
						vconsole_reset(vc->tty);
						vc->x = vc->y = 0;
						csi_J(vc, CSI_J_SCREEN);
						CSE;
						continue;
					default:
						CSE;
						break;
				}
			}
		}
		switch(ch) {
			case '\033':
				vc->esc = 1;
				vc->sbracket = 0;
				vc->semicolon = 0;
				vc->question = 0;
				vc->parmv1 = vc->parmv2 = 0;
				continue;
			default:
				echo_char(vc, &ch, 1);
				continue;
		}
	}
	if(vc->vc_mode != KD_GRAPHICS) {
		update_curpos(vc);
	}
	wakeup(&tty_write);
}

void vconsole_select(int new_cons)
{
	new_cons++;
	if(current_cons != new_cons) {
		if(vc[current_cons].vt_mode.mode == VT_PROCESS) {
			if(!kill_pid(vc[current_cons].tty->pid, vc[current_cons].vt_mode.acqsig)) {
				vc[current_cons].switchto_tty = new_cons;
				return;
			}
			init_vt(&vc[current_cons]);
		}
		if(vc[current_cons].vc_mode == KD_GRAPHICS) {
			return;
		}
		vconsole_select_final(new_cons);
	}
}

void vconsole_select_final(int new_cons)
{
	if(current_cons != new_cons) {
		if(vc[new_cons].vt_mode.mode == VT_PROCESS) {
			if(kill_pid(vc[new_cons].tty->pid, vc[new_cons].vt_mode.acqsig)) {
				init_vt(&vc[new_cons]);
			}
		}
		if(buf_top) {
			vconsole_restore(&vc[current_cons]);
			buf_top = 0;
			update_curpos(&vc[current_cons]);
		}
		if(vc[current_cons].vc_mode != KD_GRAPHICS) {
			vconsole_save(&vc[current_cons]);
		}
		vc[current_cons].vidmem = vc[current_cons].scrbuf;
		vc[current_cons].has_focus = 0;
		vc[new_cons].vidmem = video_base_address;
		vc[new_cons].has_focus = 1;
		vconsole_restore(&vc[new_cons]);
		current_cons = new_cons;
		set_leds(vc[current_cons].led_status);
		update_curpos(&vc[current_cons]);

		buf_y = vc[current_cons].y;
		buf_top = 0;
		memset_w(vcbuf, BLANK_MEM, VC_BUF_SIZE / sizeof(unsigned short int));
		memcpy_b(vcbuf, vc[current_cons].vidmem, SCREEN_SIZE);
		show_cursor(ON);
	}
}

void vconsole_save(struct vconsole *vc)
{
	memcpy_b(vc->scrbuf, vc->vidmem, SCREEN_SIZE);
}

void vconsole_restore(struct vconsole *vc)
{
	memcpy_b(vc->vidmem, vc->scrbuf, SCREEN_SIZE);
}

void vconsole_buffer_scrl(int mode)
{
	int buf_line = buf_y;

	if(buf_line <= SCREEN_LINES) {
		return;
	}
	if(mode == VC_BUF_UP) {
		if(buf_top < 0) {
			return;
		}
		if(!buf_top) {
			vconsole_save(&vc[current_cons]);
			buf_top = (buf_line - SCREEN_LINES + 1) * SCREEN_COLS;
			buf_top -= (SCREEN_LINES / 2) * SCREEN_COLS;
		} else {
			buf_top -= (SCREEN_LINES / 2) * SCREEN_COLS;
		}
		if(buf_top < 0) {
			buf_top = 0;
		}
		memcpy_b(vc[current_cons].vidmem, vcbuf + buf_top, SCREEN_SIZE);
		if(!buf_top) {
			buf_top = -1;
		}
		show_cursor(OFF);
		return;
	}
	if(mode == VC_BUF_DOWN) {
		if(!buf_top) {
			return;
		}
		if(buf_top == buf_line * SCREEN_COLS) {
			return;
		}
		if(buf_top < 0) {
			buf_top = 0;
		}
		buf_top += (SCREEN_LINES / 2) * SCREEN_COLS;
		if(buf_top >= (buf_line - SCREEN_LINES + 1) * SCREEN_COLS) {
			vconsole_restore(&vc[current_cons]);
			buf_top = 0;
			show_cursor(ON);
			update_curpos(&vc[current_cons]);
			return;
		}
		memcpy_b(vc[current_cons].vidmem, vcbuf + buf_top, SCREEN_SIZE);
		return;
	}
}

void blank_screen(struct vconsole *vc)
{
	if(vc->blanked) {
		return;
	}
	vconsole_save(vc);
	memset_w(vc->vidmem, BLANK_MEM, SCREEN_SIZE / sizeof(short int));
	vc->blanked = 1;
	show_cursor(OFF);
}

void unblank_screen(struct vconsole *vc)
{
	if(!vc->blanked) {
		return;
	}
	vconsole_restore(vc);
	vc->blanked = 0;
	show_cursor(ON);
}

void screen_on(void)
{
	unsigned long int flags;
	struct callout_req creq;

	if(screen_is_off) {
		SAVE_FLAGS(flags); CLI();
		inport_b(INPUT_STAT1);
		inport_b(0x3BA);
		outport_b(ATTR_CONTROLLER, ATTR_CONTROLLER_PAS);
		RESTORE_FLAGS(flags);
	}
	creq.fn = screen_off;
	creq.arg = 0;
	add_callout(&creq, BLANK_INTERVAL);
}

void screen_off(unsigned int arg)
{
	unsigned long int flags;

	screen_is_off = 1;
	SAVE_FLAGS(flags); CLI();
	inport_b(INPUT_STAT1);
	inport_b(0x3BA);
	outport_b(ATTR_CONTROLLER, 0);
	RESTORE_FLAGS(flags);
}

void vconsole_start(struct tty *tty)
{
	struct vconsole *vc;

	vc = (struct vconsole *)tty->driver_data;
	if(!vc->scrlock) {
		return;
	}
	vc->led_status &= ~SCRLBIT;
	vc->scrlock = 0;
	set_leds(vc->led_status);
}

void vconsole_stop(struct tty *tty)
{
	struct vconsole *vc;

	vc = (struct vconsole *)tty->driver_data;
	if(vc->scrlock) {
		return;
	}
	vc->led_status |= SCRLBIT;
	vc->scrlock = 1;
	set_leds(vc->led_status);
}

void vconsole_beep(void)
{
	struct callout_req creq;

	pit_beep_on();
	creq.fn = pit_beep_off;
	creq.arg = 0;
	add_callout(&creq, HZ / 8);
}

void vconsole_deltab(struct tty *tty)
{
	unsigned short int col, n;
	unsigned char count;
	struct vconsole *vc;
	struct cblock *cb;
	unsigned char ch;

	vc = (struct vconsole *)tty->driver_data;
	cb = tty->cooked_q.head;
	col = count = 0;

	while(cb) {
		for(n = 0; n < cb->end_off; n++) {
			if(n >= cb->start_off) {
				ch = cb->data[n];
				if(ch == '\t') {
					while(!vc->tab_stop[++col]);
				} else {
					col++;
					if(ISCNTRL(ch) && !ISSPACE(ch) && tty->termios.c_lflag & ECHOCTL) {
						col++;
					}
				}
				col %= vc->columns;
			}
		}
		cb = cb->next;
	}
	count = vc->x - col;

	while(count--) {
		tty_queue_putchar(tty, &tty->write_q, '\b');
	}
}

void console_flush_log_buf(char *buffer, unsigned int count)
{
	char *b;
	struct tty *tty;

	tty = get_tty(_syscondev);
	b = buffer;

	while(count) {
		if(tty_queue_putchar(tty, &tty->write_q, *b) < 0) {
			tty->output(tty);
			continue;
		}
		count--;
		b++;
	}
	tty->output(tty);
}

void vconsole_init(void)
{
	int n;
	struct tty *tty;

	printk("console");
	if((*(unsigned short int *)0x410 & 0x30) == 0x30) {
		/* monochrome = 0x30 */
		video_base_address = (void *)MONO_ADDR;
		video_port = MONO_6845_ADDR;
		printk("   0x%04X-0x%04X    -    VGA monochrome 80x25", video_port, video_port + 1);
	} else {
		/* color = 0x00 || 0x20 */
		video_base_address = (void *)COLOR_ADDR;
		video_port = COLOR_6845_ADDR;
		printk("   0x%04X-0x%04X    -    VGA color 80x25", video_port, video_port + 1);
	}

	printk(" (%d virtual consoles)\n", NR_VCONSOLES);
	screen_on();

	for(n = 1; n < NR_VCONSOLES + 1; n++) {
		if(!register_tty(MKDEV(VCONSOLES_MAJOR, n))) {
			tty = get_tty(MKDEV(VCONSOLES_MAJOR, n));
			tty->driver_data = (void *)&vc[n];
			tty->stop = vconsole_stop;
			tty->start = vconsole_start;
			tty->deltab = vconsole_deltab;
			tty->reset = vconsole_reset;
			tty->input = do_cook;
			tty->output = vconsole_write;
			vc[n].tty = tty;
			vc[n].vidmem = vc[n].scrbuf;
			memset_w(vc[n].scrbuf, BLANK_MEM, SCREEN_SIZE / sizeof(short int));
			vconsole_reset(tty);
			termios_reset(tty);
			tty_queue_init(tty);
		}
	}
	current_cons = 1;
	vc[current_cons].vidmem = video_base_address;
	vc[current_cons].has_focus = 1;
//	vc[current_cons].count++;	/* XXX */

/*	memset_b(vc[current_cons].vidmem, BLANK_MEM, SCREEN_SIZE); */
	memcpy_b(vcbuf, vc[current_cons].vidmem, SCREEN_SIZE);

	get_curpos(&vc[current_cons]);
	update_curpos(&vc[current_cons]);
	buf_y = vc[current_cons].y;
	buf_top = 0;

	register_device(CHR_DEV, &console_device);
	register_device(CHR_DEV, &tty_device);
	register_console(console_flush_log_buf);
}
