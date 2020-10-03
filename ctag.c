#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#define CURSOR_INVIS    0
/* We get keys correspond to ASCII integer value */
#define KEY_QUIT    0x71
#define KEY_TAB     0x09
#define KEY_ENT     0x0a

#define DIRECTORYLINES 1000
#define MAXDIRWIDTH 256
#define STRBUFF 66
#define STRBUFFET 200

char dirlines[DIRECTORYLINES][MAXDIRWIDTH]; // Consider a malloc approach, so it extends to any directory with size>1000
char *filenameEditing;

typedef struct windowData {
    WINDOW* window;
    int     boxed;
    char*   titleBar;
    int     state;
    int     id; // Corresponds with position when scrolling up/down
    int     sel_id; // Corresponds with selected element
    int     size; // Size in number of elements
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

void createNewWindow(int height, int width, int starty, int startx, int boxed, char* title, int state, windowData *wd);
void render(void);
void drawWindow(windowData *wd);
void initialiseColors(void);
void getDirectoryInfo(int *size);
void drawDirectory(windowData *wd);
int isRegularFile(char* filename);
int compare(const void* pa, const void *pb);

int main( int argc, char *argv[]) {
    int ch;
    int row, col;
    char * toptext = "Made by RisingThumb          https://risingthumb.xyz ";
    char * bottomtext = "TAB switch menu    Q to quit";
    int *sel_id;
    int *size;
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
    initialiseColors();
    refresh();
    getmaxyx(stdscr, row, col);

    /* Create windows accordingly */
    createNewWindow(row-2, col/2, 1, 0, 1, "-Directory-", dir, &dirwin);
    createNewWindow(row-2, col/2, 1, col/2, 1, "-Files-", edit, &editwin);
    createNewWindow(1, col, 0, (col-strlen(toptext))/2, 0, toptext, never, &panwin);
    createNewWindow(1, col, row-1, (col-strlen(bottomtext))/2, 0, bottomtext, never, &panwinbottom);

    getDirectoryInfo(&windows.directory->size);
    for(render(); (ch = tolower(getch())) != KEY_QUIT; render()){
        if (windows.state == dir) {
            sel_id = &windows.directory->sel_id;
            size = &windows.directory->size;
            switch(ch) {
                case KEY_TAB:
                    windows.state = edit;
                    continue;
                case KEY_DOWN:
                    if ((*sel_id) < (*size) - 1)
                        (*sel_id) += 1;
                    continue;
                case KEY_UP:
                    if (*sel_id > 0)
                        (*sel_id) -= 1;
                    continue;
                case KEY_ENT:
                    wclear(windows.directory->window);
                    wclear(windows.editor->window);
                    if ( isRegularFile(dirlines[*sel_id]) ) {
                        windows.state = edit;
                        filenameEditing = dirlines[*sel_id];
                    }
                    else {
                        if (chdir(dirlines[*sel_id]) == 0)
                            (*sel_id) = 0;
                        getDirectoryInfo(size);
                    }
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

int isRegularFile(char* filename) {
    struct stat file_stat;
    stat(filename, &file_stat);
    return S_ISREG(file_stat.st_mode);
}

void getDirectoryInfo(int *size) {
    int i = 0;
    DIR *dir;
    struct dirent *dp;

    dir = opendir(".");
    strcpy(dirlines[i++], "..");
    while((dp = readdir(dir)) != NULL)
        if (dp->d_name[0] != '.') {
            strcpy(dirlines[i++], dp->d_name);
            }
    qsort(&dirlines, i, MAXDIRWIDTH, compare);
    *size = i;
}

int compare(const void* pa, const void *pb) {
    return strcmp(pa, pb);
}

void initialiseColors() {
    init_pair(normal, COLOR_WHITE, COLOR_BLACK);
    init_pair(highlight, COLOR_YELLOW, COLOR_BLACK);
    init_pair(panel, COLOR_BLUE, COLOR_BLACK);
}

void createNewWindow(int height, int width, int starty, int startx, int boxed, char* title, int state, windowData *wd) {
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    wd->window=local_win;
    wd->boxed=boxed;
    wd->titleBar=title;
    wd->state = state;
    wd->id = 0;
    wd->sel_id = 0;
    wd->size = 0;
}

void render() {

    drawDirectory(windows.directory);
    drawWindow(windows.directory);

    drawWindow(windows.editor);
    drawWindow(windows.toppanel);
    drawWindow(windows.bottompanel);
}

void drawDirectory(windowData *wd) {
    int i = 0;
    WINDOW* w = wd->window;

    for(; i < wd->size; i++) {
        if (wd->sel_id == i)
            wattron(w, A_REVERSE);
        mvwprintw(w, i+1, 1, dirlines[i]);
        wattroff(w, A_REVERSE);
    }
}

void drawWindow(windowData *wd) {
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
