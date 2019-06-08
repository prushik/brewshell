#define RESERVED_KEYWORDS 8
extern const char *reserved[];

#define OP_ADD 0
#define OP_SUB 1
#define OP_MUL 2
#define OP_DIV 3
#define OP_MOD 4
#define OP_LSFT 5
#define OP_RSFT 6
#define OP_DOT 7

#define OPERATORS 8
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

#define COMMENTS 4
extern const char *comment_ini[];
extern const char *comment_end[];
