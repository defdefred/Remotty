
# Remotty - SSH to USB serial console

## Build

I don't use arduino-ide, it is way to bloaded and slow. I prefer arduino-cli and vim :-).

Compiling, flashing monitoring is easy and fast with a little shell script:
```
user@minipc1:~/Arduino$ cat ./do
./arduino-cli compile -b esp32:esp32:esp32 $1 || exit
./arduino-cli upload -p /dev/ttyUSB0 -b esp32:esp32:esp32 $1 || exit
./arduino-cli monitor -p /dev/ttyUSB0 -b esp32:esp32:esp32 -c 115200,off,8,none,off,1
```
Example:
```
user@minipc1:~/Arduino$ ./do remotty
Sketch uses 972922 bytes (74%) of program storage space. Maximum is 1310720 bytes.
Global variables use 52904 bytes (16%) of dynamic memory, leaving 274776 bytes for local variables. Maximum is 327680 bytes.

Used library Version Path
LibSSH-ESP32 3.0.0   /home/user/Arduino/libraries/LibSSH-ESP32
WiFi         1.0     /home/user/.arduino15/packages/esp32/hardware/esp32/1.0.5-rc4/libraries/WiFi

Used platform Version   Path
esp32:esp32   1.0.5-rc4 /home/user/.arduino15/packages/esp32/hardware/esp32/1.0.5-rc4

esptool.py v3.0-dev
Serial port /dev/ttyUSB0
Connecting.....
Chip is ESP32-D0WDQ6-V3 (revision 3)
Features: WiFi, BT, Dual Core, 240MHz, VRef calibration in efuse, Coding Scheme None
Crystal is 40MHz
MAC: 3c:61:05:0c:aa:7c
Uploading stub...
Running stub...
Stub running...
Changing baud rate to 921600
Changed.
Configuring flash size...
Auto-detected Flash size: 4MB
Compressed 8192 bytes to 47...
Wrote 8192 bytes (47 compressed) at 0x0000e000 in 0.0 seconds (effective 9118.2 kbit/s)...
Hash of data verified.
Compressed 18624 bytes to 12037...
Wrote 18624 bytes (12037 compressed) at 0x00001000 in 0.2 seconds (effective 874.2 kbit/s)...
Hash of data verified.
Compressed 973040 bytes to 602165...
Wrote 973040 bytes (602165 compressed) at 0x00010000 in 9.0 seconds (effective 865.8 kbit/s)...
Hash of data verified.
Compressed 3072 bytes to 128...
Wrote 3072 bytes (128 compressed) at 0x00008000 in 0.0 seconds (effective 7078.2 kbit/s)...
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
Monitor port settings:
baudrate=115200
Monitor port settings:
rts=off
Monitor port settings:
bits=8
Monitor port settings:
parity=none
Monitor port settings:
rts=off
Monitor port settings:
stop_bits=1
Connected to /dev/ttyUSB0! Press CTRL-C to exit.

```
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

## Use cases


## Usefull links
https://arduino.github.io/arduino-cli/0.19/installation/

https://github.com/ewpa/LibSSH-ESP32

https://www.chiark.greenend.org.uk/~sgtatham/putty/

https://www.openssh.com/

https://www.libssh.org/


