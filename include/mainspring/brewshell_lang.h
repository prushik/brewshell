
#define KEY_DECL_SECTION 11
#define KEY_DECL_PRESERVE 14
#define KEY_DECL_PROTOTYPE 15
#define KEY_SECTION_TEXT 20
#define KEY_SECTION_DATA 21
#define KEY_SECTION_RODATA 22
#define KEY_SECTION_BSS 23
#define KEY_SIZE_BYTE 24
#define KEY_SIZE_WORD 25
#define KEY_SIZE_DWORD 26
#define KEY_SIZE_QWORD 27
#define KEY_REG_RANGE_BEGIN 28
#define KEY_REG_RANGE_END 43
#define RESERVED_KEYWORDS 44
extern const char *reserved[];

#define OP_ADD 0
#define OP_SUB 1
#define OP_MUL 2
#define OP_DIV 3
#define OP_MOD 4
#define OP_LSFT 5
#define OP_RSFT 6
#define OP_SET 7
#define OP_EQU 8
#define OP_NEQ 9
#define OP_AND 10
#define OP_OR 11
#define OP_LAND 12
#define OP_LOR 13
#define OP_LNO 14
#define OP_XOR 15
#define OP_NOT 16
#define OP_GT 17
#define OP_LT 18
#define OP_COM 19
#define OP_DOT 20
#define OP_INC 21
#define OP_DEC 22

#define OPERATORS 23
extern const char *operator[];

#define PUNC_INVD -1
#define PUNC_LABL 0
#define PUNC_ARGS 1
#define PUNC_PP 2
#define PUNC_TERM 3
#define PUNC_ESC 4

#define PUNCTUATORS 5
extern const char *punctuator[];

#define CHAR_TYPE_INV -1
#define CHAR_TYPE_NUM 0
#define CHAR_TYPE_SYM 1
#define CHAR_TYPE_WHT 2
#define CHAR_TYPE_GRP 3
#define CHAR_TYPE_OPR 4
#define CHAR_TYPE_STR 5
#define CHAR_TYPE_SPE 6
#define CHAR_TYPE_PRE 7
#define CHAR_TYPE_NUL 8
#define CHAR_TYPE_COM 16

#define CHARSETS 8
extern const char *char_ini[];
extern const char *char_set[];
extern const char *char_end[];

#define COMMENTS 3
extern const char *comment_ini[];
extern const char *comment_end[];
