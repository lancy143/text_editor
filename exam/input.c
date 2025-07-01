#include <termios.h>
#include "file_io.h"

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
    default:
       editorInsertChar(c);
       break;
  }
}


