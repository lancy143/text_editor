#ifndef EDITOR_H
#define EDITOR_H

#include <termios.h>
#include <stddef.h> // for size_t

/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f)
#define KILO_VERSION "0.0.1"
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE


/* 定义 editor 行结构 */
typedef struct erow { 
  int size;
  char *chars;
} erow;

/*** key enum ***/
enum editorKey {
  ARROW_LEFT = 200,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};


/* 编辑器配置结构体 */
extern editorConfig {
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

/* 声明一个全局 editorConfig 实例 */
extern struct editorConfig E;

void editorAppendRow(char *s, size_t len) ;

void editorRowInsertChar(erow *row, int at, int c) ;


void initEditor();

#endif // EDITOR_H
