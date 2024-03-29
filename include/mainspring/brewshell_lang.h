#define KEY_ACTIONS_START 0
#define KEY_ACTIONS_END 10
#define KEY_ACTION_BREW 0
#define KEY_ACTION_OPEN 1
#define KEY_ACTION_SAVE 2
#define KEY_ACTION_EXPORT 3
#define KEY_ACTION_ADD 4
#define KEY_ACTION_RM 5
#define KEY_ACTION_PRINT 6
#define KEY_ACTION_SET 7
#define KEY_ACTION_QUIT 8
#define KEY_ACTION_SCALE 9
#define KEY_ACTION_LIST 10

#define KEY_PARAMETERS_START 11
#define KEY_PARAMETERS_END 23
#define KEY_STRING_PARAMETERS_START 11
#define KEY_STRING_PARAMETERS_END 12
#define KEY_FLOAT_PARAMETERS_START 13
#define KEY_FLOAT_PARAMETERS_END 20
#define KEY_ARRAY_PARAMETERS_START 21
#define KEY_ARRAY_PARAMETERS_END 23
#define KEY_PARAM_NAME   11
#define KEY_PARAM_AUTHOR 12
#define KEY_PARAM_VOLUME 13
#define KEY_PARAM_BG     14
#define KEY_PARAM_OG     15
#define KEY_PARAM_FG     16
#define KEY_PARAM_IBU    17
#define KEY_PARAM_EFFICIENCY 18
#define KEY_PARAM_SRM    19
#define KEY_PARAM_ABV    20
#define KEY_PARAM_HOPS   21
#define KEY_PARAM_MALTS  22
#define KEY_PARAM_YEASTS 23
#define KEY_PARAM_TYPE   24
#define KEY_PARAM_QUANTITY 25
#define KEY_PARAM_TIME   26

#define RESERVED_KEYWORDS 31
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
#define PUNC_ASGN 0
#define PUNC_LABL 1
#define PUNC_ARGS 2
#define PUNC_PP 3
#define PUNC_TERM 4
#define PUNC_ESC 5

#define PUNCTUATORS 6
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
