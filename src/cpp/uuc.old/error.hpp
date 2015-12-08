#ifndef ERROR_H
#define ERROR_H

void warning(const char *format, ...);
void error(const char *format, ...);
void fatal_error(const char *format, ...);
int error_count(void);

#endif //ERROR_H
