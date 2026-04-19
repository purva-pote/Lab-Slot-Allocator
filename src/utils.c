/* =========================================================
 * utils.c  –  Helper utilities
 * ========================================================= */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

/* ----------------------------------------------------------
 * trim()
 * Remove leading and trailing whitespace (spaces, \r, \n)
 * from string s, modifying it in place.
 * ---------------------------------------------------------- */
void trim(char *s) {
    if (!s) return;

    /* Remove trailing whitespace */
    int len = (int)strlen(s);
    while (len > 0 && (isspace((unsigned char)s[len - 1]))) {
        s[--len] = '\0';
    }

    /* Remove leading whitespace */
    int start = 0;
    while (s[start] && isspace((unsigned char)s[start])) {
        start++;
    }
    if (start > 0) {
        memmove(s, s + start, len - start + 1);
    }
}

/* ----------------------------------------------------------
 * split_csv()
 * Split a CSV line (comma-separated) into token pointers.
 * Modifies `line` in place (replaces commas with '\0').
 * Returns the number of tokens found.
 * ---------------------------------------------------------- */
int split_csv(char *line, char **tokens, int max_tokens) {
    int count = 0;
    char *ptr = line;

    while (*ptr && count < max_tokens) {
        tokens[count++] = ptr;
        /* Advance to the next comma or end-of-string */
        while (*ptr && *ptr != ',') ptr++;
        if (*ptr == ',') {
            *ptr = '\0'; /* terminate this token */
            ptr++;       /* move past the comma   */
        }
    }

    /* Trim each token */
    for (int i = 0; i < count; i++) {
        trim(tokens[i]);
    }

    return count;
}

/* ----------------------------------------------------------
 * print_separator()
 * Print `width` dashes followed by a newline.
 * ---------------------------------------------------------- */
void print_separator(int width) {
    for (int i = 0; i < width; i++) putchar('-');
    putchar('\n');
}

/* ----------------------------------------------------------
 * print_banner()
 * Print a centered title inside a box of '=' characters.
 * ---------------------------------------------------------- */
void print_banner(const char *title) {
    int width = 60;
    print_separator(width);
    int title_len = (int)strlen(title);
    int padding   = (width - title_len) / 2;
    for (int i = 0; i < padding; i++) putchar(' ');
    printf("%s\n", title);
    print_separator(width);
}
