#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define CURSOR_INVIS    0
/* We get keys correspond to ASCII integer value */
#define KEY_QUIT    0x71
#define KEY_TAB     0x09

#define DIRECTORYLINES 1000
#define MAXDIRWIDTH 100
#define STRBUFF 66
#define STRBUFFET 200

char dirlines[DIRECTORYLINES][MAXDIRWIDTH];
int g_row;
int g_col;
int d_id = 0;
int d_sel_id = 0;
int d_size = 0;

typedef struct windowData {
    WINDOW* window;
    int     boxed;
    char*   titleBar;
    int     state;
} windowData;

struct windows{
    windowData* directory;
    windowData* editor;
    windowData* toppanel;
    windowData* bottompanel;
    int         state;
} windows;

enum states {
    never = 0,
    dir,
    edit
};

enum colorpairs {
    normal = 1,
    highlight,
    panel
};

void create_newwin(int height, int width, int starty, int startx, int boxed, char* title, int state, windowData *wd);
void render(void);
void drawwindow(windowData *wd);
void initialisecolors(void);
void getdirectoryinfo();
void drawdirectory(windowData *wd);

int main( int argc, char *argv[]) {
    int ch;
    char * toptext = "Made by RisingThumb          https://risingthumb.xyz ";
    char * bottomtext = "TAB switch menu    Q to quit";
    windowData dirwin, editwin, panwin, panwinbottom;

    windows.directory = &dirwin;
    windows.editor = &editwin;
    windows.toppanel = &panwin;
    windows.bottompanel = &panwinbottom;
    windows.state = dir;

    initscr();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(CURSOR_INVIS);
    raw();
    start_color();
    initialisecolors();
    refresh();
    getmaxyx(stdscr, g_row, g_col);
             /*  height, width,  y, x,          boxed,  title, windowData */
    create_newwin(g_row-2, g_col/2, 1, 0, 1, "-Directory-", dir, &dirwin);
    create_newwin(g_row-2, g_col/2, 1, g_col/2, 1, "-Files-", edit, &editwin);
    create_newwin(1, g_col, 0, (g_col-strlen(toptext))/2, 0, toptext, never, &panwin);
    create_newwin(1, g_col, g_row-1, (g_col-strlen(bottomtext))/2, 0, bottomtext, never, &panwinbottom);
    getdirectoryinfo();
    for(render(); tolower(ch = getch()) != KEY_QUIT; render()){
        if (windows.state == dir) {
            switch(ch) {
                case KEY_TAB:
                    windows.state = edit;
                    continue;
                case KEY_DOWN:
                    if (d_sel_id < d_size - 1)
                        d_sel_id += 1;
                    continue;
                case KEY_UP:
                    if (d_sel_id > 0)
                        d_sel_id -= 1;
                    continue;
                case 'o':
                    chdir(dirlines[d_sel_id]);
                    getdirectoryinfo();
                    continue;
            }
        }
        if (windows.state == edit) {
            switch(ch) {
                case KEY_TAB:
                    windows.state = dir;
            }
        }
    }
    endwin();
    return 0;
}

void getdirectoryinfo() {
    int errno;
    FILE *fp;
    int width = getmaxx(windows.directory->window);
    char line[width];
    int i = 1;
    int len;

    errno = system("ls -1 > /tmp/ctag-temp");
    if (errno != 0)
        return; /* todo: unfuck. Return error to user */
    fp = fopen("/tmp/ctag-temp", "r");
    strcpy(dirlines[0], "..");
    while(fgets(line, sizeof(line), fp) != NULL) {
        len = strlen(line);
        if (line[len-1] == '\n')
            line[len-1] = 0;
        strcpy(dirlines[i++], line);
        mvwprintw(windows.directory->window, i, 1, line);
    }
    fclose(fp);
    d_size = i;
}

void initialisecolors() {
    init_pair(normal, COLOR_WHITE, COLOR_BLACK);
    init_pair(highlight, COLOR_YELLOW, COLOR_BLACK);
    init_pair(panel, COLOR_BLUE, COLOR_BLACK);
}

void create_newwin(int height, int width, int starty, int startx, int boxed, char* title, int state, windowData *wd) {
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    wd->window=local_win;
    wd->boxed=boxed;
    wd->titleBar=title;
    wd->state = state;
}

void render() {
    wclear(windows.directory->window);
    wclear(windows.editor->window);

    drawdirectory(windows.directory);
    drawwindow(windows.directory);

    drawwindow(windows.editor);
    drawwindow(windows.toppanel);
    drawwindow(windows.bottompanel);
}

void drawdirectory(windowData *wd) {
    int i = 0;
    WINDOW* w = wd->window;

    for(; i < d_size; i++) {
        if (d_sel_id == i)
            wattron(w, A_REVERSE);
        mvwprintw(w, i+1, 1, dirlines[i]);
        wattroff(w, A_REVERSE);
    }
}

void drawwindow(windowData *wd) {
    WINDOW* w = wd->window;
    if (windows.state && windows.state == wd->state)
        wattron(w, COLOR_PAIR(highlight));
    if (wd->boxed)
        box(w, 0, 0);
    wattroff(w, COLOR_PAIR(highlight));
    wattron(w, A_BOLD | A_REVERSE | COLOR_PAIR(panel));
    mvwprintw(w, 0, 1, wd->titleBar);
    wattroff(w, A_BOLD | A_REVERSE | COLOR_PAIR(panel));

    wrefresh(w);
}
