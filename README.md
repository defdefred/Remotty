# Remotty - SSH to USB serial console
![](https://github.com/defdefred/Remotty/blob/main/tty.gif)
## Use cases
Running an headless server in a remote location is prone to network access issue. Expensive server are addressing this with a management board accessible via a dedicated management network. Looking at you Idrac, ILO, CIMC, IMPI...

Remotty is an ESP32 based, cheap remote serial console, prodiving a secure WiFi SSH access to console enabled on USB ports.

A cheap Openwrt router can acte as a SSH portal and serve a dedicated WiFi management network to several Remotty.

## Hardware & Software
Any ESP32 board with usb port and flashed with [Arduino](https://github.com/defdefred/EasyLibSSH/blob/main/README_Arduino.md). Tested Boards:
- Lolin mini s2 (/dev/ttyACM0) -> Ok but can't be remotely flashed
- LILYGO TTGO T-Display (/dev/ttyUSB0) -> Ok

The libSSH server api is quite tricky, so I use the [easylibssh](https://github.com/defdefred/EasyLibSSH) library.
## Configuration

### Remotty
Uncomment only if OS serial console is not activated.
```
// Display errors via serial
// #define DEBUG
// #define EASYLIBSSH_DEBUG
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
// Set an array of authorized_key for libSSH
const uint8_t EASYLIBSSH_NBAUTHKEY = 2;
const char *EASYLIBSSH_TYPEKEY[] = { "ssh-ed25519",
                                     "ssh-ed25519" };
const char *EASYLIBSSH_AUTHKEY[] = { "AAAAC3NzaC1lZDI1NTE5AAAAIPtooFfereunifeni34345352y/qI2Iys6kkMo6mUHWq",
                                     "AAAAC3NzaC1lZDI1NTE5AAAAIPtooFfcMRdCSSouYMrBpXVG2y/qI2Iys6kkMo6mUHWq" };
```
Data to connect to your confidential WiFi management network. 
```
// Set local WiFi credentials
const char *configSTASSID = "mySID";
const char *configSTAPSK = "mySECRET";
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

### EFI
To do...

## Troubleshooting

### How to compile/flash remotty?
Compile:
```
./arduino-cli compile -b esp32:esp32:esp32 remotty
```
Flash:
```
./arduino-cli upload -p /dev/ttyUSB0 -b esp32:esp32:esp32 remotty
```
Monitor after disabling the agetty console:
```
./arduino-cli monitor -p /dev/ttyUSB0 -b esp32:esp32:esp32 -c 115200,off,8,none,off,1
```

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

### ssh is not authenticating
Try to specify you private keys and wait for agetty to issue the login prompt:
```
$ ssh -i ~/.ssh/id_ed25519 192.168.3.131
# Welcome to Remotty - SSH access to serial USB console
Waiting for serial.

Debian GNU/Linux bookworm/sid wize ttyACM0

wize login:
```
### agetty did not start on /dev/ttyUSBx
Did you customize the systemd service as advised?

### ESP32 is in undefined state
You can reset it by reflashing it.

### What is the Remotty IP?
If you dont want to compile the code with debug mode, you can search for the Remotty IP with the one-liner:
```
$ IP=1;while [ "$IP" -lt 255 ] ; do echo -n "$IP " ; ssh -i ~/.ssh/id_ed25519 -o ConnectTimeOut=1 192.168.3."$IP" 2>&1 | grep -vF "Connection timed out" ; IP=$(("$IP"+1)) ; done
```
Just change `192.168.3.` with your network and `id_ed25519` with your private ssh key.

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





