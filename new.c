#include <usb.h>
#include <stdio.h>
#include <unistd.h>


struct usb_dev_handle*  get_rocker_launcher_dev_handle();
void test(usb_dev_handle *launcher, int sig);

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
  
  /* test(launcher, 16); */
  /* usleep(500000); */
  /* test(launcher, 1);
  
  test(launcher, 4);
  usleep(100000);
  test(launcher, 4);

  test(launcher, 4);
  usleep(100000);
  test(launcher, 4);
 */

  char * key;
  key = malloc(sizeof(char) * 2);
  while (1) {
    key = fgets(key, 2, stdin);
    printf("got key %s\n", key);
  }

  /*
    for (int i = 0; i < 10; i++) {
    test(launcher, i);
    usleep(100000);
    test(launcher, 1);
    } 
  */

  printf("-- Releasing device\n");
  usb_release_interface(launcher, 0);
  return(0);
}

void test(usb_dev_handle *launcher, int sig)
{
  char msg[8];
  for (int i = 0; i < 8; i++) {
    msg[i] = 0x0;
  }

  usb_control_msg(launcher, 0x21, 0x9, 0x200, 1, msg, 8, 1000);
  msg[0] = sig;
  
  printf("-- sending sig %d\n", sig);
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
	printf("-- Found device with vid %d\n", dev->descriptor.idVendor);
	return usb_open(dev);
      }
    }
  }

  return NULL;
}
