#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <ncurses.h>



/* hardcoded strings */
const char* title = "-------------------PXLi-------------------";
const char* menu[] = {
"pxl sort options------------------------",
"filename        :[%s]", // f str(0)
"start threshold : %19s", // t etc.
"stop threshold  : %19s", // t
"min length      : %19s", // l
"max length      : %19s", // L
"random start    : %19s", // r
"random stop     : %19s", // R
"sort direction  : %19s", // v
"auto render     : %19s",  // auto reload
"[----------------render----------------]"  // manual reload
};
/* easier indexes */
typedef enum menu_values {
  FILENAME,
  START_T,
  STOP_T,
  MIN_L,
  MAX_L,
  START_R,
  STOP_R,
  SORT_D,
  AUTO,
  MANUAL,
} menu_values;
const int menu_length = sizeof(menu) / sizeof(menu[0]);


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
void msleep(long msec) {
  struct timespec ts;
  if (msec < 0) return;
  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;
  nanosleep(&ts, &ts);
}

/* globals */
int selection = 0;

// starting vars
char file[20] = "png/goodbye.png";
float start_t = 0;
float stop_t = 1;
int min_l = 1;
int max_l = 100;
int start_r = 0;
int stop_r = 0;
int sort_v = 0;
int auto_s = 0;
int val_changed = 0;


// READ IN DATA

// lets just convert to string first
char str[9][50] = {0};
void read_data() {
  for (int i = 0; i < 9; i++) {
    snprintf(str[0], sizeof(str[0]), "%-19s", file);
    snprintf(str[1], sizeof(str[1]), "%.2f", start_t);
    snprintf(str[2], sizeof(str[2]), "%.2f", stop_t);
    snprintf(str[3], sizeof(str[3]), "%d", min_l);
    snprintf(str[4], sizeof(str[4]), "%d", max_l);
    snprintf(str[5], sizeof(str[5]), "%d", start_r);
    snprintf(str[6], sizeof(str[6]), "%d", stop_r);
    snprintf(str[7], sizeof(str[7]), "%s", (sort_v ? "-v" : "  " ));
    snprintf(str[8], sizeof(str[8]), "%s", (auto_s ? "on " : "off" ));
  }
}

#define print(l, c, m, o) \
  mvprintw(l, c, m, o);

void print_menu(int line_start, int col_start, int selected){
  read_data();
  mvaddstr(line_start++, col_start, menu[0]);
  int m, s;
  for (m = 1; m < menu_length; m++) {
    if (m-1 == selected) {
      attron(A_UNDERLINE);
      refresh();
      print(line_start++, col_start, menu[m], str[m-1]);
      attroff(A_UNDERLINE);
      refresh();
    } else {
      attroff(A_UNDERLINE);
      refresh();
      print(line_start++, col_start, menu[m], str[m-1]);
    }
  }
}

void execute_pxl() {
  char* const args[] = {
    "./pxl",
    // "-f",
    // str[0],
    "-f",
    "png/goodbye.png",
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
    str[7],
    NULL
  };
  pid_t pid = fork();
  if (pid == -1) {
    return;
  } else if (pid == 0) {
    // child process
    // set up dev null
    int dn_fd = open("/dev/null", O_WRONLY);
    dup2(dn_fd, STDOUT_FILENO);
    dup2(dn_fd, STDERR_FILENO);
    close(dn_fd);

    execvp("./pxl", args);
    // if returns then failure
    return;
  } else {
    wait(NULL);
  }
}


void handle_input(int ch, int l, int c) {

  switch (ch) {

    case ERR :
      ch = '?';
      break;
    case 'j':
      if (selection < menu_length - 2) {
        selection++;
      }
      break;
    case 'k':
      if (selection > 0) {
        selection--;
      }
      break;
    case 'l':
      switch (selection) {
        case START_T:
          if (start_t < stop_t) {
            start_t += 0.01;
            val_changed = 1;
          }
          break;
        case STOP_T:
          stop_t += 0.01;
          val_changed = 1;
          break;
        case MIN_L:
          if (min_l < max_l) {
            min_l += 1;
            val_changed = 1;
          }
          break;
        case MAX_L:
          max_l += 1;
          val_changed = 1;
          break;
        case START_R:
          start_r += 1;
          val_changed = 1;
          break;
        case STOP_R:
          stop_r += 1;
          val_changed = 1;
          break;
      }
      break;
    case 'h':
      switch (selection) {
        case START_T:
          if (start_t > 0) {
            start_t -= 0.01;
            val_changed = 1;
          }
          break;
        case STOP_T:
          if (stop_t > 0 && stop_t > start_t) {
            stop_t -= 0.01;
            val_changed = 1;
          }
          break;
        case MIN_L:
          if (min_l > 0) {
            min_l -= 1;
            val_changed = 1;
          }
          break;
        case MAX_L:
          if (max_l > 0 && max_l > min_l) {
            max_l -= 1;
            val_changed = 1;
          }
          break;
        case START_R:
          if (start_r > 0){
            start_r -= 1;
            val_changed = 1;
          }
          break;
        case STOP_R:
          if (stop_r > 0) {
            stop_r -= 1;
            val_changed = 1;
          }
          break;
      }
      break;
    case '\n':
      switch(selection) {
        case FILENAME:
          // read into file;
          int filename_i = 0;
          memset(file, 0, sizeof(file));
          while ((ch = getch()) != '\n' && filename_i < sizeof(file) - 1) {
            file[filename_i++] = (char)ch;
            print_menu(l, c, selection);
          }
          file[filename_i] = '\0';
          val_changed = 1;
          break;
        case SORT_D:
          sort_v = (sort_v ? 0 : 1); // swap
          snprintf(str[7], sizeof(str[7]), "%s", (sort_v ? "-v" : "  " ));
          val_changed = 1;
          break;
        case AUTO:
          auto_s = (auto_s ? 0 : 1);
          val_changed = 1;
          break;
        case 9:
          val_changed = 0;
          execute_pxl();
          break;
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
  int l = LINES / 3;
  int c = col_center(title);
  raw();
  keypad(stdscr, TRUE);
  noecho();
  nodelay(stdscr, TRUE);
  curs_set(0);
  style();
  print_menu(l, c, selection);
  refresh();

  while (ch = getch()) {
    if (ch == 'q') {
      center_text_line("bye!", l);
      mvprintw(l+10, c, "char entered   : %c", ch);
      style();
      refresh();
      break;
    }
    // change selection
    handle_input(ch, l, c);

    print_menu(l, c, selection);
    style();
    refresh();

    if (auto_s && val_changed) {
      execute_pxl();
      val_changed = 0;
      //msleep(40);
    }

    // debug prints
    // mvprintw(l+12, c, "-debuginfo-");
    // mvprintw(l+13, c, "char entered   : %c", ch);
    // mvprintw(l+14, c, "menu item selected: %d", selection);



    // const char* fmt = "filename: %s";
    // mvprintw(++l, col_center(fmt), fmt, file);
    //  run program
    //
    // const char* execstr = "./bin/pxl -t 0.01 -T 0.4 -m 20 -f png/sortfuck.png >/dev/null 2>&1";
    //system("./bin/pxl -t 0.01 -T 0.4 -m 20 -f png/sortfuck.png >/dev/null 2>&1");

  }
  delwin(win);
  endwin();
  refresh();
  return 0;

}
