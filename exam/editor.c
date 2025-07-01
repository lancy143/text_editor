#include <stdlib.h>
#include <stdio.h>


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

void editorRowInsertChar(erow *row, int at, int c) {
    if (at < 0 || at > row->size) at = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    row->size++;
    row->chars[at] = c;
    editorUpdateRow(row);
}

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
