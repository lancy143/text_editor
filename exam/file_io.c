#include <errno.h>
#include <string.h>

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