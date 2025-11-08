#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define RESET "\033[0m"
#define MAGENTA "\033[0;35m"

enum arrow { UP = 0, DOWN = 1 };

struct termios start_raw_mode(int);
int wherey(int);
void gotoxy(int, int);
bool should_quit(char *);
void redraw_line(char **, int, int, int);

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

  // init & start menu
  lower_bound = wherey(fd);
  pos = lower_bound;
  upper_bound = lower_bound + argc - 2;
  for (int i = 1; i < argc; i++) {
    printf("%s\n\r", argv[i]);
  }
  redraw_line(argv + 1, lower_bound, pos, upper_bound);

  while (read(STDIN_FILENO, &c, 1) == 1 && should_quit(&c)) {
    if (c == '[') {
      read(fd, &c, 1);
      switch (c - 'A') {
      case UP: {
        if (pos > lower_bound) {
          pos--;
          redraw_line(argv + 1, lower_bound, pos, upper_bound);
        }
        break;
      }
      case DOWN: {
        if (pos < upper_bound) {
          pos++;
          redraw_line(argv + 1, lower_bound, pos, upper_bound);
        }
        break;
      }
      default: {
        break;
      }
      }
    }
  }

  // Stop raw mode
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
  write(fd, "\x1b[6n", 4);
  // Read the response from stdin: ESC [ <row> ; <col> R
  scanf("\x1b[%d;%*dR", &row);
  return row;
}

void gotoxy(int x, int y) { printf("\033[%d;%dH", y, x); }

// Business
bool should_quit(char *c) {
  // Enter
  return *c != '\r'
         // Ctrl+c
         && *c != 3;
}

void redraw_line(char **s, int lower_bound, int pos, int upper_bound) {
  int i, len;
  i = pos - lower_bound;
  len = upper_bound - lower_bound;
  if (i == 0) {
    gotoxy(1, pos + 1);
    printf(RESET "%s\n", s[i + 1]);
  } else if (i == len) {
    gotoxy(1, pos - 1);
    printf(RESET "%s\n", s[i - 1]);
  } else {
    gotoxy(1, pos - 1);
    printf(RESET "%s\n", s[i - 1]);
    gotoxy(1, pos + 1);
    printf(RESET "%s\n", s[i + 1]);
  }

  gotoxy(1, pos);
  printf(MAGENTA "%s\n", s[i]);

  gotoxy(1, upper_bound + 1);
  printf(RESET
         "lower_bound: %d  pos: %d  upper_bound: %d  return value: %d\r\n",
         lower_bound, pos, upper_bound, pos - lower_bound);
}
