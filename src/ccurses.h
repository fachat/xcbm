
#include <curses.h>

extern int color;

int cur_init(void);

void cur_setup(void);
void cur_exit(void);

int cur_getch();

ssize_t cur_getline(char **line, size_t *llen);

