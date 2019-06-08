#include "brewshell_lang.h"

const char *reserved[RESERVED_KEYWORDS] = {
	"brew","create","open","save","export","add","rm","print"
};

const char *operator[OPERATORS] = {"+","-","*","/","%","<<",">>","."};

const char *punctuator[PUNCTUATORS] = {":",",",";","\\"};

const char *char_ini[CHARSETS] = {"0123456789",              "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",           " \r\n\t\0", "([{}])", "+-*/%><=!&|,^~.`@$", "\"'", ":,;\\", "#" };
const char *char_set[CHARSETS] = {"0123456789xabcdefABCDEF", "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", " \r\t\0",   "\x03",   "+-*/%><=!&|^~.`@$",  "",    "\x03" , ""  };
const char *char_end[CHARSETS] = {"",                        "",                                                                "",          "",       "",                   "\"'", ""     , "\n"};

const char *comment_ini[COMMENTS] = {"#",  "//", "/*", "*/"};
const char *comment_end[COMMENTS] = {"\n", "\n", "*/", ""};
