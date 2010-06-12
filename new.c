#include <usb.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>

struct usb_dev_handle*  get_rocker_launcher_dev_handle();
void test(usb_dev_handle *launcher, int sig);
void move_rl(usb_dev_handle *launcher, int sig);

int main() 
{
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
  
  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();

  int key;
  while (key != 3) {
    key = getch();

    switch (key) {
    case KEY_UP:
      printw("up\n");
      move_rl(launcher, 1);
      break;
    case KEY_DOWN:
      printw("down\n");
      move_rl(launcher, 2);
      break;
    case KEY_LEFT:
      printw("left\n");
      move_rl(launcher, 4);
      break;
    case KEY_RIGHT:
      printw("right\n");
      move_rl(launcher, 8);
      break;
    case 32:
      printw("FIRE\n");
      test(launcher, 16);
      break;
    case 27:
      stop_rl(launcher);
      break;
    }
    //printw("%d\n", key);
  }

  endwin();
  
  printf("-- Releasing device\n");
  usb_release_interface(launcher, 0);
  return(0);
}

void stop_rl(usb_dev_handle *launcher)
{
  test(launcher, 1);
  test(launcher, 2);
}

void move_rl(usb_dev_handle *launcher, int sig)
{
  test(launcher, sig);
  usleep(30000);
  test(launcher, 1);
}

void test(usb_dev_handle *launcher, int sig)
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
