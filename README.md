
# Remotty - SSH to USB serial console
Running an headless server in a remote location is prone to network access issue. Expensive server are addressing this with a management board accessible via a dedicated management network. Looking at you Idrac, ILO, CIMC, IMPI...

Remotty is an ESP32 based, cheap remote serial console, prodiving a secure WiFi SSH access to console enabled on USB ports.

## Hardware & Software
Any ESP32 board with usb port and flashed with Arduino (https://github.com/defdefred/Remotty/blob/main/README_Arduino.md)

## Use cases
A cheap Openwrt router can acte as a SSH portal and serve a dedicated WiFi management network to several Remotty.

## Configuration
### Linux
### Grub

## Troubleshooting
### arduino-cli upload: [Errno 13] Permission denied: '/dev/ttyUSB0'
```
esptool.py v3.0-dev
Serial port /dev/ttyUSB0
Traceback (most recent call last):
  File "/usr/lib/python3/dist-packages/serial/serialposix.py", line 322, in open
    self.fd = os.open(self.portstr, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
PermissionError: [Errno 13] Permission denied: '/dev/ttyUSB0'
```
Enaling/Disabling the USB serial console is resetting `/dev/ttyUSB0` permission.
You need to `chmod 666 /dev/ttyUSB0` it.

## Next step
### Hard Reset server
### ?


## Usefull links
https://github.com/arduino/arduino-cli/releases

https://github.com/ewpa/LibSSH-ESP32

https://www.chiark.greenend.org.uk/~sgtatham/putty/

https://www.openssh.com/

https://www.libssh.org/

https://www.coreboot.org/GRUB2#On_a_USB_serial_or_USB_debug_adapter

https://www.rfc-editor.org/rfc/rfc4252#section-7



