#ifndef DEBUGSYS_H
#define DEBUGSYS_H
#include <stdio.h>
#include <stdarg.h>

#ifdef linux
#define DEBUG_COLOR_BLUE	"\033[34m"
#define DEBUG_COLOR_GREEN	"\033[32m"
#define DEBUG_COLOR_MAGENTA	"\033[35m"
#define DEBUG_COLOR_YELLOW	"\033[33m"
#define DEBUG_COLOR_RESET	"\033[0m"
#else
#define DEBUG_COLOR_BLUE ""
#define DEBUG_COLOR_GREEN ""
#define DEBUG_COLOR_MAGENTA ""
#define DEBUG_COLOR_YELLOW ""
#define DEBUG_COLOR_RESET ""
#endif

#define DBLVL_NONE	0
#define DBLVL_WARN	1
#define DBLVL_INFO	2
#define DBLVL_DEBUG	3

#define DBLVL DBLVL_INFO

static unsigned int inflvl;
#define MAXIL 1

#if DBLVL >= DBLVL_WARN
#define debug_print(tag, pcol, format, ...) printf("%s" tag " %s " format "\n", pcol, DEBUG_COLOR_RESET __VA_OPT__(,) __VA_ARGS__)
#define debug_warn(format, ...) debug_print("[WARNING]", DEBUG_COLOR_YELLOW, format __VA_OPT__(,) __VA_ARGS__)
#else
#define debug_print(tag, pcol, format, ...)
#define debug_warn(format, ...)
#endif

#if DBLVL >= DBLVL_INFO
//#define debug_inform(format, ...) debug_print("[INFO]   ", DEBUG_COLOR_GREEN, format __VA_OPT__(,) __VA_ARGS__)
inline void debug_inform(char *format, ...) {
	if (inflvl <= MAXIL) {
		printf(DEBUG_COLOR_GREEN "[INFO]   " DEBUG_COLOR_RESET);
		for (int i = 0; i <= inflvl; i++) {
			printf(" ");
		}
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
		printf("\n");
	}
}
#else
#define debug_inform(format, ...)
#endif

#if DBLVL >= DBLVL_DEBUG
//#define debug_log(format, ...) debug_print("[DEBUG]            ", DEBUG_COLOR_BLUE, "%s%s::%u:%s " format, DEBUG_COLOR_MAGENTA, __FUNCTION__, __LINE__, DEBUG_COLOR_RESET __VA_OPT__(,) __VA_ARGS__)
inline void debug_log(char *format, ...) {
	printf(DEBUG_COLOR_BLUE "[DEBUG]  " DEBUG_COLOR_RESET);
	for (int i = 0; i <= (inflvl + 1); i++) {
		printf(" ");
	}
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}
#else
#define debug_log(format, ...)
#endif

#endif /* DEBUGSYS_H */
