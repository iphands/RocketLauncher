#include <usb.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <signal.h>

#define FALSE 0
#define TRUE 1

#define DOWN 1
#define UP 2
#define LEFT 4
#define RIGHT 8
#define FIRE 16
#define STOP 32

int loop_until_ret(usb_dev_handle *launcher, unsigned int timeout, unsigned int sig);
char can_move_rl(usb_dev_handle *launcher, char direction);
int get_sig(int key);
char* get_mesg(char *mesg, int key);
void do_cli(usb_dev_handle *launcher, char direction, int timeout);
char do_arg_check(int argc, char ** argv);
void do_tui(usb_dev_handle *launcher);
void disarm_rl(usb_dev_handle *launcher);
void stop_rl(usb_dev_handle *launcher);
void move_rl(usb_dev_handle *launcher, int sig);
void send_msg(usb_dev_handle *launcher, int sig);
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
  
  char dir_char = 0;

  switch (direction) {
  case 'u':
    dir_char = UP;
    break;
  case 'd':
    dir_char = DOWN;
    break;
  case 'l':
    dir_char = LEFT;
    break;
  case 'r':
    dir_char = RIGHT;
    break;
  case 'f':
    dir_char = FIRE;
    break;
  case 's':
    dir_char = STOP;
    break;
  default:
    return;
    break;
  }

  if (can_move_rl(launcher, dir_char) == FALSE) {
    printf("!! Unable to move %c! Already at the max.\n", direction);
    return;
  }

  switch (direction) {
  case 'u':
    send_msg(launcher, UP);
    break;
  case 'd':
    send_msg(launcher, DOWN);
    break;
  case 'l':
    send_msg(launcher, LEFT);
    break;
  case 'r':
    send_msg(launcher, RIGHT);
    break;
  case 'f':
    send_msg(launcher, FIRE);
    int ret = loop_until_ret(launcher, 5000, FIRE);
    
    if (ret) {
      printf("Done firing!");
    } else {
      printf("!! Hit firing timeout!");
    }
    
    disarm_rl(launcher);
    stop_rl(launcher);
    return;
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

  WINDOW *info_win = newwin(15, COLS - 8, 0, 8);
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
  mvwprintw(info_win, 12, 1, "Thanks to Dan Krasinski for figuring out the usb command codes,");
  mvwprintw(info_win, 13, 1, "and writing some example code.");
  wrefresh(info_win);

  WINDOW *debug_win = newwin(LINES - 15, COLS, 15, 8);
  box(debug_win, '|', '-');
  mvwprintw(debug_win, 0, 1, "device-debug");
  wrefresh(debug_win);

  curs_set(0);

  int key = 0;
  int log_line_pos = 1;
  int debug_line_pos = 1;
  char *mesg = "-----";  

  while (key != 3) {
    key = getch();

    mesg = get_mesg(mesg, key);

    for (int i = 2; i < 8; i++) {
      mvwprintw(info_win, i, 1, " ");
    }

    if (debug_line_pos >= (LINES - 15 - 1)) {
      debug_line_pos = 1;
      wclear(debug_win);
      box(debug_win, '|', '-');
      mvwprintw(debug_win, 0, 1, "device-debug");
    }
    
    if (can_move_rl(launcher, get_sig(key)) == FALSE) {      
      mvwprintw(debug_win, debug_line_pos++, 1, "Unable to move %s! Already at the max. (%d)", mesg, get_sig(key));

      wrefresh(debug_win);
      curs_set(0);
      continue;
    }

    switch (key) {
    case KEY_UP:
      mvwprintw(info_win, 2, 1, "*");
      move_rl(launcher, UP);
      break;
    case KEY_DOWN:
      mvwprintw(info_win, 3, 1, "*");
      move_rl(launcher, DOWN);
      break;
    case KEY_LEFT:
      mvwprintw(info_win, 4, 1, "*");
      move_rl(launcher, LEFT);
      break;
    case KEY_RIGHT:
      mvwprintw(info_win, 5, 1, "*");
      move_rl(launcher, RIGHT);
      break;
    case 32:
      mvwprintw(info_win, 6, 1, "*");
      send_msg(launcher, FIRE);
      int ret = loop_until_ret(launcher, 5000, FIRE);
            
      if (ret) {
	mvwprintw(debug_win, debug_line_pos++, 1, "Done firing!");
      } else {
	mvwprintw(debug_win, debug_line_pos++, 1, "!! Hit firing timeout!");
      }
      
      wrefresh(debug_win);
      disarm_rl(launcher);
      stop_rl(launcher);
      break;
    case 27:
      mvwprintw(info_win, 7, 1, "*");
      disarm_rl(launcher);
      stop_rl(launcher);
      break;
    }

    if (log_line_pos >= LINES - 1) {
      log_line_pos = 1;
      wclear(log_win);
      box(log_win, '|', '-');
      mvwprintw(log_win, 0, 1, "log");
    }

    mvwprintw(log_win, log_line_pos++, 1, mesg);
    curs_set(0);
    wrefresh(log_win);
    wrefresh(info_win);
  }

  endwin();
  return;
}

char* get_mesg(char *mesg, int key)
{
  switch (key) {
  case KEY_UP:
    mesg = "up";
    break;
  case KEY_DOWN:
    mesg = "down";
    break;
  case KEY_LEFT:
    mesg = "left";
    break;
  case KEY_RIGHT:
    mesg = "right";
    break;
  case 32:
    mesg = "FIRE";
    break;
  case 27:
    mesg = "stop";
    break;
  }

  return mesg;
}

int get_sig(int key)
{
  switch (key) {
  case KEY_UP:
    return(UP);
    break;
  case KEY_DOWN:
    return(DOWN);
    break;
  case KEY_LEFT:
    return(LEFT);
    break;
  case KEY_RIGHT:
    return(RIGHT);
    break;
  case 32:
    return(FIRE);
    break;
  case 27:
    return(STOP);
    break;
  }

  return(0);
}

int loop_until_ret(usb_dev_handle *launcher, unsigned int timeout, unsigned int sig) 
{
  char rec_buf[1];
  char msg[8];
  memset(rec_buf, 0x0, 1);
  memset(msg, 0x0, 8);
  msg[0] = 0x40;

  char ret = FALSE;
  for (unsigned int i = 0; i < timeout; i += 100) {
    ret = can_move_rl(launcher, sig);
    send_msg(launcher, sig);
    if (ret == FALSE) {
      break;
    }
    usleep(1000 * 100);
  }
  
  for (unsigned int i = 0; i < timeout; i += 100) {
    ret = can_move_rl(launcher, sig);
    send_msg(launcher, sig);
    if (ret == TRUE) {
      return(TRUE);
    }
    usleep(1000 * 100);
  }
  
  return(FALSE);
}

char can_move_rl(usb_dev_handle *launcher, char direction)
{
  char rec_buf[1];
  char msg[8];
  memset(msg, 0x0, 8);
  msg[0] = 0x40;
  
  usb_control_msg(launcher, 0x21, 0x9, 0x200, 0, msg, 8, 100);
  usb_interrupt_read(launcher, 0x81, rec_buf,  1, 250);

  for (int mask = 32; mask > 0; mask >>= 1) {    
    if ((((int)rec_buf[0]) &  mask) == direction) {
      return(FALSE);
    }
  }
  
  return(TRUE);
}

void disarm_rl(usb_dev_handle *launcher)
{
  send_msg(launcher, 1);
  send_msg(launcher, 2);
  return;
}

void stop_rl(usb_dev_handle *launcher)
{
  send_msg(launcher, 32);
  return;
}

void move_rl(usb_dev_handle *launcher, int sig)
{
  send_msg(launcher, sig);
  usleep(32 * 1000);
  stop_rl(launcher);
  return;
}

void send_msg(usb_dev_handle *launcher, int sig)
{
  char msg[8];

  /*
    unsigned char rec_buf[1];
    memset(rec_buf, 0x0, 1);
    memset(msg, 0x0, 8);
    msg[0] = 0x40;
    usb_control_msg(launcher, 0x21, 0x9, 0x200, 0, msg, 8, 5);

    int ret = usb_interrupt_read(launcher, 0x81, rec_buf,  1, 5);
    if (ret == 1) {
    if (rec_buf > 0) {
    return;
    }
    }
  */

  memset(msg, 0, 8);
  msg[0] = sig;
  usb_control_msg(launcher, 0x21, 0x9, 0x200, 0, msg, 8, 5);

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
