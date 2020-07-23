#ifndef _KEYMAP_H
#define _KEYMAP_H

static unsigned char ps2_keyboard_map[128] = {
	0,
	27, // escape
	49, // 1
	50, // 2
	51, // 3
	52, // 4
	53, // 5
	54, // 6
	55, // 7
	56, // 8
	57, // 9
	48, // 0
	173, //-
	61, //=
	8, // backspace
	9, // tab
	81, // q
	87, // w
	69, // e
	82, // r
	84, // t
	89, // y
	85, // u
	73, // i
	79, // o
	80, // p
	219, // [
	221, // ]
	13, // enter
	17, // left control
	65, // a
	83, // s
	68, // d
	70, // f
	71, // g
	72, // h
	74, // j
	75, // k
	76, // l
	59, // ;
	222, // '
	192, // `
	16, // left shift
	220, // backslash
	90, // z
	88, // x
	67, // c
	86, // v
	66, // b
	78, // n
	77, // m
	188, // ,
	190, // .
	191, // /
	16, // right shift
	106, // numpad *
	18, // left alt
	32, // space
	20, // capslock
	112, // f1
	113, // f2
	114, // f3
	115, // f4
	116, // f5
	117, // f6
	118, // f7
	119, // f8
	120, // f9
	121, // f10
	144, // numlock
	145, // scrolllock
	103, // numpad 7
	104, // numpad 8
	105, // numpad 9
	109, // numpad -
	100, // numpad 4
	101, // numpad 5
	102, // numpad 6
	107, // numpad +
	97, // numpad 1
	98, // numpad 2
	99, // numpad 3
	96, // numpad 0
	110, // numpad 110
	0,	 0, 0,
	122, // f11
	123, // f12
};

#endif
