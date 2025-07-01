#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

void die(const char *s) {
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) //当前属性读入结构体 raw
    die("tcsetattr"); 
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST); //
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0; /* Return each byte, or zero for timeout. */
  raw.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tens of second). */

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {//从标准输入读取一个字节
    if (nread == -1 && errno != EAGAIN) die("read"); 
  }
    if (c == '\x1b') { 
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b'; //读取下一个字节
        if (read(STDIN_FILENO, &seq[1] , 1) != 1) return '\x1b'; //读取下下个字节
        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') { 
                if (read(STDIN_FILENO, seq + 2, 1) != 1) return '\x1b'; 
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        //case '3': return DEL_KEY; //删除键
                        case '5': return PAGE_UP; 
                        case '6': return PAGE_DOWN; 
                    }
                }
            }
            else 
            {
                switch (seq[1]) {
                    case 'A': return ARROW_UP; //上
                    case 'B': return ARROW_DOWN; //下
                    case 'C': return ARROW_RIGHT; //右
                    case 'D': return ARROW_LEFT; //左
                }
            }
        }
        else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return HOME_KEY; // Home key
                case 'F': return END_KEY; // End key
            }
        }
        return '\x1b';
    }
    else{
        return c;
    }
  
}

int getCursorPosition(int *rows, int *cols) {
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

  printf("\r\n");
  char c;
  while (read(STDIN_FILENO, &c, 1) == 1) {
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
  }

  editorReadKey();

  return -1;
}

int getWindowSize(int *rows, int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}