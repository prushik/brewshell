#define LANG_CUSTOM -1
#define LANG_MAINSPRING 0
#define LANG_MAINSPRING_PREPROCESSOR 1
#define LANG_MAINSPRING_ASSEMBLER 2
#define LANG_BREWSHELL 4

#ifndef LANG
	#define LANG LANG_BREWSHELL
#endif

#if LANG == LANG_MAINSPRING
	#include "mainspring_lang.h"
#endif

#if LANG == LANG_MAINSPRING_PREPROCESSOR
	#include "preprocessor_lang.h"
#endif

#if LANG == LANG_MAINSPRING_ASSEMBLER
	#include "assembler_lang.h"
#endif

#if LANG == LANG_BREWSHELL
	#include "brewshell_lang.h"
#endif

#if LANG == LANG_CUSTOM
	#ifdef LANG_CUSTOM_HEADER
		#include LANG_CUSTOM_HEADER
	#else
		#warning "No language header specified, using defaults. This is probably not what you want..."
	#endif
#endif

#include "lang_defaults.h"
