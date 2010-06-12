#include <usb.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>

#define DOWN 1
#define UP 2
#define LEFT 4
#define RIGHT 8
#define FIRE 16
#define STOP 32

void do_cli(usb_dev_handle *launcher, char direction, int timeout);
char do_arg_check(int argc, char ** argv);
void do_tui(usb_dev_handle *launcher);
void disarm_rl(usb_dev_handle *launcher);
void stop_rl(usb_dev_handle *launcher);
void move_rl(usb_dev_handle *launcher, int sig);
void send_sig(usb_dev_handle *launcher, int sig);
struct usb_dev_handle* get_rocker_launcher_dev_handle();

int main(int argc, char ** argv)
{

  if (argc > 1) {
    if(do_arg_check(argc, argv)) {
      printf("Usage: %s [COMMAND TIMEOUT]\n\nCommands are:\n\tup\n\tdown\n\tleft\n\tright\n\tfire\n\tstop\n\n", argv[0]);
      printf("Timeout is a number of millisecond from 0 - 10000\n\n");
      printf("Or use no args for an interactive version. In the interactive version the controls are:");
      printf("\n\tup arrow\n\tdown arrow\n\tleft arrow\n\tright arrow\n\tspace bar (to fire)\n\tescape (to stop)\n\tctrl+c (to quit)\n\n");
      return(1);
    }
  }

  struct usb_dev_handle *launcher = get_rocker_launcher_dev_handle();
  
  if (launcher == NULL) {
    printf("!! Unable to find rocket launcher device!\nExiting...\n");
    return(1);
  }
  
  printf("-- Opened device\n");
  printf("-- Detaching driver from device\n");
  usb_detach_kernel_driver_np(launcher, 0);
  printf("-- Claiming device\n");
  if (usb_claim_interface(launcher, 0) != 0) {
    printf("!! Unable to claim rocket launcher device!\nExiting...\n");
    return(1);
  }
  
  if (argc > 1) {
    char direction = argv[1][0];
    int timeout = (atoi(argv[2]) * 1000);
    do_cli(launcher, direction, timeout);
  } else {
    do_tui(launcher);
  }

  disarm_rl(launcher);
  stop_rl(launcher);

  printf("-- Releasing device\n");
  usb_release_interface(launcher, 0);
  return(0);
}

void do_cli(usb_dev_handle *launcher, char direction, int timeout)
{
  switch (direction) {
  case 'u':
    send_sig(launcher, UP);
    break;
  case 'd':
    send_sig(launcher, DOWN);
    break;
  case 'l':
    send_sig(launcher, LEFT);
    break;
  case 'r':
    send_sig(launcher, RIGHT);
    break;
  case 'f':
    send_sig(launcher, FIRE);
    break;
  case 's':
    stop_rl(launcher);
    break;
  default:
    return;
    break;
  }
  
  usleep(timeout);
  disarm_rl(launcher);
  stop_rl(launcher);
  return;  
}

char do_arg_check(int argc, char ** argv)
{
  if (argc != 3) {
    return(1);
  }

  switch (argv[1][0]) {
  case 'u':
    break;
  case 'd':
    break;
  case 'l':
    break;
  case 'r':
    break;
  case 'f':
    break;
  case 's':
    break;
  default:
    return(1);
    break;
  }
  
  int time = atoi(argv[2]);
  if ((time > 10000) || (time < 0)) {
    return(1);
  }

  return(0);
}

void do_tui(usb_dev_handle *launcher)
{
  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();

  refresh();
  WINDOW *log_win = newwin(LINES, 7, 0, 0);
  box(log_win, '|', '-');
  mvwprintw(log_win, 0, 1, "log");
  wrefresh(log_win);

  WINDOW *info_win = newwin(LINES, COLS - 8, 0, 8);
  box(info_win, '|', '-');
  mvwprintw(info_win, 0, 1, "info");
  mvwprintw(info_win, 1, 1, "Controls:");
  mvwprintw(info_win, 2, 1, "\tup arrow");
  mvwprintw(info_win, 3, 1, "\tdown arrow");
  mvwprintw(info_win, 4, 1, "\tleft arrow");
  mvwprintw(info_win, 5, 1, "\tright arrow");
  mvwprintw(info_win, 6, 1, "\tspace bar (fire)");
  mvwprintw(info_win, 7, 1, "\tescape (stop)");
  mvwprintw(info_win, 8, 1, "\tctrl+c (quit)");
  mvwprintw(info_win, 10, 1, "Written by Ian Page Hands");
  mvwprintw(info_win, 12, 1, "Thanks to Dan Krasinski for figuring");
  mvwprintw(info_win, 13, 1, "out the usb command codes, and writing");
  mvwprintw(info_win, 14, 1, "some example code.");
  wrefresh(info_win);

  curs_set(0);

  int key, line_pos = 1;
  char *mesg = "-----";
  while (key != 3) {
    key = getch();

    for (int i = 2; i < 8; i++) {
      mvwprintw(info_win, i, 1, " ");
    }

    switch (key) {
    case KEY_UP:
      mesg = "up";
      mvwprintw(info_win, 2, 1, "*");
      move_rl(launcher, UP);
      break;
    case KEY_DOWN:
      mesg = "down";
      mvwprintw(info_win, 3, 1, "*");
      move_rl(launcher, DOWN);
      break;
    case KEY_LEFT:
      mesg = "left";
      mvwprintw(info_win, 4, 1, "*");
      //wprintw(log_win, "left\n");
      move_rl(launcher, LEFT);
      break;
    case KEY_RIGHT:
      mesg = "right";
      mvwprintw(info_win, 5, 1, "*");
      //wprintw(log_win, "right\n");
      move_rl(launcher, RIGHT);
      break;
    case 32:
      mesg = "FIRE";
      mvwprintw(info_win, 6, 1, "*");
      //wprintw(log_win, "FIRE\n");
      send_sig(launcher, FIRE);
      break;
    case 27:
      //wprintw(log_win, "stop\n");
      mesg = "stop";
      mvwprintw(info_win, 7, 1, "*");
      disarm_rl(launcher);
      stop_rl(launcher);
      break;
    }

    if (line_pos >= LINES - 1) {
      line_pos = 1;
      wclear(log_win);
      box(log_win, '|', '-');
      mvwprintw(log_win, 0, 1, "log");
    }

    mvwprintw(log_win, line_pos++, 1, mesg);
    curs_set(0);
    wrefresh(log_win);
    wrefresh(info_win);
  }

  endwin();
  return;
}

void disarm_rl(usb_dev_handle *launcher)
{
  send_sig(launcher, 1);
  return;
}

void stop_rl(usb_dev_handle *launcher)
{
  send_sig(launcher, 32);
  return;
}

void move_rl(usb_dev_handle *launcher, int sig)
{
  send_sig(launcher, sig);
  usleep(30000);
  stop_rl(launcher);
  return;
}

void send_sig(usb_dev_handle *launcher, int sig)
{
  char msg[8];
  for (int i = 0; i < 8; i++) {
    msg[i] = 0x0;
  }

  usb_control_msg(launcher, 0x21, 0x9, 0x200, 1, msg, 8, 1000);
  msg[0] = sig;
  
  //printf("-- sending sig %d\n", sig);
  usb_control_msg(launcher, 0x21, 0x9, 0x200, 0, msg, 8, 1000);
  usb_control_msg(launcher, 0x21, 0x9, 0x200, 1, msg, 8, 1000);

  return;
}

struct usb_dev_handle* get_rocker_launcher_dev_handle()
{
  usb_init();
  usb_find_busses();
  usb_find_devices();
  
  struct usb_bus *busses = usb_get_busses();
  struct usb_bus *bus;

  for (bus = busses; bus; bus = bus->next) {
    struct usb_device *dev;
    for (dev = bus->devices; dev; dev = dev->next) {

      if ((dev->descriptor.idVendor == 2689) || (dev->descriptor.idVendor == 6465)) {
	printf("-- Found device with vendor id %d\n", dev->descriptor.idVendor);
	return usb_open(dev);
      }
    }
  }

  return NULL;
}
