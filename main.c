#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
struct termios orig_termios;

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); //当前属性读入结构体 raw
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP);
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IXON);
  raw.c_cc[VMIN] = 0; /* Return each byte, or zero for timeout. */
  raw.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tens of second). */
  
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


int main() {
    enableRawMode();
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
    if (iscntrl(c)) {
      printf("%d\n", c);
    } else {
      printf("%d ('%c')\n", c, c);
    }
  }
    return 0;
}

