#ifndef KEY_H
#define KEY_H

#ifdef KEY_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

#define K_ESC       1
#define K_1         2
#define K_2         3
#define K_3         4
#define K_4         5
#define K_5         6
#define K_6         7
#define K_7         8
#define K_8         9
#define K_9         10
#define K_0         11
#define K_MINUS     12
#define K_EQUALS    13
#define K_BACK		14
#define K_TAB		15
#define K_Q		    16
#define K_W		    17
#define K_E		    18
#define K_R		    19
#define K_T		    20
#define K_Y		    21
#define K_U		    22
#define K_I		    23
#define K_O		    24
#define K_P		    25
#define K_LBRACKET	26
#define K_RBRACKET	27
#define K_ENTER		28
#define K_CTRL		29
#define K_A		    30
#define K_S		    31
#define K_D		    32
#define K_F		    33
#define K_G		    34
#define K_H		    35
#define K_J		    36
#define K_K		    37
#define K_L		    38
#define K_SEMI		39
#define K_APOS		40
#define K_LSHIFT	42
#define K_BACKSLASH	43
#define K_Z		    44
#define K_X		    45
#define K_C		    46
#define K_V		    47
#define K_B		    48
#define K_N		    49
#define K_M		    50
#define K_COMMA		51
#define K_PERIOD	52	
#define K_SLASH		53
#define K_RSHIFT	54	
#define K_PRINT		55
#define K_ALT		56
#define K_SPACE		57
#define K_CAPS		58
#define K_F1		59
#define K_F2		60
#define K_F3		61
#define K_F4		62
#define K_F5		63
#define K_F6		64
#define K_F7		65
#define K_F8		66
#define K_F9		67
#define FK_10		68
#define K_F11       87
#define K_F12       88
#define K_NUMLOCK   69
#define K_SCROLL	70	
#define K_HOME		71
#define K_UP		72
#define K_PGUP		73
#define K_NUMMINUS	74
#define K_LEFT		75
#define K_CENTER	76
#define K_RIGHT		77
#define K_NUMPLUS	78
#define K_END		79
#define K_DOWN		80
#define K_PGDOWN	81	
#define K_INS		82
#define K_DEL       83

EXTERN void Key_initialize(void);
EXTERN void Key_terminate(void);
EXTERN void Key_update(void);
EXTERN unsigned char Key_down(unsigned char key);
EXTERN unsigned char Key_pressed(unsigned char key);
EXTERN unsigned char Key_released(unsigned char key);
EXTERN unsigned char Key_up(unsigned char key);

#undef KEY_IMPORT
#undef EXTERN

#endif