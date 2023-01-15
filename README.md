# Remotty - SSH to USB serial console
![](https://github.com/defdefred/Remotty/blob/main/tty.gif)
## Use cases
Running an headless server in a remote location is prone to network access issue. Expensive server are addressing this with a management board accessible via a dedicated management network. Looking at you Idrac, ILO, CIMC, IMPI...

Remotty is an ESP32 based, cheap remote serial console, prodiving a secure WiFi SSH access to console enabled on USB ports.

A cheap Openwrt router can acte as a SSH portal and serve a dedicated WiFi management network to several Remotty.

## Hardware & Software
Any ESP32 board with usb port and flashed with Arduino (https://github.com/defdefred/EasyLibSSH/blob/main/README_Arduino.md)

## Configuration

### Remotty
Uncomment only if OS serial console is not activated.
```
// Display errors via serial
// #define DEBUG
```
Data to connect to your confidential WiFi management network. 
```
// Set local WiFi credentials
const char *configSTASSID = "mySID";
const char *configSTAPSK = "mySECRET";
```
You should customize SSH private host key for each microcontroller board.
```
// Set remotty hostkey
const char *configHOSTKEY = "-----BEGIN OPENSSH PRIVATE KEY-----\nb3BlbnNzaC1rZXktdjEAAAAABG5vbmUAA
↪AAEbm9uZQAAAAAAAAABAAAAMwAAAAtzc2gtZWQyNTUxOQAAACDGogjt/r8zbECmh6lm1UX6Gx+lAmbfG7PsFHTNkQiYQAAAAJD
↪QkgeZ0JIHmQAAAAtzc2gtZWQyNTUxOQAAACDGogjt/r8zbECmh6lm1UX6Gx+lAmbfG7PsFHTNkQiYQAAAAEAhjpXJ4AgPfRC8P
↪uuNIEq0itAFa2pxG0S5iMEe0iAY/saiCO3+vzNsQKaHqWbVRfobH6UCZt8bs+wUdM2RCJhAAAAAAAECAwQFBgcICQoLDA0=\n-
↪----END OPENSSH PRIVATE KEY-----";
```
Public key authorisation is the only one allowed.
```
// Set authorized_key
const char *configTYPEKEY = "ssh-ed25519";
const char *configAUTHKEY = "AAAAC3NzaC1lZDI1NTE5AAAAIPtooFfcMRdCSSouYMrBpXVG2y/qI2Iys6kkMo6mUHWq";
```
### Linux
Plug the microcontroller and check the TTY name:
```
$ ls -l /dev/ttyUSB*
crw-rw---- 1 root dialout 188, 0 Dec 24 00:45 /dev/ttyUSB0
```
Instantiate the systemd service:
```
root@minipc1:~# cp /usr/lib/systemd/system/serial-getty@.service /etc/systemd/system/serial-getty@ttyUSB0.service
```
Customize the service:
```
root@minipc:~# diff /usr/lib/systemd/system/serial-getty@.service /etc/systemd/system/serial-getty@ttyUSB0.service
34c34
< ExecStart=-/sbin/agetty -o '-p -- \\u' --keep-baud 115200,57600,38400,9600 - $TERM
---
> ExecStart=-/sbin/agetty 115200 %I -8 vt100
```
Use the service:
```
root@minipc:~# systemctl enable serial-getty@ttyUSB0.service
root@minipc:~# systemctl start serial-getty@ttyUSB0.service
```
When connected to the serial console, you can resize the terminal with:
```
stty rows 50 cols 132
```

### Linux kernel
To do...

### Grub
To do...

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
### agetty did not start on /dev/ttyUSBx
Did you customize the systemd service as advised? 
## Next ideas
### Hard Reset server with ATX
http://michael.stapelberg.ch/posts/2022-10-09-remote-power-button/
### Using true serial console to access old computer
Need to find a old equipment with RS-232 console...

## Usefull links
https://github.com/arduino/arduino-cli/releases

https://github.com/ewpa/LibSSH-ESP32

https://www.chiark.greenend.org.uk/~sgtatham/putty/

https://www.openssh.com/

https://www.libssh.org/

https://www.coreboot.org/GRUB2#On_a_USB_serial_or_USB_debug_adapter

https://szymonkrajewski.pl/how-to-boot-system-from-usb-using-grub/

https://www.rfc-editor.org/rfc/rfc4252#section-7

http://0pointer.de/blog/projects/serial-console.html





