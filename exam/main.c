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
#include "editor_core.h"
#include "editor.h"
#include "terminal.h"
#include "file_io.h"
#include "display.h"

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

/*** init ******************************************************************/



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

