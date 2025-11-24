#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <ncurses.h>
#include <sys/types.h>
// git@github.com:jstmaxlol/kat
#include "kat.h"

// gim - the g interactive mode
// copyleft (Unlicense) - czjstmax
// for i am a funny lad, gim_version is binary

// Global variables
const wchar_t *welcome_message_pt1 = L"おはよう！";
const wchar_t *welcome_message_pt2 = L"へようこそ！";
const wchar_t *version_message = L"のばーじょんは「00001」だ";
//
int curr_line = 0;
char buff[1024];
//
char cmd_sym[64] = ">";

// Global matches
const char *no_vars[] = {"n", "N", "no", NULL};
const char *yes_vars[] = {"y", "Y", "ye", "yes", NULL};
const char *quit_vars[] = {":q", ":q!", ":qa", ":qa!", ":x", ":wq", ":wqa", ":wqa!", NULL};
const char *exit_vars[] = {":ex", ":Ex", ":exi", ":exit", NULL};
const char *help_vars[] = {":h", ":H", ":he", ":hel", ":help", ":usage", "-h", "--help", NULL};
const char *edit_vars[] = {":e", ":E", ":ed", ":edi", ":edit", ":edito", ":editor", NULL};
const char *clear_vars[] = {":c", ":C", ":cl", ":cle", ":clea", ":clear", "clear", "cls", "CLS", NULL};
const char *cd_vars[] = {":cd", ":Cd", ":cD", ":CD", ":change-dir", ":changedir", NULL};

// Helpers
int matches(const char *cmd, const char *list[]);
void ncurses_print_ansi_in_win(WINDOW *win, int start_row, int start_col, const char *str);
void usage();

int main(void) {
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    echo();
    start_color();
    use_default_colors();

    // ncurses color pairs
    init_pair(1, COLOR_RED, -1);
    init_pair(2, COLOR_WHITE, -1);
    init_pair(3, COLOR_RED, COLOR_WHITE);

    char *pwd = getenv("PWD");
    char *home = getenv("HOME");

    const char *rcfile = ".gimrc";
    char rcpath[64]; // based on the fact that LOGIN_NAME_MAX is 32 (with '\0')
    snprintf(rcpath, sizeof(rcpath), "%s/%s", home, rcfile);

    // Check config file stuff
    FILE *f = fopen(rcpath, "r");
    if (!f) {
        // If rc file does not exist, ask to create it
        attron(COLOR_PAIR(1));
        mvprintw(curr_line, 0, ":: no ~/.gimrc was found, do you want to create it now? (Y/n)");
        attroff(COLOR_PAIR(1));
        curr_line++;
        mvprintw(curr_line, 0, "?> ");
        curr_line++;
        wgetnstr(stdscr, buff, sizeof(buff) - 1);
        refresh();
        if (matches(buff, yes_vars) == 0 || strlen(buff) == 0) {
            mvprintw(curr_line, 0, ":: creating ~/.gimrc");
            curr_line++;
            int ret = kat.create(rcpath);
            if (ret == 1) {
                mvprintw(curr_line, 0, ":: an error occurred, couldn't create file");
                curr_line++;
            } else {
                mvprintw(curr_line, 0, ":: file created successfully");
                curr_line++;
            }
        } // elsewise just continue execution minding my business
        refresh();
    }
    // Preference buffers
    char cmd_buff[65];
    char default_editor[16];
    // Read preference into buffers and apply them to global variables
    if (kat.read("cmd", rcpath, cmd_buff, sizeof(buff))) {
        strcpy(cmd_sym, cmd_buff);
    }
    kat.read("editor", rcpath, default_editor, sizeof(buff)); // already read into default_editor

    clear();
    curr_line = 0;

    // Print welcome message + version message
    //                                              welcome message
    move(curr_line, 0);
    wprintw(stdscr, "%ls", welcome_message_pt1); // ohayou!
    attron(COLOR_PAIR(3));                       // enable color for gim
    printw("gim");                               // print gim
    attroff(COLOR_PAIR(3));                      // disable color for gim
    wprintw(stdscr, "%ls", welcome_message_pt2);
    curr_line++;
    //                                              version message
    move(curr_line, 0);
    attron(COLOR_PAIR(3));                       // enable color for gim
    printw("gim");                               // print gim
    attroff(COLOR_PAIR(3));                      // disable color for gim
    wprintw(stdscr, "%ls", version_message);
    curr_line++;

    refresh();

    while (true) {
        char cwd[4096];
        pwd = getenv("PWD");
        if (strcmp(pwd, home) == 0) {
            // at ~
            snprintf(cwd, sizeof(cwd), "~%s", pwd); // genius idea right here boys
        } else {
            strcpy(cwd, pwd);
        }
        // Print prompt
        attron(COLOR_PAIR(2)); // white
        mvprintw(curr_line, 0, "(");
        attroff(COLOR_PAIR(2));
        attron(COLOR_PAIR(1)); // red
        printw("gim");
        attroff(COLOR_PAIR(1));
        attron(COLOR_PAIR(2)); // white
        printw("@");
        attroff(COLOR_PAIR(2));
        attron(COLOR_PAIR(1)); // red
        printw("%s", cwd);
        attroff(COLOR_PAIR(1));
        printw(")%s ", cmd_sym);
        // Read to buffer
        wgetnstr(stdscr, buff, sizeof(buff) - 1);
        curr_line++;

        if (matches(buff, quit_vars) == 0 || matches(buff, exit_vars) == 0) {
            nocbreak();
            endwin();
            return 0;
        } else if (matches(buff, edit_vars) == 0) {
            if (system(default_editor)) {
                curr_line++;
                mvprintw(curr_line, 0, ":: could not open default editor (%s)", default_editor);
            } else {
                clear();
                curr_line = 0;
            }
        } else if (strcmp(buff, ":evi") == 0) {
            if (system("vi")) {
                curr_line++;
                mvprintw(curr_line, 0, ":: could not open vi");
            } else {
                clear();
                curr_line = 0;
            }
        } else if (strcmp(buff, ":evim") == 0) {
            if (system("vim")) {
                curr_line++;
                mvprintw(curr_line, 0, ":: could not open vim");
            } else {
                clear();
                curr_line = 0;
            }
        } else if (strcmp(buff, ":envim") == 0) {
            if (system("nvim")) {
                curr_line++;
                mvprintw(curr_line, 0, ":: could not open neovim");
            } else {
                clear();
                curr_line = 0;
            }
        } else if (strcmp(buff, ":ec") == 0) {
            mvprintw(curr_line, 0, "?> ");
            curr_line++;
            wgetnstr(stdscr, buff, sizeof(buff) - 1);
            if (system(buff)) {
                mvprintw(curr_line, 0, ":: could not open %s", buff);
            } else {
                clear();
                curr_line = 0;
            }
        } else if (strcmp(buff, ":") == 0) {
            mvprintw(curr_line, 0, ":: you need to provide at least a command");
            curr_line++;
        } else if (matches(buff, clear_vars) == 0) {
            clear();
            curr_line = 0;
        } else if (matches(buff, help_vars) == 0) {
            usage();
        } else if (matches(buff, cd_vars) == 0) {
            char cmd[4096];
            mvprintw(curr_line, 0, "?> ");
            curr_line++;
            wgetnstr(stdscr, buff, sizeof(buff) - 1);

            snprintf(cmd, sizeof(cmd), "cd %s", buff);
            if (system(cmd)) {
                curr_line++;
            }
        } else if (strlen(buff) > 0) {
            char cmd[2048]; // huge buffer so we are 420% sure not to bfof
            snprintf(cmd, sizeof(cmd), "g %s", buff);

            FILE *pipe = popen(cmd, "r");
            if (!pipe) {
                mvprintw(curr_line, 0, ":: there has been an error while running");
                curr_line++;
                mvprintw(curr_line, 0, ":: \"%s\"", cmd);
                curr_line++;
            } else {
                refresh();
                doupdate();
            }
            WINDOW *win = newwin(curr_line, COLS, 1, 0);
            //scrollok(win, TRUE);
            char line[384];
            int row = 0;

            while (fgets(line, sizeof(line), pipe)) {
                if (row == curr_line) {
                    wscrl(win, 1); // scroll up if we reach the window bottom
                    row--;
                }
                ncurses_print_ansi_in_win(win, row++, 0, line);
                wrefresh(win);
            }

            pclose(pipe);
        }

        refresh();
    }

    return 0;
}

int matches(const char *cmd, const char *list[]) {
    for (int i = 0; list[i] != NULL; i++) {
        if (strcmp(cmd, list[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

void ncurses_print_ansi_in_win(WINDOW *win, int start_row, int start_col, const char *str) {
    // this little parser took me 2 hours to write
    // maybe i am stupid, maybe i am not
    // who gives a fuck if it works though? :)
    // also, it's 3:33:33 in the night right now TwT
    int row = start_row;
    int col = start_col;
    const char *p = str;

    while (*p) {
        if (*p == '\033' && *(p+1) == '[') {
            p += 2;
            int code = 0;
            while (*p >= '0' && *p <= '9') {
                code = code * 10 + (*p - '0');
                p++;
            }
            if (*p == 'm') {
                // handle code
                switch (code) {
                    case 0:
                        wattroff(win, A_BOLD | COLOR_PAIR(1) | COLOR_PAIR(2) | COLOR_PAIR(3));
                        wattroff(win, A_REVERSE);
                        // reset attributes
                        break;
                    case 31: // red fg
                        wattron(win, COLOR_PAIR(1));
                        break;
                    case 33: // white fg
                        wattron(win, COLOR_PAIR(2));
                        break;
                }
                p++;
                continue;
            }
        }
        if (*p == '\n') {
            row++;
            col = start_col;
            p++;
            continue;
        }
        mvwaddch(win, row, col++, *p++);
    }
    wrefresh(win);
}

void usage() {
    printw("syntax:");
    curr_line++;

    move(curr_line, 0);
    printw("(gim)> :[command]");
    curr_line++;

    move(curr_line, 0);
    printw("(gim)> [g parameters]");
    curr_line++;

    move(curr_line, 0);
    printw("[command]s");
    curr_line++;

    move(curr_line, 0);
    printw(":e - opens default text editor, specifiable in .gimrc with \'editor=execname\'");
    curr_line++;

    move(curr_line, 0);
    printw(":e[n]vi[m] - opens [n]vi[m] (vi/vim/nvim)");
    curr_line++;

    move(curr_line, 0);
    printw(":c|C[l[e[a[r]]]] - clears the virtual screen");
    curr_line++;

    move(curr_line, 0);
    printw(":cd|Cd|cD|CD|changedir|change-dir - changes working directory");
    curr_line++;
}
