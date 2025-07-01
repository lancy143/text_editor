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

/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f)
#define KILO_VERSION "0.0.1"
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

enum editorKey {
  ARROW_LEFT = 200,//大于127
  ARROW_RIGHT ,
  ARROW_UP ,
  ARROW_DOWN ,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

/*** data ***/

typedef struct erow { //editor row
  int size;
  char *chars;
} erow;


struct editorConfig {
    int cx, cy; /* Cursor x and y position in characters */
    int screenrows; /* Number of rows that we can show */
    int screencols; /* Number of cols that we can show */
    int numrows; /* Number of rows */
    int rowoff;
    int coloff;
    char *filename; /* Currently open filename */
    erow *row;
    struct termios orig_termios;
};

struct editorConfig E;

/*** terminal *******************************************************************/
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

/*** row operations ***********************************************************/
void editorAppendRow(char *s, size_t len) {
  E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
  int at = E.numrows;
  E.row[at].size = len;
  E.row[at].chars = malloc(len + 1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';
  E.numrows++;
}

/*** file i/o ***********************************************************************/
void editorOpen(char *filename) {
//   char *line = "Hello, world!";
//   ssize_t linelen = 13;
    free (E.filename);
    E.filename = strdup(filename); //复制文件名到 E.filename
  FILE *fp = fopen(filename, "r");
  if (!fp) die("fopen");

  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen;
  linelen = getline(&line, &linecap, fp); //getline() 在到达文件末尾且没有更多行可读时返回 -1 。
  while ((linelen = getline(&line, &linecap, fp)) != -1) {
    while (linelen > 0 && (line[linelen - 1] == '\n' ||
                           line[linelen - 1] == '\r'))
      linelen--;
   editorAppendRow(line, linelen);
    }
    free (line);
    fclose(fp);
}

/*** append buffer ********************************************************************/

struct abuf {
  char *b;
  int len;
};

#define ABUF_INIT {NULL, 0}

void abAppend(struct abuf *ab, const char *s, int len) {
  char *new = realloc(ab->b, ab->len + len);

  if (new == NULL) return;
  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}

void abFree(struct abuf *ab) {
  free(ab->b);
}



/*** output ***************************************************************/
void editorScroll() { //调整 E.rowoff 使光标正好位于可见窗口内
    if (E.cy < E.rowoff) {
        E.rowoff = E.cy;
    }
    if (E.cy >= E.rowoff + E.screenrows) {
        E.rowoff = E.cy - E.screenrows + 1;
    }
    if (E.cx < E.coloff) {
        E.coloff = E.cx;
    }
    if (E.cx >= E.coloff + E.screencols) {
        E.coloff = E.cx - E.screencols + 1;
    }
}

void editorDrawRows(struct abuf *ab) {
  int y;
  for (y = 0; y < E.screenrows; y++) {
    int filerow = E.rowoff + y;
    if (filerow >= E.numrows) {
        if (E.numrows == 0 && y == E.screenrows / 3) {
            char welcome[20];
            int welcomelen = snprintf(welcome, sizeof(welcome),"Hello, World!");
            //   "Text editor -- version %s", KILO_VERSION);
            if (welcomelen > E.screencols) welcomelen = E.screencols;
            abAppend(ab, welcome, welcomelen);
        } 
    
        else 
        {
            abAppend(ab, "~", 1);
        }
    } 
    else {
        int len = E.row[filerow].size - E.coloff; //当前行的长度
        if (len < 0) len = 0; 
        if (len > E.screencols) len = E.screencols; //如果长度大于屏幕宽度
        abAppend(ab, &E.row[filerow].chars[E.coloff], len);
    }

    abAppend(ab, "\x1b[K", 3);
   
      abAppend(ab, "\r\n", 2);
    }
}

void editorDrawStatusBar(struct abuf *ab) {
    abAppend(ab, "\x1b[7m", 4); //\x1b[7m"反色
    char status[80];
    // int len = snprintf(status, sizeof(status), "%.10s - %d lines",
    // E.filename ? E.filename : "[No Name]", E.numrows);
    int len = snprintf(status, sizeof(status), "%.20s - %d lines",
     ("[No Name]"), E.numrows);
    if (len > E.screencols) len = E.screencols;
        abAppend(ab, status, len);
    while (len < E.screencols) {
        abAppend(ab, " ", 1);
        len++;
    }
    abAppend(ab, "\x1b[m", 3);
}

void editorRefreshScreen() {
    editorScroll();
    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6);
    abAppend(&ab, "\x1b[H", 3); // Move cursor to the top-left corner

    editorDrawRows(&ab);// Draw the rows
    editorDrawStatusBar(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
    abAppend(&ab, buf, strlen(buf)); // 光标移动到左上

    abAppend(&ab, "\x1b[?25h", 6); //

    write(STDOUT_FILENO, ab.b, ab.len); //write() 将缓冲区中的内容写入标准输出
    
    abFree(&ab);
}



/*** input ***/

void editorMoveCursor(int key) {
  switch (key) {
    case ARROW_LEFT:
      if (E.cx != 0) E.cx--;
      else if (E.cy > 0){
        E.cy--;
        E.cx = E.row[E.cy].size; // 上一行行尾
      }
      break;
    case ARROW_RIGHT:
      E.cx++;
      if (E.cx > E.row[E.cy].size) {
        E.cy ++;
        E.cx = 0; 
        if (E.cy >= E.numrows) {
          E.cy = 0;
          E.cx = 0; // 如果超过行数，回到第一行
        }
      }
      break;
    case ARROW_UP:
      if (E.cy != 0) E.cy--;
      break;
    case ARROW_DOWN:
      if (E.cy < E.numrows)  E.cy++;
      break;
  }
}

void editorProcessKeypress() {
  int c = editorReadKey();
  switch (c) {
    case CTRL_KEY('q'):
    //退出清屏
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
    case HOME_KEY:
        E.cx = 0; // Move cursor to the beginning of the line
        break;
    case END_KEY:
        E.cx = E.screencols - 1; // Move cursor to the end
        break;
    case PAGE_UP:
    case PAGE_DOWN:
        {
            int times = E.screenrows;
            if (c == PAGE_UP && E.cy != 0) {
                E.cy = 0;
            } else if (c == PAGE_DOWN && E.cy != E.screenrows - 1) {
                E.cy = E.screenrows - 1;
            }
            while (times--) {
                editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;
        }
    case ARROW_LEFT:
    case ARROW_RIGHT:
    case ARROW_UP:
    case ARROW_DOWN:
        editorMoveCursor(c);
        break;
  }
}



/*** init ******************************************************************/

void initEditor() {
    E.cx = 0;
    E.cy = 0;
    E.screenrows = 0;
    E.screencols = 0;
    E.numrows = 0; 
    E.rowoff = 0;
    E.coloff = 0;
    E.row = NULL;
    E.filename = NULL; // Initialize filename to NULL
    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
    E.screenrows -= 1; // Reserve space for status bar
}

int main(int argc, char *argv[]) {
    enableRawMode();
    initEditor();
    if (argc >= 2) 
    {
        editorOpen(argv[1]);
    }
    while(1){
        editorRefreshScreen();
        editorProcessKeypress();
    }

  return 0;
}
