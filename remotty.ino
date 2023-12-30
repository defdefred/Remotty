// Display errors via serial
// #define DEBUG
// #define EASYLIBSSH_DEBUG

// Set remotty hostkey
const char *EASYLIBSSH_HOSTKEY = "-----BEGIN OPENSSH PRIVATE KEY-----\nb3BlbnNzaC1rZXktdjEAAAAABG5vbmUAAAAEbm9uZQAAAAAAAAABAAAAMwAAAAtzc2gtZWQyNTUxOQAAACDGogjt/r8zbECmh6lm1UX6Gx+lAmbfG7PsFHTNkQiYQAAAAJDQkgeZ0JIHmQAAAAtzc2gtZWQyNTUxOQAAACDGogjt/r8zbECmh6lm1UX6Gx+lAmbfG7PsFHTNkQiYQAAAAEAhjpXJ4AgPfRC8PuuNIEq0itAFa2pxG0S5iMEe0iAY/saiCO3+vzNsQKaHqWbVRfobH6UCZt8bs+wUdM2RCJhAAAAAAAECAwQFBgcICQoLDA0=\n-----END OPENSSH PRIVATE KEY-----";

// Set an array of authorized_key for libSSH
const uint8_t EASYLIBSSH_NBAUTHKEY = 2;
const char *EASYLIBSSH_TYPEKEY[] = { "ssh-ed25519",
                                     "ssh-ed25519" };
const char *EASYLIBSSH_AUTHKEY[] = { "AAAAC3NzaC1lZDI1NTE5AAAAIPtooFfereunifeni34345352y/qI2Iys6kkMo6mUHWq",
                                     "AAAAC3NzaC1lZDI1NTE5AAAAIPtooFfcMRdCSSouYMrBpXVG2y/qI2Iys6kkMo6mUHWq" };

#include "easylibssh.h"
// choose the name of your ssh_channel
ssh_channel chan;

// Set local WiFi credentials
// const char *configSTASSID = "mySID";
// const char *configSTAPSK = "mySECRET";
#include "WiFi.h"

#define MAX 2048
char buf[MAX];

void setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setHostname("Remotty");  
  WiFi.begin(configSTASSID, configSTAPSK);
  while (WiFi.status() != WL_CONNECTED) delay(1000);

  Serial.begin(115200);
  #ifdef DEBUG | EASYLIBSSH_DEBUG
  Serial.print("Wifi MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Wifi IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("EasyLibSSH begin");
  #endif

  easylibssh_begin();
}

void loop()
{
  int i;  

  delay(500);
  chan =  easylibssh_loop_start();

  if ( chan == NULL ) return;

  i=sprintf( buf, "# Welcome to Remotty - SSH access to serial USB console\n\rWaiting for serial");
  ssh_channel_write( chan, buf, i);

  while( Serial.available() == 0 ) {
    i=sprintf( buf, "%c", 27);
    Serial.write(buf, i);
    i=sprintf( buf, ".");
    ssh_channel_write( chan, buf, i);
    delay(1000);
  }
  i=sprintf( buf, "\r\n");
  ssh_channel_write( chan, buf, i);

  Serial.setTimeout(5);
  do {
    // From console to SSH
    i=Serial.readBytes(buf, MAX );
    if (i > 0) ssh_channel_write( chan, buf, i);

    // From SSH to console
    i=ssh_channel_read_nonblocking(chan, buf, MAX, 0);
    if (i > 0) Serial.write(buf, i);

    #ifdef DEBUG
    if (i > 0 && buf[0] == '\r') Serial.print("\n");
    #endif

  } while (ssh_channel_is_open(chan) && !ssh_channel_is_eof(chan));

  easylibssh_loop_end(chan);
}
