#ifndef COLOR_H
#define COLOR_H

extern int COLOR_LEN;

const char *color(const char *col, const char *message);
void clr_output_push(FILE *output, const char *clr);
void clr_output_pop(FILE *output);

#endif //COLOR_H
