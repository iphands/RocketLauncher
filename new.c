#include <usb.h>
#include <stdio.h>

struct usb_dev_handle*  get_rocker_launcher_dev_handle();

int main() 
{
  struct usb_dev_handle *launcher = get_rocker_launcher_dev_handle();
  
  if (launcher == NULL) {
    printf("!! Unable to find rocket launcher device!\nExiting...\n");
    return(1);
  }
  
  printf("-- Opened device\n");
  printf("-- Test %d\n", usb_detach_kernel_driver_np(launcher, 0));
  printf("-- Test %d\n", usb_claim_interface(launcher, 0));
  printf("-- Test %d\n", usb_release_interface(launcher, 0));

  return(0);
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
