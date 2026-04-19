#ifndef UTILS_H
#define UTILS_H

/* =========================================================
 * utils.h  –  Small helper utilities
 * ========================================================= */

/* Trim leading/trailing whitespace in-place */
void trim(char *s);

/* Split a CSV line into tokens; returns number of tokens */
int split_csv(char *line, char **tokens, int max_tokens);

/* Print a horizontal separator line of given width */
void print_separator(int width);

/* Print a banner / heading */
void print_banner(const char *title);

#endif /* UTILS_H */
