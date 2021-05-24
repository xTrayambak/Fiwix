/*
 * fiwix/drivers/char/defkeymap.c
 *
 * Copyright 2018-2021, Jordi Sanfeliu. All rights reserved.
 * Distributed under the terms of the Fiwix License.
 */

#include <fiwix/keyboard.h>
#include <fiwix/string.h>

#define BS	127	/* backspace */

__key_t keymap[NR_MODIFIERS * NR_SCODES] = {
/*
 * Standard US keyboard (default keymap) with 16 modifiers
 *																		Shift
 *										Shift				Shift		Shift	AltGr	AltGr
 * SCAN						Shift		Shift	AltGr	AltGr		Shift	AltGr	AltGr	Ctrl	Ctrl	Ctrl	Ctrl
 * CODE  KEY		Base	Shift	AltGr	AltGr	Ctrl	Ctrl	Ctrl	Ctrl	Alt	Alt	Alt	Alt	Alt	Alt	Alt	Alt
 * ==================================================================================================================================================== */
/*  00 - NULL	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
/*  01 - ESC	*/	C('['),	C('['),	NULL,	NULL,	NULL,	0,	0,	0,	A('['),	0,	0,	0,	0,	0,	0,	0,
/*  02 - 1	*/	'1',	'!',	NULL,	NULL,	NULL,	0,	0,	0,	A('1'),	0,	0,	0,	0,	0,	0,	0,
/*  03 - 2	*/	'2',	'@',	'@',	NULL,	NULL,	0,	0,	0,	A('2'),	0,	0,	0,	0,	0,	0,	0,
/*  04 - 3	*/	'3',	'#',	NULL,	NULL,	C('['),	0,	0,	0,	A('3'),	0,	0,	0,	0,	0,	0,	0,
/*  05 - 4	*/	'4',	'$',	'$',	NULL,	C('\\'),0,	0,	0,	A('4'),	0,	0,	0,	0,	0,	0,	0,
/*  06 - 5	*/	'5',	'%',	NULL,	NULL,	C(']'),	0,	0,	0,	A('5'),	0,	0,	0,	0,	0,	0,	0,
/*  07 - 6	*/	'6',	'^',	NULL,	NULL,	C('^'),	0,	0,	0,	A('6'),	0,	0,	0,	0,	0,	0,	0,
/*  08 - 7	*/	'7',	'&',	'{',	NULL,	C('_'),	0,	0,	0,	A('7'),	0,	0,	0,	0,	0,	0,	0,
/*  09 - 8	*/	'8',	'*',	'[',	NULL,	BS,	0,	0,	0,	A('8'),	0,	0,	0,	0,	0,	0,	0,
/*  10 - 9	*/	'9',	'(',	']',	NULL,	NULL,	0,	0,	0,	A('9'),	0,	0,	0,	0,	0,	0,	0,
/*  11 - 0	*/	'0',	')',	'}',	NULL,	NULL,	0,	0,	0,	A('0'),	0,	0,	0,	0,	0,	0,	0,
/*  12 - -_	*/	'-',	'_',	'\\',	NULL,	C('_'),	0,	0,	0,	A('-'),	0,	0,	0,	0,	0,	0,	0,
/*  13 - =+	*/	'=',	'+',	NULL,	NULL,	NULL,	0,	0,	0,	A('='),	0,	0,	0,	0,	0,	0,	0,
/*  14 - BS	*/	BS,	BS,	NULL,	NULL,	NULL,	0,	0,	0,	A(BS),	0,	0,	0,	0,	0,	0,	0,
/*  15 - TAB	*/	'\t',	'\t',	NULL,	NULL,	NULL,	0,	0,	0,	A('\t'),0,	0,	0,	0,	0,	0,	0,
/*  16 - q	*/	L('q'),	L('Q'),	L('q'),	L('Q'),	C('Q'),	0,	0,	0,	A('q'),	0,	0,	0,	0,	0,	0,	0,
/*  17 - w	*/	L('w'),	L('W'),	L('w'),	L('W'),	C('W'),	0,	0,	0,	A('w'),	0,	0,	0,	0,	0,	0,	0,
/*  18 - e	*/	L('e'),	L('E'),	L('e'),	L('E'),	C('E'),	0,	0,	0,	A('e'),	0,	0,	0,	0,	0,	0,	0,
/*  19 - r	*/	L('r'),	L('R'),	L('r'),	L('R'),	C('R'),	0,	0,	0,	A('r'),	0,	0,	0,	0,	0,	0,	0,
/*  20 - t	*/	L('t'),	L('T'),	L('t'),	L('T'),	C('T'),	0,	0,	0,	A('t'),	0,	0,	0,	0,	0,	0,	0,
/*  21 - y	*/	L('y'),	L('Y'),	L('y'),	L('Y'),	C('Y'),	0,	0,	0,	A('y'),	0,	0,	0,	0,	0,	0,	0,
/*  22 - u	*/	L('u'),	L('U'),	L('u'),	L('U'),	C('U'),	0,	0,	0,	A('u'),	0,	0,	0,	0,	0,	0,	0,
/*  23 - i	*/	L('i'),	L('I'),	L('i'),	L('I'),	C('I'),	0,	0,	0,	A('i'),	0,	0,	0,	0,	0,	0,	0,
/*  24 - o	*/	L('o'),	L('O'),	L('o'),	L('O'),	C('O'),	0,	0,	0,	A('o'),	0,	0,	0,	0,	0,	0,	0,
/*  25 - p	*/	L('p'),	L('P'),	L('p'),	L('P'),	C('P'),	0,	0,	0,	A('p'),	0,	0,	0,	0,	0,	0,	0,
/*  26 - [{	*/	'[',	'{',	NULL,	NULL,	C('['),	0,	0,	0,	A('['),	0,	0,	0,	0,	0,	0,	0,
/*  27 - ]}	*/	']',	'}',	'~',	NULL,	C(']'),	0,	0,	0,	A(']'),	0,	0,	0,	0,	0,	0,	0,
/*  28 - CR	*/	CR,	CR,	CR,	CR,	CR,	0,	0,	0,	A(CR),	0,	0,	0,	0,	0,	0,	0,
/*  29 - LCTRL	*/	LCTRL,	LCTRL,	LCTRL,	LCTRL,	LCTRL,	0,	0,	0,	LCTRL,	0,	0,	0,	0,	0,	0,	0,
/*  30 - a	*/	L('a'),	L('A'),	L('a'),	L('A'),	C('A'),	0,	0,	0,	A('a'),	0,	0,	0,	0,	0,	0,	0,
/*  31 - s	*/	L('s'),	L('S'),	L('s'),	L('S'),	C('S'),	0,	0,	0,	A('s'),	0,	0,	0,	0,	0,	0,	0,
/*  32 - d	*/	L('d'),	L('D'),	L('d'),	L('D'),	C('D'),	0,	0,	0,	A('d'),	0,	0,	0,	0,	0,	0,	0,
/*  33 - f	*/	L('f'),	L('F'),	L('f'),	L('F'),	C('F'),	0,	0,	0,	A('f'),	0,	0,	0,	0,	0,	0,	0,
/*  34 - g	*/	L('g'),	L('G'),	L('g'),	L('G'),	C('G'),	0,	0,	0,	A('g'),	0,	0,	0,	0,	0,	0,	0,
/*  35 - h	*/	L('h'),	L('H'),	L('h'),	L('H'),	C('H'),	0,	0,	0,	A('h'),	0,	0,	0,	0,	0,	0,	0,
/*  36 - j	*/	L('j'),	L('J'),	L('j'),	L('J'),	C('J'),	0,	0,	0,	A('j'),	0,	0,	0,	0,	0,	0,	0,
/*  37 - k	*/	L('k'),	L('K'),	L('k'),	L('K'),	C('K'),	0,	0,	0,	A('k'),	0,	0,	0,	0,	0,	0,	0,
/*  38 - l	*/	L('l'),	L('L'),	L('l'),	L('L'),	C('L'),	0,	0,	0,	A('l'),	0,	0,	0,	0,	0,	0,	0,
/*  39 - ;:	*/	';',	':',	NULL,	NULL,	NULL,	0,	0,	0,	A(';'),	0,	0,	0,	0,	0,	0,	0,
/*  40 - '"	*/	'\'',	'"',	NULL,	NULL,	C('G'),	0,	0,	0,	A('\''),0,	0,	0,	0,	0,	0,	0,
/*  41 - `~	*/	'`',	'~',	NULL,	NULL,	NULL,	0,	0,	0,	A('`'),	0,	0,	0,	0,	0,	0,	0,
/*  42 - LSHF	*/	LSHIFT,	LSHIFT,	LSHIFT,	LSHIFT,	LSHIFT,	0,	0,	0,	LSHIFT,	0,	0,	0,	0,	0,	0,	0,
/*  43 - \|	*/	'\\',	'|',	NULL,	NULL,	C('\\'),0,	0,	0,	A('\\'),0,	0,	0,	0,	0,	0,	0,
/*  44 - z	*/	L('z'),	L('Z'),	L('z'),	L('Z'),	C('Z'),	0,	0,	0,	A('z'),	0,	0,	0,	0,	0,	0,	0,
/*  45 - x	*/	L('x'),	L('X'),	L('x'),	L('X'),	C('X'),	0,	0,	0,	A('x'),	0,	0,	0,	0,	0,	0,	0,
/*  46 - c	*/	L('c'),	L('C'),	L('c'),	L('C'),	C('C'),	0,	0,	0,	A('c'),	0,	0,	0,	0,	0,	0,	0,
/*  47 - v	*/	L('v'),	L('V'),	L('v'),	L('V'),	C('V'),	0,	0,	0,	A('v'),	0,	0,	0,	0,	0,	0,	0,
/*  48 - b	*/	L('b'),	L('B'),	L('b'),	L('B'),	C('B'),	0,	0,	0,	A('b'),	0,	0,	0,	0,	0,	0,	0,
/*  49 - n	*/	L('n'),	L('N'),	L('n'),	L('N'),	C('N'),	0,	0,	0,	A('n'),	0,	0,	0,	0,	0,	0,	0,
/*  50 - m	*/	L('m'),	L('M'),	L('m'),	L('M'),	C('M'),	0,	0,	0,	A('m'),	0,	0,	0,	0,	0,	0,	0,
/*  51 - ,<	*/	',',	'<',	NULL,	NULL,	NULL,	0,	0,	0,	A(','),	0,	0,	0,	0,	0,	0,	0,
/*  52 - .>	*/	'.',	'>',	NULL,	NULL,	NULL,	0,	0,	0,	A('.'),	0,	0,	0,	0,	0,	0,	0,
/*  53 - /?	*/	SLASH,	'?',	NULL,	NULL,	BS,	0,	0,	0,	A('/'),	0,	0,	0,	0,	0,	0,	0,
/*  54 - RSHF	*/	RSHIFT,	RSHIFT,	RSHIFT,	RSHIFT,	RSHIFT,	0,	0,	0,	RSHIFT,	0,	0,	0,	0,	0,	0,	0,
/*  55 - *	*/	ASTSK,	ASTSK,	ASTSK,	ASTSK,	ASTSK,	0,	0,	0,	ASTSK,	0,	0,	0,	0,	0,	0,	0,
/*  56 - ALT	*/	ALT,	ALT,	ALT,	ALT,	ALT,	0,	0,	0,	ALT,	0,	0,	0,	0,	0,	0,	0,
/*  57 - SPC	*/	' ',	' ',	NULL,	NULL,	NULL,	0,	0,	0,	A(' '),	0,	0,	0,	0,	0,	0,	0,
/*  58 - CAPS	*/	CAPS,	CAPS,	CAPS,	CAPS,	CAPS,	0,	0,	0,	CAPS,	0,	0,	0,	0,	0,	0,	0,
/*  59 - F1	*/	F1,	SF1,	NULL,	NULL,	F1,	0,	0,	0,	AF1,	0,	0,	0,	0,	0,	0,	0,
/*  60 - F2	*/	F2,	SF2,	NULL,	NULL,	F2,	0,	0,	0,	AF2,	0,	0,	0,	0,	0,	0,	0,
/*  61 - F3	*/	F3,	SF3,	NULL,	NULL,	F3,	0,	0,	0,	AF3,	0,	0,	0,	0,	0,	0,	0,
/*  62 - F4	*/	F4,	SF4,	NULL,	NULL,	F4,	0,	0,	0,	AF4,	0,	0,	0,	0,	0,	0,	0,
/*  63 - F5	*/	F5,	SF5,	NULL,	NULL,	F5,	0,	0,	0,	AF5,	0,	0,	0,	0,	0,	0,	0,
/*  64 - F6	*/	F6,	SF6,	NULL,	NULL,	F6,	0,	0,	0,	AF6,	0,	0,	0,	0,	0,	0,	0,
/*  65 - F7	*/	F7,	SF7,	NULL,	NULL,	F7,	0,	0,	0,	AF7,	0,	0,	0,	0,	0,	0,	0,
/*  66 - F8	*/	F8,	SF8,	NULL,	NULL,	F8,	0,	0,	0,	AF8,	0,	0,	0,	0,	0,	0,	0,
/*  67 - F9	*/	F9,	SF9,	NULL,	NULL,	F9,	0,	0,	0,	AF9,	0,	0,	0,	0,	0,	0,	0,
/*  68 - F10	*/	F10,	SF10,	NULL,	NULL,	F10,	0,	0,	0,	AF10,	0,	0,	0,	0,	0,	0,	0,
/*  69 - NUMS	*/	NUMS,	NUMS,	NUMS,	NUMS,	NUMS,	0,	0,	0,	NUMS,	0,	0,	0,	0,	0,	0,	0,
/*  70 - SCRL	*/	SCRL,	SCRL3,	SCRL2,	NULL,	SCRL4,	0,	0,	0,	SCRL,	0,	0,	0,	0,	0,	0,	0,
/*  71 - HOME/7	*/	HOME,	HOME,	HOME,	HOME,	HOME,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  72 - UP  /8	*/	UP,	UP,	UP,	UP,	UP,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  73 - PGUP/9	*/	PGUP,	PGUP,	PGUP,	PGUP,	PGUP,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  74 - MINUS	*/	MINUS,	MINUS,	MINUS,	MINUS,	MINUS,	0,	0,	0,	MINUS,	0,	0,	0,	0,	0,	0,	0,
/*  75 - LEFT/4	*/	LEFT,	LEFT,	LEFT,	LEFT,	LEFT,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  76 - MID /5	*/	MID,	MID,	MID,	MID,	MID,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  77 - RIGH/6	*/	RIGHT,	RIGHT,	RIGHT,	RIGHT,	RIGHT,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  78 - PLUS	*/	PLUS,	PLUS,	PLUS,	PLUS,	PLUS,	0,	0,	0,	PLUS,	0,	0,	0,	0,	0,	0,	0,
/*  79 - END /1	*/	END,	END,	END,	END,	END,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  80 - DOWN/2	*/	DOWN,	DOWN,	DOWN,	DOWN,	DOWN,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  81 - PGDN/3	*/	PGDN,	PGDN,	PGDN,	PGDN,	PGDN,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  82 - INS /0	*/	INS,	INS,	INS,	INS,	INS,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  83 - DEL /.	*/	DEL,	DEL,	DEL,	DEL,	DEL,	0,	0,	0,	DEL,	0,	0,	0,	0,	0,	0,	0,
/*  84 - SYSRQ	*/	SYSRQ,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	SYSRQ,	0,	0,	0,	0,	0,	0,	0,
/*  85 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  86 - <>	*/	'<',	'>',	'|',	NULL,	NULL,	0,	0,	0,	A('<'),	0,	0,	0,	0,	0,	0,	0,
/*  87 - F11	*/	SF1,	SF1,	NULL,	NULL,	F11,	0,	0,	0,	AF11,	0,	0,	0,	0,	0,	0,	0,
/*  88 - F12	*/	SF2,	SF2,	NULL,	NULL,	F12,	0,	0,	0,	AF12,	0,	0,	0,	0,	0,	0,	0,
/*  89 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  90 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  91 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  92 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  93 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  94 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  95 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/*  96 - E0ENTER*/	E0ENTER,E0ENTER,E0ENTER,E0ENTER,E0ENTER,0,	0,	0,	E0ENTER,0,	0,	0,	0,	0,	0,	0,
/*  97 - RCTRL	*/	RCTRL,	RCTRL,	RCTRL,	RCTRL,	RCTRL,	0,	0,	0,	RCTRL,	0,	0,	0,	0,	0,	0,	0,
/*  98 - E0SLASH*/	E0SLASH,E0SLASH,E0SLASH,E0SLASH,E0SLASH,0,	0,	0,	E0SLASH,0,	0,	0,	0,	0,	0,	0,
/*  99 -	*/	NULL,	NULL,	NULL,	NULL,	C('\\'),0,	0,	0,	C('\\'),0,	0,	0,	0,	0,	0,	0,
/* 100 - ALTGR	*/	ALTGR,	ALTGR,	ALTGR,	ALTGR,	ALTGR,	0,	0,	0,	ALTGR,	0,	0,	0,	0,	0,	0,	0,
/* 101 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 102 - E0HOME	*/	E0HOME,	E0HOME,	E0HOME,	E0HOME,	E0HOME,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 103 - E0UP  	*/	E0UP,	E0UP,	E0UP,	E0UP,	E0UP,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 104 - E0PGUP	*/	E0PGUP,	E0PGUP,	E0PGUP,	E0PGUP,	E0PGUP,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 105 - E0LEFT	*/	E0LEFT,	E0LEFT,	E0LEFT,	E0LEFT,	E0LEFT,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 106 - E0RIGHT*/	E0RIGHT,E0RIGHT,E0RIGHT,E0RIGHT,E0RIGHT,0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 107 - E0END	*/	E0END,	E0END,	E0END,	E0END,	E0END,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 108 - E0DOWN	*/	E0DOWN,	E0DOWN,	E0DOWN,	E0DOWN,	E0DOWN,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 109 - E0PGDN	*/	E0PGDN,	E0PGDN,	E0PGDN,	E0PGDN,	E0PGDN,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 110 - E0INS	*/	E0INS,	E0INS,	E0INS,	E0INS,	E0INS,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 111 - E0DEL	*/	E0DEL,	E0DEL,	E0DEL,	E0DEL,	E0DEL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 112 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 113 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 114 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 115 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 116 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 117 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 118 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 119 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 120 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 121 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 122 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 123 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 124 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 125 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 126 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
/* 127 -	*/	NULL,	NULL,	NULL,	NULL,	NULL,	0,	0,	0,	NULL,	0,	0,	0,	0,	0,	0,	0,
};
