#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define COLOR_RESET "\033[0m"
#define COLOR_MAGENTA "\033[0;35m"
#define CURSOR_HIDE "\033[?25l"
#define CURSOR_POSITION "\x1b[6n"
#define CURSOR_SHOW "\033[?25h"

enum arrow { UP = 0, DOWN = 1 };

struct termios start_raw_mode(int);
int wherey(int);
void gotoxy(int, int, int);
void fd_write(int fd, char *);
int _strlen(char *);
bool should_quit(char *);
void print_list(int, char **, int);
void redraw_line(int, char **, int, int, int);

// TODO
// Add support for overflowing lists

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Missing parameters!\n");
        exit(EXIT_FAILURE);
    }

    char c;
    int lower_bound, pos, upper_bound;

    // init & start termios
    int fd = STDIN_FILENO;
    struct termios bkp_term;
    bkp_term = start_raw_mode(fd);
    fd_write(fd, CURSOR_HIDE);

    // init & start menu
    lower_bound = wherey(fd);
    pos = lower_bound;
    upper_bound = lower_bound + argc - 2;

    print_list(fd, argv + 1, argc);

    while (read(STDIN_FILENO, &c, 1) == 1 && should_quit(&c)) {
        if (c == '[') {
            read(fd, &c, 1);
            switch (c - 'A') {
            case UP: {
                if (pos > lower_bound) {
                    pos--;
                    redraw_line(fd, argv + 1, lower_bound, pos, upper_bound);
                }
                break;
            }
            case DOWN: {
                if (pos < upper_bound) {
                    pos++;
                    redraw_line(fd, argv + 1, lower_bound, pos, upper_bound);
                }
                break;
            }
            default: {
                break;
            }
            }
        }
    }

    gotoxy(fd, 1, upper_bound + 1);
    // Stop raw mode
    fd_write(fd, CURSOR_SHOW);
    tcsetattr(fd, TCSANOW, &bkp_term);

    return pos - lower_bound;
}

// Utilities
struct termios start_raw_mode(int fd) {
    struct termios term, bkp_term;

    tcgetattr(fd, &term);
    bkp_term = term;
    cfmakeraw(&term);
    tcsetattr(fd, TCSANOW, &term);

    return bkp_term;
}

int wherey(int fd) {
    int row;
    // Request cursor position
    write(fd, CURSOR_POSITION, 4);
    // Read the response from stdin: ESC [ <row> ; <col> R
    scanf("\x1b[%d;%*dR", &row);
    return row;
}

void gotoxy(int fd, int x, int y) {
    char escape_sequence[16];
    snprintf(escape_sequence, sizeof(escape_sequence), "\033[%d;%dH", y, x);
    write(fd, escape_sequence, _strlen(escape_sequence));
}

void fd_write(int fd, char *s) { write(fd, s, _strlen(s)); }

int _strlen(char *s) {
    char *p = s;
    while (*p++)
        ;
    return p - s;
}

// Business
bool should_quit(char *c) {
    // Enter
    return *c != '\r'
           // Ctrl+c
           && *c != 3;
}

void print_list(int fd, char **s, int argc) {
    int row = wherey(fd);

    gotoxy(fd, 1, row++);
    fd_write(fd, COLOR_MAGENTA);
    fd_write(fd, s[0]);
    fd_write(fd, COLOR_RESET);

    for (int i = 1; i < argc - 1; i++) {
        gotoxy(fd, 1, row++);
        fd_write(fd, s[i]);
    }
}

void redraw_line(int fd, char **s, int lower_bound, int pos, int upper_bound) {
    int i, len;
    i = pos - lower_bound;
    len = upper_bound - lower_bound;

    if (i > 0) {
        gotoxy(fd, 1, pos - 1);
        fd_write(fd, s[i - 1]);
    }
    gotoxy(fd, 1, pos);
    fd_write(fd, COLOR_MAGENTA);
    fd_write(fd, s[i]);
    fd_write(fd, COLOR_RESET);
    if (i < len) {
        gotoxy(fd, 1, pos + 1);
        fd_write(fd, s[i + 1]);
    }

    // Uncomment for debug purposes
    // gotoxy(fd, 1, upper_bound + 1);
    // char debug_output[256];
    // snprintf(debug_output, sizeof(debug_output),
    //          "lower_bound: %d  pos: %d  upper_bound: %d  return value:
    //          %d\r\n", lower_bound, pos, upper_bound, i);
    // fd_write(fd, debug_output);
}
