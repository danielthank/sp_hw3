#include <stdarg.h>
#include <time.h>

//void err_doit(int, int, const char *, va_list);

void err_dump(const char *, ...);

void err_sys(const char *, ...);

void err_quit(const char *, ...);

void create_time(struct timespec* spec, long time);

long create_long(struct timespec* spec);

