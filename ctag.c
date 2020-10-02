#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CURSOR_INVIS 0
#define ESCAPE_KEY  27

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

int main( int argc, char *argv[]) {
    int row, col;
    int ch;
    char * toptext = "Made by RisingThumb          https://risingthumb.xyz ";
    char * bottomtext = "TAB switch menu    ESC quit";
    windowData dirwin, editwin, panwin, panwinbottom;

    windows.directory = &dirwin;
    windows.editor = &editwin;
    windows.toppanel = &panwin;
    windows.bottompanel = &panwinbottom;
    windows.state = dir;

    initscr();
    noecho();
    curs_set(CURSOR_INVIS);
    raw();
    start_color();
    initialisecolors();
    refresh();
    getmaxyx(stdscr, row, col);
             /*  height, width,  y, x,          boxed,  title, windowData */
    create_newwin(row-2, col/2, 1, 0, 1, "-Directory-", dir, &dirwin);
    create_newwin(row-2, col/2, 1, col/2, 1, "-Files-", edit, &editwin);
    create_newwin(1, col, 0, (col-strlen(toptext))/2, 0, toptext, never, &panwin);
    create_newwin(1, col, row-1, (col-strlen(bottomtext))/2, 0, bottomtext, never, &panwinbottom);

    for(render(); (ch = getch()) != ESCAPE_KEY; render()){
        switch(ch) {
            case '\t':
                if (windows.state == edit)
                    windows.state = dir;
                else
                    windows.state = edit;
                break;
        }
    }
    endwin();
    return 0;
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
    drawwindow(windows.directory);
    drawwindow(windows.editor);
    drawwindow(windows.toppanel);
    drawwindow(windows.bottompanel);
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
