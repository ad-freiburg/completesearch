/* ----------------------------------------------------------------------


		< TEXTEC Software Dr. Erwin Stegentritt >


        PROJEKT:        TRAPHO
 
        DATEI:          phonetik.h

        INHALT:         Funktionen fr phonetische Transkrition

        AUTOR:          -es-

        VERSION:        2.1

        STAND:          26.12.98

        MODIFIKATION:   
	                
                        
-----------------------------------------------------------------------*/


#ifndef __PHONETIK_H_
#define __PHONETIK_H_


// #define PHONETIK_STANDALONE // Definieren, wenn die Phonetik in 
                               // einer Standalone-Version compiliert
                               // werden soll. 

// ----------------------------------------------------------------------
//  INCLUDE
// ----------------------------------------------------------------------




// ----------------------------------------------------------------------
//  DEFINE
// ----------------------------------------------------------------------

#define PHONETIK_MAX_WORDLENGTH  256
#define WMAX 40


#define __UNIX__  // __UNIX__ gilt auch fr WindowsXX
// #define __DOS__ // __DOS__ nur fr DOS


// Keytable definition

#ifdef __DOS__

#define SK_NUL                 0x000
#define SK_SOH                 0x001
#define SK_STX                 0x002
#define SK_ETX                 0x003
#define SK_EOT                 0x004
#define SK_ENQ                 0x005
#define SK_ACK                 0x006
#define SK_BEL                 0x007
#define SK_BS                  0x008
#define SK_HT                  0x009
#define SK_LF                  0x00a
#define SK_VT                  0x00b
#define SK_FF                  0x00c
#define SK_CR                  0x00d
#define SK_SO                  0x00e
#define SK_SI                  0x00f
#define SK_DLE                 0x010
#define SK_DC1                 0x011
#define SK_DC2                 0x012
#define SK_DC3                 0x013
#define SK_DC4                 0x014
#define SK_NAC                 0x015
#define SK_SYN                 0x016
#define SK_ETB                 0x017
#define SK_CAN                 0x018
#define SK_EM                  0x019
#define SK_SUB                 0x01a
#define SK_ESC                 0x01b
#define SK_FS                  0x01c
#define SK_GS                  0x01d
#define SK_RS                  0x01e
#define SK_US                  0x01f
#define SK_space               0x020
#define SK_exclam              0x021
#define SK_quotedbl            0x022
#define SK_numbersign          0x023
#define SK_dollar              0x024
#define SK_percent             0x025
#define SK_ampersand           0x026
#define SK_apostrophe          0x027
#define SK_parenleft           0x028
#define SK_parenright          0x029
#define SK_asterisk            0x02a
#define SK_plus                0x02b
#define SK_comma               0x02c
#define SK_minus               0x02d
#define SK_period              0x02e
#define SK_slash               0x02f
#define SK_0                   0x030
#define SK_1                   0x031
#define SK_2                   0x032
#define SK_3                   0x033
#define SK_4                   0x034
#define SK_5                   0x035
#define SK_6                   0x036
#define SK_7                   0x037
#define SK_8                   0x038
#define SK_9                   0x039
#define SK_colon               0x03a
#define SK_semicolon           0x03b
#define SK_less                0x03c
#define SK_equal               0x03d
#define SK_greater             0x03e
#define SK_question            0x03f
#define SK_at                  0x040
#define SK_A                   0x041
#define SK_B                   0x042
#define SK_C                   0x043
#define SK_D                   0x044
#define SK_E                   0x045
#define SK_F                   0x046
#define SK_G                   0x047
#define SK_H                   0x048
#define SK_I                   0x049
#define SK_J                   0x04a
#define SK_K                   0x04b
#define SK_L                   0x04c
#define SK_M                   0x04d
#define SK_N                   0x04e
#define SK_O                   0x04f
#define SK_P                   0x050
#define SK_Q                   0x051
#define SK_R                   0x052
#define SK_S                   0x053
#define SK_T                   0x054
#define SK_U                   0x055
#define SK_V                   0x056
#define SK_W                   0x057
#define SK_X                   0x058
#define SK_Y                   0x059
#define SK_Z                   0x05a
#define SK_bracketleft         0x05b
#define SK_backslash           0x05c
#define SK_bracketright        0x05d
#define SK_asciicircum         0x05e
#define SK_underscore          0x05f
#define SK_grave               0x060
#define SK_a                   0x061
#define SK_b                   0x062
#define SK_c                   0x063
#define SK_d                   0x064
#define SK_e                   0x065
#define SK_f                   0x066
#define SK_g                   0x067
#define SK_h                   0x068
#define SK_i                   0x069
#define SK_j                   0x06a
#define SK_k                   0x06b
#define SK_l                   0x06c
#define SK_m                   0x06d
#define SK_n                   0x06e
#define SK_o                   0x06f
#define SK_p                   0x070
#define SK_q                   0x071
#define SK_r                   0x072
#define SK_s                   0x073
#define SK_t                   0x074
#define SK_u                   0x075
#define SK_v                   0x076
#define SK_w                   0x077
#define SK_x                   0x078
#define SK_y                   0x079
#define SK_z                   0x07a
#define SK_braceleft           0x07b
#define SK_bar                 0x07c
#define SK_braceright          0x07d
#define SK_asciitilde          0x07e
#define SK_07f                 0x07f
#define SK_Ccedilla            0x080
#define SK_udiaeresis          0x081
#define SK_eacute              0x082
#define SK_acircumflex         0x083
#define SK_adiaeresis          0x084
#define SK_agrave              0x085
#define SK_aring               0x086
#define SK_ccedilla            0x087
#define SK_ecircumflex         0x088
#define SK_ediaeresis          0x089
#define SK_egrave              0x08a
#define SK_idiaeresis          0x08b
#define SK_icircumflex         0x08c
#define SK_igrave              0x08d
#define SK_Adiaeresis          0x08e
#define SK_Aring               0x08f
#define SK_Eacute              0x090
#define SK_ae                  0x091
#define SK_AE                  0x092
#define SK_ocircumflex         0x093
#define SK_odiaeresis          0x094
#define SK_ograve              0x095
#define SK_ucircumflex         0x096
#define SK_ugrave              0x097
#define SK_ydiaeresis          0x098
#define SK_Odiaeresis          0x099
#define SK_Udiaeresis          0x09a
#define SK_cent                0x09b
#define SK_sterling            0x09c
#define SK_yen                 0x09d
#define SK_pesetas             0x09e
#define SK_florin              0x09f
#define SK_aacute              0x0a0
#define SK_iacute              0x0a1
#define SK_oacute              0x0a2
#define SK_uacute              0x0a3
#define SK_ntilde              0x0a4
#define SK_Ntilde              0x0a5
#define SK_ordfeminine         0x0a6
#define SK_masculine           0x0a7
#define SK_questiondown        0x0a8
#define SK_notsignleft         0x0a9
#define SK_notsignright        0x0aa
#define SK_onehalf             0x0ab
#define SK_onequarter          0x0ac
#define SK_exclamdown          0x0ad
#define SK_guillemotleft       0x0ae
#define SK_guillemotright      0x0af
#define SK_graphic_0b0         0x0b0
#define SK_graphic_0b1         0x0b1
#define SK_graphic_0b2         0x0b2
#define SK_graphic_0b3         0x0b3
#define SK_graphic_0b4         0x0b4
#define SK_graphic_0b5         0x0b5
#define SK_graphic_0b6         0x0b6
#define SK_graphic_0b7         0x0b7
#define SK_graphic_0b8         0x0b8
#define SK_graphic_0b9         0x0b9
#define SK_graphic_0ba         0x0ba
#define SK_graphic_0bb         0x0bb
#define SK_graphic_0bc         0x0bc
#define SK_graphic_0bd         0x0bd
#define SK_graphic_0be         0x0be
#define SK_graphic_0bf         0x0bf
#define SK_graphic_0c0         0x0c0
#define SK_graphic_0c1         0x0c1
#define SK_graphic_0c2         0x0c2
#define SK_graphic_0c3         0x0c3
#define SK_graphic_0c4         0x0c4
#define SK_graphic_0c5         0x0c5
#define SK_graphic_0c6         0x0c6
#define SK_graphic_0c7         0x0c7
#define SK_graphic_0c8         0x0c8
#define SK_graphic_0c9         0x0c9
#define SK_graphic_0ca         0x0ca
#define SK_graphic_0cb         0x0cb
#define SK_graphic_0cc         0x0cc
#define SK_graphic_0cd         0x0cd
#define SK_graphic_0ce         0x0ce
#define SK_graphic_0cf         0x0cf
#define SK_graphic_0d0         0x0d0
#define SK_graphic_0d1         0x0d1
#define SK_graphic_0d2         0x0d2
#define SK_graphic_0d3         0x0d3
#define SK_graphic_0d4         0x0d4
#define SK_graphic_0d5         0x0d5
#define SK_graphic_0d6         0x0d6
#define SK_graphic_0d7         0x0d7
#define SK_graphic_0d8         0x0d8
#define SK_graphic_0d9         0x0d9
#define SK_graphic_0da         0x0da
#define SK_graphic_0db         0x0db
#define SK_graphic_0dc         0x0dc
#define SK_graphic_0dd         0x0dd
#define SK_graphic_0de         0x0de
#define SK_graphic_0df         0x0df
#define SK_alpha               0x0e0
#define SK_beta                0x0e1
#define SK_GAMMA               0x0e2
#define SK_pi                  0x0e3
#define SK_SIGMA               0x0e4
#define SK_sigma               0x0e5
#define SK_mu                  0x0e6
#define SK_tau                 0x0e7
#define SK_PHI                 0x0e8
#define SK_THETA               0x0e9
#define SK_OMEGA               0x0ea
#define SK_delta               0x0eb
#define SK_infinity            0x0ec
#define SK_0ed                 0x0ed
#define SK_epsilon             0x0ee
#define SK_intersection        0x0ef
#define SK_identical           0x0f0
#define SK_plusminus           0x0f1
#define SK_greaterthanequal    0x0f2
#define SK_lessthanequal       0x0f3
#define SK_topintegral         0x0f4
#define SK_botintergral        0x0f5
#define SK_division            0x0f6
#define SK_approximate         0x0f7
#define SK_degree              0x0f8
#define SK_bigperiodcentered   0x0f9
#define SK_smallperiodcentered 0x0fa
#define SK_squareroot          0x0fb
#define SK_nsuperior           0x0fc
#define SK_twosuperior         0x0fd
#define SK_filledsquare        0x0fe
#define SK_0ff                 0x0ff

/* Caracteres non definis dans l'ASCII PC */
#define SK_Agrave      SK_A
#define SK_Aacute      SK_A
#define SK_Acircumflex SK_A
#undef  SK_Atilde
#define SK_Egrave      SK_E
#define SK_Ecircumflex SK_E
#define SK_Ediaeresis  SK_E
#define SK_Igrave      SK_I
#define SK_Iacute      SK_I
#define SK_Icircumflex SK_I
#define SK_Idiaeresis  SK_I
#define SK_Ograve      SK_O
#define SK_Oacute      SK_O
#define SK_Ocircumflex SK_O
#undef  SK_0tilde
#undef  SK_Ooblique
#define SK_Ugrave      SK_U
#define SK_Uacute      SK_U
#define SK_Ucircumflex SK_U
#undef  SK_Yacute
#define SK_Ydiaeresis  SK_Y
#undef  SK_atilde
#undef  SK_otilde
#undef  SK_oslash
#undef  SK_yacute

#endif

#ifdef __UNIX__


#define SK_A                   0x041
#define SK_B                   0x042
#define SK_C                   0x043
#define SK_D                   0x044
#define SK_E                   0x045
#define SK_F                   0x046
#define SK_G                   0x047
#define SK_H                   0x048
#define SK_I                   0x049
#define SK_J                   0x04a
#define SK_K                   0x04b
#define SK_L                   0x04c
#define SK_M                   0x04d
#define SK_N                   0x04e
#define SK_O                   0x04f
#define SK_P                   0x050
#define SK_Q                   0x051
#define SK_R                   0x052
#define SK_S                   0x053
#define SK_T                   0x054
#define SK_U                   0x055
#define SK_V                   0x056
#define SK_W                   0x057
#define SK_X                   0x058
#define SK_Y                   0x059
#define SK_Z                   0x05a

#define SK_a                   0x061
#define SK_b                   0x062
#define SK_c                   0x063
#define SK_d                   0x064
#define SK_e                   0x065
#define SK_f                   0x066
#define SK_g                   0x067
#define SK_h                   0x068
#define SK_i                   0x069
#define SK_j                   0x06a
#define SK_k                   0x06b
#define SK_l                   0x06c
#define SK_m                   0x06d
#define SK_n                   0x06e
#define SK_o                   0x06f
#define SK_p                   0x070
#define SK_q                   0x071
#define SK_r                   0x072
#define SK_s                   0x073
#define SK_t                   0x074
#define SK_u                   0x075
#define SK_v                   0x076
#define SK_w                   0x077
#define SK_x                   0x078
#define SK_y                   0x079
#define SK_z                   0x07a
#define SK_Ccedilla            0x0c7 // diff to DOS
#define SK_udiaeresis          0x0fc //
#define SK_eacute              0x0e9 //
#define SK_acircumflex         0x0e2 //
#define SK_adiaeresis          0x0e4 //
#define SK_agrave              0x0e0 //
#define SK_aring               0x0e5 //
#define SK_ccedilla            0x0e7 //
#define SK_ecircumflex         0x0ea //
#define SK_ediaeresis          0x0eb //
#define SK_egrave              0x0e8 //
#define SK_idiaeresis          0x0ef //
#define SK_icircumflex         0x0ee //
#define SK_igrave              0x0ec //
#define SK_Adiaeresis          0x0c4 //
#define SK_Aring               0x0c5 //
#define SK_Eacute              0x0c9 //
#define SK_ocircumflex         0x0f4 //
#define SK_odiaeresis          0x0f6 //
#define SK_ograve              0x0f2 //
#define SK_ucircumflex         0x0fb //
#define SK_ugrave              0x0f9 //
#define SK_Odiaeresis          0x0d6 //
#define SK_Udiaeresis          0x0dc //
#define SK_aacute              0x0e1 //
#define SK_iacute              0x0ed //
#define SK_oacute              0x0f3 //
#define SK_uacute              0x0fa //
#define SK_ntilde              0x0f1 //
#define SK_Ntilde              0x0d1 //
#define SK_beta                0x0df //

/* Caracteres non definis dans l'ASCII PC */
#define SK_Agrave      SK_A
#define SK_Aacute      SK_A
#define SK_Acircumflex SK_A
#undef  SK_Atilde
#define SK_Egrave      SK_E
#define SK_Ecircumflex SK_E
#define SK_Ediaeresis  SK_E
#define SK_Igrave      SK_I
#define SK_Iacute      SK_I
#define SK_Icircumflex SK_I
#define SK_Idiaeresis  SK_I
#define SK_Ograve      SK_O
#define SK_Oacute      SK_O
#define SK_Ocircumflex SK_O
#undef  SK_0tilde
#undef  SK_Ooblique
#define SK_Ugrave      SK_U
#define SK_Uacute      SK_U
#define SK_Ucircumflex SK_U
#undef  SK_Yacute
#define SK_Ydiaeresis  SK_Y
#undef  SK_atilde
#undef  SK_otilde
#undef  SK_oslash
#undef  SK_yacute

#endif


// ----------------------------------------------------------------------
//  FUNCTION PROTOTYPES
// ----------------------------------------------------------------------

char* phonGerman_with(unsigned char *);
char *phonEnglish_with(unsigned char *);
char *phonFrench_with(unsigned char *);


// #ifdef PHONETIK_STANDALONE
void lowerstring(char *str);
// #endif
int leven(char *w1, char *w2);

#endif
