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


