/********************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)     *
 * Copyright (c) 2025 MacroModel. All rights reserved.  *
 * Licensed under the APL-2 License (see LICENSE file). *
 ********************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2025-03-23
 * @copyright   APL-2 License
 */

/****************************************
 *  _   _ __        ____     __ __  __  *
 * | | | |\ \      / /\ \   / /|  \/  | *
 * | | | | \ \ /\ / /  \ \ / / | |\/| | *
 * | |_| |  \ V  V /    \ V /  | |  | | *
 *  \___/    \_/\_/      \_/   |_|  |_| *
 *                                      *
 ****************************************/

/// @brief      The following are the macros used by ansi escape sequences.
/// @details    Use `push_macro` to avoid side effects on existing macros. Please use `pop_macro` in conjunction.

#pragma once

/// @brief Text color variables:

#pragma push_macro("UWVM_AES_U8_BLACK")
#undef UWVM_AES_U8_BLACK
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BLACK u8""
#else
# define UWVM_AES_U8_BLACK u8"\033[30m"
#endif

#pragma push_macro("UWVM_AES_U8_RED")
#undef UWVM_AES_U8_RED
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_RED u8""
#else
# define UWVM_AES_U8_RED u8"\033[31m"
#endif

#pragma push_macro("UWVM_AES_U8_GREEN")
#undef UWVM_AES_U8_GREEN
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_GREEN u8""
#else
# define UWVM_AES_U8_GREEN u8"\033[32m"
#endif

#pragma push_macro("UWVM_AES_U8_ORANGE")
#undef UWVM_AES_U8_ORANGE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_ORANGE u8""
#else
# define UWVM_AES_U8_ORANGE u8"\033[33m"
#endif

#pragma push_macro("UWVM_AES_U8_BLUE")
#undef UWVM_AES_U8_BLUE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BLUE u8""
#else
# define UWVM_AES_U8_BLUE u8"\033[34m"
#endif

#pragma push_macro("UWVM_AES_U8_PURPLE")
#undef UWVM_AES_U8_PURPLE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_PURPLE u8""
#else
# define UWVM_AES_U8_PURPLE u8"\033[35m"
#endif

#pragma push_macro("UWVM_AES_U8_CYAN")
#undef UWVM_AES_U8_CYAN
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_CYAN u8""
#else
# define UWVM_AES_U8_CYAN u8"\033[36m"
#endif

#pragma push_macro("UWVM_AES_U8_GRAY")
#undef UWVM_AES_U8_GRAY
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_GRAY u8""
#else
# define UWVM_AES_U8_GRAY u8"\033[37m"
#endif

#pragma push_macro("UWVM_AES_U8_DK_GRAY")
#undef UWVM_AES_U8_DK_GRAY
#if defined(__DJGPP__)
# define UWVM_AES_U8_DK_GRAY UWVM_AES_U8_BLACK
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_DK_GRAY u8""
#else
# define UWVM_AES_U8_DK_GRAY u8"\033[90m"
#endif

#pragma push_macro("UWVM_AES_U8_LT_RED")
#undef UWVM_AES_U8_LT_RED
#if defined(__DJGPP__)
# define UWVM_AES_U8_LT_RED UWVM_AES_U8_RED
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_LT_RED u8""
#else
# define UWVM_AES_U8_LT_RED u8"\033[91m"
#endif

#pragma push_macro("UWVM_AES_U8_LT_GREEN")
#undef UWVM_AES_U8_LT_GREEN
#if defined(__DJGPP__)
# define UWVM_AES_U8_LT_GREEN UWVM_AES_U8_GREEN
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_LT_GREEN u8""
#else
# define UWVM_AES_U8_LT_GREEN u8"\033[92m"
#endif

#pragma push_macro("UWVM_AES_U8_YELLOW")
#undef UWVM_AES_U8_YELLOW
#if defined(__DJGPP__)
# define UWVM_AES_U8_YELLOW UWVM_AES_U8_ORANGE
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_YELLOW u8""
#else
# define UWVM_AES_U8_YELLOW u8"\033[93m"
#endif

#pragma push_macro("UWVM_AES_U8_LT_BLUE")
#undef UWVM_AES_U8_LT_BLUE
#if defined(__DJGPP__)
# define UWVM_AES_U8_LT_BLUE UWVM_AES_U8_BLUE
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_LT_BLUE u8""
#else
# define UWVM_AES_U8_LT_BLUE u8"\033[94m"
#endif

#pragma push_macro("UWVM_AES_U8_LT_PURPLE")
#undef UWVM_AES_U8_LT_PURPLE
#if defined(__DJGPP__)
# define UWVM_AES_U8_LT_PURPLE UWVM_AES_U8_PURPLE
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_LT_PURPLE u8""
#else
# define UWVM_AES_U8_LT_PURPLE u8"\033[95m"
#endif

#pragma push_macro("UWVM_AES_U8_LT_CYAN")
#undef UWVM_AES_U8_LT_CYAN
#if defined(__DJGPP__)
# define UWVM_AES_U8_LT_CYAN UWVM_AES_U8_CYAN
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_LT_CYAN u8""
#else
# define UWVM_AES_U8_LT_CYAN u8"\033[96m"
#endif

#pragma push_macro("UWVM_AES_U8_WHITE")
#undef UWVM_AES_U8_WHITE
#if defined(__DJGPP__)
# define UWVM_AES_U8_WHITE UWVM_AES_U8_GRAY
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_WHITE u8""
#else
# define UWVM_AES_U8_WHITE u8"\033[97m"
#endif

/// @brief Background color variables:

#pragma push_macro("UWVM_AES_U8_BG_BLACK")
#undef UWVM_AES_U8_BG_BLACK
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_BLACK u8""
#else
# define UWVM_AES_U8_BG_BLACK u8"\033[40m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_RED")
#undef UWVM_AES_U8_BG_RED
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_RED u8""
#else
# define UWVM_AES_U8_BG_RED u8"\033[41m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_GREEN")
#undef UWVM_AES_U8_BG_GREEN
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_GREEN u8""
#else
# define UWVM_AES_U8_BG_GREEN u8"\033[42m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_ORANGE")
#undef UWVM_AES_U8_BG_ORANGE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_ORANGE u8""
#else
# define UWVM_AES_U8_BG_ORANGE u8"\033[43m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_BLUE")
#undef UWVM_AES_U8_BG_BLUE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_BLUE u8""
#else
# define UWVM_AES_U8_BG_BLUE u8"\033[44m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_PURPLE")
#undef UWVM_AES_U8_BG_PURPLE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_PURPLE u8""
#else
# define UWVM_AES_U8_BG_PURPLE u8"\033[45m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_CYAN")
#undef UWVM_AES_U8_BG_CYAN
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_CYAN u8""
#else
# define UWVM_AES_U8_BG_CYAN u8"\033[46m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_GRAY")
#undef UWVM_AES_U8_BG_GRAY
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_GRAY u8""
#else
# define UWVM_AES_U8_BG_GRAY u8"\033[47m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_DK_GRAY")
#undef UWVM_AES_U8_BG_DK_GRAY
#if defined(__DJGPP__)
# define UWVM_AES_U8_BG_DK_GRAY UWVM_AES_U8_BG_BLACK
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_DK_GRAY u8""
#else
# define UWVM_AES_U8_BG_DK_GRAY u8"\033[100m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_LT_RED")
#undef UWVM_AES_U8_BG_LT_RED
#if defined(__DJGPP__)
# define UWVM_AES_U8_BG_LT_RED UWVM_AES_U8_BG_RED
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_LT_RED u8""
#else
# define UWVM_AES_U8_BG_LT_RED u8"\033[101m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_LT_GREEN")
#undef UWVM_AES_U8_BG_LT_GREEN
#if defined(__DJGPP__)
# define UWVM_AES_U8_BG_LT_GREEN UWVM_AES_U8_BG_GREEN
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_LT_GREEN u8""
#else
# define UWVM_AES_U8_BG_LT_GREEN u8"\033[102m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_YELLOW")
#undef UWVM_AES_U8_BG_YELLOW
#if defined(__DJGPP__)
# define UWVM_AES_U8_BG_YELLOW UWVM_AES_U8_BG_ORANGE
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_YELLOW u8""
#else
# define UWVM_AES_U8_BG_YELLOW u8"\033[103m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_LT_BLUE")
#undef UWVM_AES_U8_BG_LT_BLUE
#if defined(__DJGPP__)
# define UWVM_AES_U8_BG_LT_BLUE UWVM_AES_U8_BG_BLUE
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_LT_BLUE u8""
#else
# define UWVM_AES_U8_BG_LT_BLUE u8"\033[104m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_LT_PURPLE")
#undef UWVM_AES_U8_BG_LT_PURPLE
#if defined(__DJGPP__)
# define UWVM_AES_U8_BG_LT_PURPLE UWVM_AES_U8_BG_PURPLE
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_LT_PURPLE u8""
#else
# define UWVM_AES_U8_BG_LT_PURPLE u8"\033[105m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_LT_CYAN")
#undef UWVM_AES_U8_BG_LT_CYAN
#if defined(__DJGPP__)
# define UWVM_AES_U8_BG_LT_CYAN UWVM_AES_U8_BG_CYAN
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_LT_CYAN u8""
#else
# define UWVM_AES_U8_BG_LT_CYAN u8"\033[106m"
#endif

#pragma push_macro("UWVM_AES_U8_BG_WHITE")
#undef UWVM_AES_U8_BG_WHITE
#if defined(__DJGPP__)
# define UWVM_AES_U8_BG_WHITE UWVM_AES_U8_BG_GRAY
#elif (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BG_WHITE u8""
#else
# define UWVM_AES_U8_BG_WHITE u8"\033[107m"
#endif

/// @brief Bold color variables:
#pragma push_macro("UWVM_AES_U8_BD_BLACK")
#undef UWVM_AES_U8_BD_BLACK
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BD_BLACK u8""
#else
# define UWVM_AES_U8_BD_BLACK u8"\033[1;30m"
#endif

#pragma push_macro("UWVM_AES_U8_BD_RED")
#undef UWVM_AES_U8_BD_RED
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BD_RED u8""
#else
# define UWVM_AES_U8_BD_RED u8"\033[1;31m"
#endif

#pragma push_macro("UWVM_AES_U8_BD_GREEN")
#undef UWVM_AES_U8_BD_GREEN
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BD_GREEN u8""
#else
# define UWVM_AES_U8_BD_GREEN u8"\033[1;32m"
#endif

#pragma push_macro("UWVM_AES_U8_BD_ORANGE")
#undef UWVM_AES_U8_BD_ORANGE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BD_ORANGE u8""
#else
# define UWVM_AES_U8_BD_ORANGE u8"\033[1;33m"
#endif

#pragma push_macro("UWVM_AES_U8_BD_BLUE")
#undef UWVM_AES_U8_BD_BLUE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BD_BLUE u8""
#else
# define UWVM_AES_U8_BD_BLUE u8"\033[1;34m"
#endif

#pragma push_macro("UWVM_AES_U8_BD_PURPLE")
#undef UWVM_AES_U8_BD_PURPLE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BD_PURPLE u8""
#else
# define UWVM_AES_U8_BD_PURPLE u8"\033[1;35m"
#endif

#pragma push_macro("UWVM_AES_U8_BD_CYAN")
#undef UWVM_AES_U8_BD_CYAN
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BD_CYAN u8""
#else
# define UWVM_AES_U8_BD_CYAN u8"\033[1;36m"
#endif

#pragma push_macro("UWVM_AES_U8_BD_GRAY")
#undef UWVM_AES_U8_BD_GRAY
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_BD_GRAY u8""
#else
# define UWVM_AES_U8_BD_GRAY u8"\033[1;37m"
#endif

/// @brief Style variables:

#pragma push_macro("UWVM_AES_U8_STY_BOLD")
#undef UWVM_AES_U8_STY_BOLD
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_STY_BOLD u8""
#else
# define UWVM_AES_U8_STY_BOLD u8"\033[1m"
#endif

#pragma push_macro("UWVM_AES_U8_STY_FAINT")
#undef UWVM_AES_U8_STY_FAINT
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_STY_FAINT u8""
#else
# define UWVM_AES_U8_STY_FAINT u8"\033[2m"
#endif

#pragma push_macro("UWVM_AES_U8_STY_ITALICS")
#undef UWVM_AES_U8_STY_ITALICS
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_STY_ITALICS u8""
#else
# define UWVM_AES_U8_STY_ITALICS u8"\033[3m"
#endif

#pragma push_macro("UWVM_AES_U8_STY_UNDERLINE")
#undef UWVM_AES_U8_STY_UNDERLINE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_STY_UNDERLINE u8""
#else
# define UWVM_AES_U8_STY_UNDERLINE u8"\033[4m"
#endif

#pragma push_macro("UWVM_AES_U8_STY_BLINK")
#undef UWVM_AES_U8_STY_BLINK
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_STY_BLINK u8""
#else
# define UWVM_AES_U8_STY_BLINK u8"\033[5m"
#endif

#pragma push_macro("UWVM_AES_U8_STY_INVERSE")
#undef UWVM_AES_U8_STY_INVERSE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_STY_INVERSE u8""
#else
# define UWVM_AES_U8_STY_INVERSE u8"\033[7m"
#endif

#pragma push_macro("UWVM_AES_U8_STY_INVISIABLE")
#undef UWVM_AES_U8_STY_INVISIABLE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_STY_INVISIABLE u8""
#else
# define UWVM_AES_U8_STY_INVISIABLE u8"\033[8m"
#endif

#pragma push_macro("UWVM_AES_U8_STY_CROSSED")
#undef UWVM_AES_U8_STY_CROSSED
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_STY_CROSSED u8""
#else
# define UWVM_AES_U8_STY_CROSSED u8"\033[9m"
#endif

#pragma push_macro("UWVM_AES_U8_STY_DOUBLE_UNDERLINE")
#undef UWVM_AES_U8_STY_DOUBLE_UNDERLINE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_STY_DOUBLE_UNDERLINE u8""
#else
# define UWVM_AES_U8_STY_DOUBLE_UNDERLINE u8"\033[21m"
#endif

/// @brief Reset features commands

#pragma push_macro("UWVM_AES_U8_RST_ALL")
#undef UWVM_AES_U8_RST_ALL
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || (defined(__MSDOS__) && !defined(__DJGPP__))
# define UWVM_AES_U8_RST_ALL u8""
#else
# define UWVM_AES_U8_RST_ALL u8"\033[0m"
#endif

#pragma push_macro("UWVM_AES_U8_RST_COLOR")
#undef UWVM_AES_U8_RST_COLOR
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || defined(__MSDOS__) || defined(__DJGPP__)
# define UWVM_AES_U8_RST_COLOR u8""
#else
# define UWVM_AES_U8_RST_COLOR u8"\033[39m"
#endif

#pragma push_macro("UWVM_AES_U8_RST_BG_COLOR")
#undef UWVM_AES_U8_RST_BG_COLOR
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || defined(__MSDOS__) || defined(__DJGPP__)
# define UWVM_AES_U8_RST_BG_COLOR u8""
#else
# define UWVM_AES_U8_RST_BG_COLOR u8"\033[49m"
#endif

#pragma push_macro("UWVM_AES_U8_RST_BD_COLOR")
#undef UWVM_AES_U8_RST_BD_COLOR
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || defined(__MSDOS__) || defined(__DJGPP__)
# define UWVM_AES_U8_RST_BD_COLOR u8""
#else
# define UWVM_AES_U8_RST_BD_COLOR u8"\033[22m\033[39m"
#endif

#pragma push_macro("UWVM_AES_U8_RST_BD_FT")
#undef UWVM_AES_U8_RST_BD_FT
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || defined(__MSDOS__) || defined(__DJGPP__)
# define UWVM_AES_U8_RST_BD_FT u8""
#else
# define UWVM_AES_U8_RST_BD_FT u8"\033[22m"
#endif

#pragma push_macro("UWVM_AES_U8_RST_ITALICS")
#undef UWVM_AES_U8_RST_ITALICS
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || defined(__MSDOS__) || defined(__DJGPP__)
# define UWVM_AES_U8_RST_ITALICS u8""
#else
# define UWVM_AES_U8_RST_ITALICS u8"\033[23m"
#endif

#pragma push_macro("UWVM_AES_U8_RST_UNDERLINE")
#undef UWVM_AES_U8_RST_UNDERLINE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || defined(__MSDOS__) || defined(__DJGPP__)
# define UWVM_AES_U8_RST_UNDERLINE u8""
#else
# define UWVM_AES_U8_RST_UNDERLINE u8"\033[24m"
#endif

#pragma push_macro("UWVM_AES_U8_RST_BLINK")
#undef UWVM_AES_U8_RST_BLINK
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || defined(__MSDOS__) || defined(__DJGPP__)
# define UWVM_AES_U8_RST_BLINK u8""
#else
# define UWVM_AES_U8_RST_BLINK u8"\033[25m"
#endif

#pragma push_macro("UWVM_AES_U8_RST_INVERSE")
#undef UWVM_AES_U8_RST_INVERSE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || defined(__MSDOS__) || defined(__DJGPP__)
# define UWVM_AES_U8_RST_INVERSE u8""
#else
# define UWVM_AES_U8_RST_INVERSE u8"\033[27m"
#endif

#pragma push_macro("UWVM_AES_U8_RST_INVISIABLE")
#undef UWVM_AES_U8_RST_INVISIABLE
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || defined(__MSDOS__) || defined(__DJGPP__)
# define UWVM_AES_U8_RST_INVISIABLE u8""
#else
# define UWVM_AES_U8_RST_INVISIABLE u8"\033[28m"
#endif

#pragma push_macro("UWVM_AES_U8_RST_CROSS")
#undef UWVM_AES_U8_RST_CROSS
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || defined(__MSDOS__) || defined(__DJGPP__)
# define UWVM_AES_U8_RST_CROSS u8""
#else
# define UWVM_AES_U8_RST_CROSS u8"\033[29m"
#endif

/// @brief Control sequences variables

/// @brief Hide cursor
#pragma push_macro("UWVM_AES_U8_CTRL_HCRS")
#undef UWVM_AES_U8_CTRL_HCRS
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || defined(__MSDOS__) || defined(__DJGPP__)
# define UWVM_AES_U8_CTRL_HCRS u8""
#else
# define UWVM_AES_U8_CTRL_HCRS u8"\033[?25l"
#endif

/// @brief Show cursor
#pragma push_macro("UWVM_AES_U8_CTRL_SCRS")
#undef UWVM_AES_U8_CTRL_SCRS
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || defined(__MSDOS__) || defined(__DJGPP__)
# define UWVM_AES_U8_CTRL_SCRS u8""
#else
# define UWVM_AES_U8_CTRL_SCRS u8"\033[?25h"
#endif

/// @brief RGB
#pragma push_macro("UWVM_AES_U8_RGB")
#undef UWVM_AES_U8_RGB
#if (defined(_WIN32) && defined(_WIN32_WINDOWS)) || defined(__MSDOS__) || defined(__DJGPP__)
# define UWVM_AES_U8_RGB(r, g, b) u8""
#else
# define UWVM_AES_U8_RGB(r, g, b) u8"\033[38;2;" #r u8";" #g u8";" #b u8"m"
#endif
