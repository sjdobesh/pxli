#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <ncurses.h>


/* hardcoded strings */
const char* title = "------------------PXLi------------------";
const char* menu[] = {
"pxl sort options-----------------------",
"filename        : %s", // f
"start threshold : %s", // t
"stop threshold  : %s", // t
"min length      : %s", // l
"max length      : %s", // L
"random start    : %s", // r
"random stop     : %s"  // R
};

/* helpers */
int col_center (const char* s) {
  return COLS / 2 - (strnlen(s, COLS) / 2);
}
void center_text_line(const char *s, int line) {
  mvaddstr(line, col_center(s), s);
}
void wstyle(WINDOW* win) {
  box((win ? win : stdscr), 0, 0);
  center_text_line(title, 2);
}
void style() {
  wstyle(NULL);
}

/* globals */
int selection = 0;

// test vars
char file[20] = "png/Catgun.png";
float start_t = 0;
float stop_t = 1;
int min_l = 1;
int max_l = 1;
int start_r = 0;
int stop_r = 0;


// READ IN DATA

// lets just convert to string first
char str[7][50] = {0};
void read_data() {
  for (int i = 0; i < 7; i++) {
    snprintf(str[0], sizeof(str[0]), "%-19s", file);
    snprintf(str[1], sizeof(str[1]), "%.4f", start_t);
    snprintf(str[2], sizeof(str[2]), "%.4f", stop_t);
    snprintf(str[3], sizeof(str[3]), "%d", min_l);
    snprintf(str[4], sizeof(str[4]), "%d", max_l);
    snprintf(str[5], sizeof(str[5]), "%d", start_r);
    snprintf(str[6], sizeof(str[6]), "%d", stop_r);
  }
}

#define print(l, c, m, o) \
  mvprintw(l, c, m, o);
void print_menu(int line_start, int col_start, int selected){
  read_data();
  mvaddstr(line_start++, col_start, menu[0]);
  int m, s;
  for (m = 1, s = 0; m < 8; m++, s++) {
    if (s == selected) {
      attron(A_BOLD | A_UNDERLINE);
    }
    print(line_start++, col_start, menu[m], str[s]);
    if (s == selected) {
      attroff(A_BOLD | A_UNDERLINE);
    }
  }
}


void handle_input(int ch, int l, int c) {

  switch (ch) {
    case 'j':
      if (selection < 6)
        selection++;
      break;
    case 'k':
      if (selection > 0)
        selection--;
      break;
    case 'l':
      switch (selection) {
        case 1:
          start_t += 0.01;
          break;
        case 2:
          stop_t += 0.01;
          break;
        case 3:
          min_l += 1;
          break;
        case 4:
          max_l += 1;
          break;
        case 5:
          start_r += 1;
          break;
        case 6:
          stop_r += 1;
          break;
      }
      break;
    case 'h':
      switch (selection) {
        case 1:
          if (start_t > 0)
            start_t -= 0.01;
          break;
        case 2:
          if (stop_t > 0)
          stop_t -= 0.01;
          break;
        case 3:
          if (min_l > 0)
            min_l -= 1;
          break;
        case 4:
          if (max_l > 0)
            max_l -= 1;
          break;
        case 5:
          if (start_r > 0)
            start_r -= 1;
          break;
        case 6:
          if (stop_r > 0)
            stop_r -= 1;
          break;
      }
      break;
    case '\n':
      if (selection == 0) {
        // read into file;
        int filename_i = 0;
        memset(file, 0, sizeof(file));
        while ((ch = getch()) != '\n' && filename_i < sizeof(file) - 1) {
          file[filename_i++] = (char)ch;
          print_menu(l, c, selection);
        }
        file[filename_i] = '\0';
      }
      break;
  }
}

int main() {
  WINDOW* win;
  int ch;

  if ((win = initscr()) == NULL) {
    fprintf(stderr, "error in initscr (ncurses)\n");
    return -1;
  }
  raw();
  keypad(stdscr, TRUE);
  noecho();
  curs_set(0);
  style();
  refresh();


  int l = LINES / 3;
  int c = col_center(title);
  while (ch = getch()) {
    if (ch == 'q') {
      clear();
      center_text_line("bye!", l);
      mvprintw(l+10, c, "char entered   : %c", ch);
      style();
      refresh();
      break;
    }
    // change selection
    handle_input(ch, l, c);
    mvprintw(l+10, c, "-debuginfo-");
    mvprintw(l+11, c, "char entered   : %c", ch);
    mvprintw(l+12, c, "menu item selected: %d", selection);
    // print menu
    print_menu(l, c, selection);
    // const char* fmt = "filename: %s";
    // mvprintw(++l, col_center(fmt), fmt, file);
    style();
    refresh();
    //  run program
    //
    // const char* execstr = "./bin/pxl -t 0.01 -T 0.4 -m 20 -f png/sortfuck.png >/dev/null 2>&1";
      char* const args[] = {
        "./pxl",
        // "-f",
        // str[0],
        "-t",
        str[1],
        "-T",
        str[2],
        "-l",
        str[3],
        "-L",
        str[4],
        "-r",
        str[5],
        "-R",
        str[6],
        NULL
      };
      mvprintw(l+13, c, "argstring");
      for (int i = 0; i < 13; i++) {
        mvprintw(l+14+i, c, "%s", args[i]);
      }

    pid_t pid = fork();
    if (pid == -1) {
      return 1;
    } else if (pid == 0) {
      // child process
      execvp("./pxl", args);
      // if returns then failure
      return 1;
    } else {
      wait(NULL);
      mvprintw(l+13, c, "finished child");
    }
    //system("./bin/pxl -t 0.01 -T 0.4 -m 20 -f png/sortfuck.png >/dev/null 2>&1");

  }
  ch = getch();

  delwin(win);
  endwin();
  refresh();
  return 0;

}
