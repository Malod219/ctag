#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#define CURSOR_INVIS    0
#define DIRECTORYLINES 1000
#define MAXDIRWIDTH 256

typedef struct windowData {
    WINDOW* window;
    int     boxed;
    char*   titleBar;
    int     state;
    int     id;
    int     sel_id;
    int     size;
} windowData;

void createNewWindow(int height, int width, int starty, int startx, int boxed, char* title, int state, windowData *wd);
void render(void);
void drawWindow(windowData *wd);
void initialiseColors(void);
void getDirectoryInfo(int *size);
void drawDirectory(windowData *wd);
int isRegularFile(char* filename);
int compare(const void* pa, const void *pb);

/* key functions */
void kbf_quit(void);
void kbf_enter(void);
void kbf_tab(void);
void kbf_up(void);
void kbf_down(void);

char dirlines[DIRECTORYLINES][MAXDIRWIDTH]; /* Consider a malloc approach, so it extends to any directory with size>1000 */
char *filenameEditing;

struct keyData {
    int     key;
    void    (*kfunc)(void);
};

struct windows{
    windowData* directory;
    windowData* editor;
    windowData* toppanel;
    windowData* bottompanel;
    int         state;
} windows;

/* based on ASCII table. We used Hex or Ncurses-provided values */
enum keys {
    kb_quit = 0x71,
    kb_tab = 0x09,
    kb_enter = 0x0a,
    kb_down = KEY_DOWN,
    kb_up = KEY_UP
};

const struct keyData keyTable[] = {
    /* key int,     func pointer */
    {kb_quit,       NULL}, /* For loop assumes it is first element. Func pointer unused */
    {kb_enter,      kbf_enter},
    {kb_tab,        kbf_tab},
    {kb_up,         kbf_up},
    {kb_down,       kbf_down},
};

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


int main( int argc, char *argv[]) {
    int ch;
    int row, col;
    char * toptext = "Made by RisingThumb          https://risingthumb.xyz ";
    char * bottomtext = "TAB switch menu    Q to quit";
    int i;
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
    for(render(); (ch = tolower(getch())) != keyTable[0].key; render()){
        for (i = 1; i < (sizeof(keyTable) / sizeof(struct keyData)); i++) {
            if (ch == keyTable[i].key) {
                (*keyTable[i].kfunc)();
            }
        }
    }
    endwin();
    return 0;
}

void kbf_enter() {
    if (windows.state == dir) {
        int* sel_id = &windows.directory->sel_id;
        int* size = &windows.directory->size;
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
    }
    return;
}

void kbf_tab() {
    if (windows.state == dir)
        windows.state = edit;
    else if ((windows.state = edit))
        windows.state = dir;
    return;
}

void kbf_up() {
    if (windows.state == dir) {
        int* sel_id = &windows.directory->sel_id;
        if (*sel_id > 0)
            (*sel_id) -= 1;
    }
    return;
}

void kbf_down() {
    if (windows.state == dir) {
        int* sel_id = &windows.directory->sel_id;
        int* size = &windows.directory->size;
        if ((*sel_id) < (*size) - 1)
            (*sel_id) += 1;
    }
    return;
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
        if (dp->d_name[0] != '.')
            strcpy(dirlines[i++], dp->d_name);
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
    wd->window = local_win;
    wd->boxed = boxed;
    wd->titleBar = title;
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
        mvwprintw(w, i + 1, 1, dirlines[i]);
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
