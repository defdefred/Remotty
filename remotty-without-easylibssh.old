// Display errors via serial
// #define DEBUG

// Set local WiFi credentials
const char *configSTASSID = "mySID";
const char *configSTAPSK = "mySECRET";

// Set remotty hostkey
const char *configHOSTKEY = "-----BEGIN OPENSSH PRIVATE KEY-----\nb3BlbnNzaC1rZXktdjEAAAAABG5vbmUAAAAEbm9uZQAAAAAAAAABAAAAMwAAAAtzc2gtZWQyNTUxOQAAACDGogjt/r8zbECmh6lm1UX6Gx+lAmbfG7PsFHTNkQiYQAAAAJDQkgeZ0JIHmQAAAAtzc2gtZWQyNTUxOQAAACDGogjt/r8zbECmh6lm1UX6Gx+lAmbfG7PsFHTNkQiYQAAAAEAhjpXJ4AgPfRC8PuuNIEq0itAFa2pxG0S5iMEe0iAY/saiCO3+vzNsQKaHqWbVRfobH6UCZt8bs+wUdM2RCJhAAAAAAAECAwQFBgcICQoLDA0=\n-----END OPENSSH PRIVATE KEY-----";

// Set authorized_key
const char *configTYPEKEY = "ssh-ed25519";
const char *configAUTHKEY = "AAAAC3NzaC1lZDI1NTE5AAAAIPtooFfcMRdCSSouYMrBpXVG2y/qI2Iys6kkMo6mUHWq";

// Set Banner
const char *configBANNER = "# Welcome to Remotty - SSH access to serial USB console\n\r\n\r";
const uint32_t sizeBANNER = 59;

#include "WiFi.h"
#include "libssh_esp32.h"
#include "libssh_esp32_config.h"
#include <libssh/libssh.h>
#include <libssh/server.h>

#define BUF_SIZE 2048

ssh_session session = NULL;
ssh_bind sshbind = NULL;
ssh_message message = NULL;
ssh_channel chan = NULL;
char buf[BUF_SIZE];
int again = 1;
int patience = 0;
const int impatience = 10;
int i;
enum ssh_keytypes_e typekey = SSH_KEYTYPE_UNKNOWN;
ssh_key host_key = NULL;
ssh_key authorized_key = NULL;

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
  Serial.print(configBANNER);
  Serial.print("Wifi MAC: ");
  Serial.println(WiFi.macAddress());
#endif
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setHostname("Remottty");  
  WiFi.begin(configSTASSID, configSTAPSK);
  while (WiFi.status() != WL_CONNECTED) delay(1000);

#ifdef DEBUG
  Serial.println(WiFi.localIP());
  Serial.println("Ssh begin");
#endif

  libssh_begin();

#ifdef DEBUG
  Serial.println("Ssh authorized key type");
#endif

  typekey = ssh_key_type_from_name(configTYPEKEY);

#ifdef DEBUG
  if (typekey == SSH_KEYTYPE_UNKNOWN) Serial.println("Failed to decode authorized key type");
  Serial.println("Ssh authorized key");
#endif

  i = ssh_pki_import_pubkey_base64( configAUTHKEY, typekey, &authorized_key);

#ifdef DEBUG
  if (i != SSH_OK) Serial.println("Failed to decode b64 authorized key");
  Serial.println("Jump master loop");
#else
  Serial.begin(115200);
#endif
}

void loop()
{
#ifdef DEBUG
  Serial.println("Begin master loop");
  Serial.println("Ssh bind new");
#endif

  sshbind=ssh_bind_new();
  if (sshbind == NULL) goto ssh_failed_bind_new;

#ifdef DEBUG
  Serial.println("Ssh hostkey");
#endif

  i = ssh_pki_import_privkey_base64( configHOSTKEY, NULL, NULL, NULL, &host_key);
  if (i != SSH_OK) goto ssh_failed_bind;

#ifdef DEBUG
  Serial.println("Ssh bind import key");
  Serial.println(ssh_key_type_to_char(ssh_key_type(host_key)));
#endif

  if (ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_IMPORT_KEY, host_key) != SSH_OK) goto ssh_failed_bind;

#ifdef DEBUG
  Serial.println("Ssh bind listen");
#endif

  if (ssh_bind_listen(sshbind) != SSH_OK) goto ssh_failed_bind;

#ifdef DEBUG
  Serial.println("Ssh new session");
#endif

  session=ssh_new();
  if (session == NULL) goto ssh_failed_bind;

#ifdef DEBUG
  Serial.println("Ssh bind session");
#endif

  if (ssh_bind_accept(sshbind, session) != SSH_OK) goto ssh_failed_session;

#ifdef DEBUG
  Serial.println("Ssh key exchange");
#endif

  if (ssh_handle_key_exchange(session) != SSH_OK) goto ssh_failed_session;

  ssh_set_auth_methods(session, SSH_AUTH_METHOD_PUBLICKEY);
  again = 1;
  patience = 0;
  do { // Ssh Authentification
#ifdef DEBUG
    Serial.print("Authentification - New message - ");
    Serial.println(patience);
#endif
    message = ssh_message_get(session);
    if(message) {
      if (ssh_message_type(message) == SSH_REQUEST_AUTH && ssh_message_subtype(message) == SSH_AUTH_METHOD_PUBLICKEY) {
#ifdef DEBUG
        Serial.print(ssh_message_auth_user(message));
        Serial.println(" want to connect with pubkey");
#endif
        if (!ssh_key_cmp(ssh_message_auth_pubkey(message), authorized_key, SSH_KEY_CMP_PUBLIC)) {
          switch (ssh_message_auth_publickey_state(message)) {
            case SSH_PUBLICKEY_STATE_NONE:
#ifdef DEBUG
              Serial.println("pubkey ok but not yet validated");
#endif
              ssh_message_auth_reply_pk_ok_simple(message);
              break;             
            case SSH_PUBLICKEY_STATE_VALID:
#ifdef DEBUG
              Serial.println("Pubkey ok and signature validated");
#endif
              again = 0;
              ssh_message_auth_reply_success(message,0);
           }
         }
      }
      else {
        ssh_message_auth_set_methods(message, SSH_AUTH_METHOD_PUBLICKEY);
#ifdef DEBUG
        Serial.print("Rejected message type: ");
        Serial.println(ssh_message_type(message));
        Serial.print("Message subtype: ");
        Serial.println(ssh_message_subtype(message));
#endif
        ssh_message_reply_default(message);
      }
      ssh_message_free(message);
    }  
#ifdef DEBUG
    else Serial.println("NULL message");
#endif
    patience++;
  } while(again && patience < impatience);

  if (patience == impatience) {
#ifdef DEBUG
    Serial.println("Too much failure");
#endif
    goto ssh_failed_session;
  }

  patience=0;
  do { // Ssh chanel
#ifdef DEBUG
    Serial.print("Ssh Channel - New message - ");
    Serial.println(patience);
#endif
    message = ssh_message_get(session);
    if(message) {
      if(ssh_message_type(message) == SSH_REQUEST_CHANNEL_OPEN && ssh_message_subtype(message) == SSH_CHANNEL_SESSION) {
#ifdef DEBUG
        Serial.println("Accepted");
#endif
        chan = ssh_message_channel_request_open_reply_accept(message);
      }
      else {
#ifdef DEBUG
        Serial.print("Rejected message type: ");
        Serial.println(ssh_message_type(message));
        Serial.print("Message subtype: ");
        Serial.println(ssh_message_subtype(message));
#endif
        ssh_message_reply_default(message);
      }
      ssh_message_free(message);
    }
#ifdef DEBUG
    else Serial.println("NULL message");
#endif
    patience++;
  } while(!chan && patience < impatience);

  if (patience == impatience) {
#ifdef DEBUG
    Serial.println("Too much failure");
#endif
    goto ssh_failed_session;
  }

  again = 1;
  patience = 0;
  do { // Ssh Pty
#ifdef DEBUG
    Serial.print("Ssh Pty - New message - ");
    Serial.println(patience);
#endif
    message = ssh_message_get(session);
    if (message) {
      if (ssh_message_type(message) == SSH_REQUEST_CHANNEL && ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_PTY) {
        again = 0;
        ssh_message_channel_request_reply_success(message);
      }
      else {
#ifdef DEBUG
        Serial.print("Rejected message type: ");
        Serial.println(ssh_message_type(message));
        Serial.print("Message subtype: ");
        Serial.println(ssh_message_subtype(message));
#endif
        ssh_message_reply_default(message);
      }
      ssh_message_free(message);
    }
#ifdef DEBUG
    else Serial.println("NULL message");
#endif
    patience++;
  } while(again && patience < impatience);

  if (patience == impatience) {
#ifdef DEBUG
    Serial.println("Too much failure");
#endif
    goto ssh_failed_channel;
  }

  again = 1;
  patience = 0;
  do { // Ssh shell
#ifdef DEBUG
    Serial.print("Ssh Shell - New message - ");
    Serial.println(patience);
#endif
    message = ssh_message_get(session);
    if(message) {
      if(ssh_message_type(message) == SSH_REQUEST_CHANNEL && ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_SHELL) {
        again = 0;
        ssh_message_channel_request_reply_success(message);
      }
      else {
#ifdef DEBUG
        Serial.print("Rejected message type: ");
        Serial.println(ssh_message_type(message));
        Serial.print("Message subtype: ");
        Serial.println(ssh_message_subtype(message));
#endif
        ssh_message_reply_default(message);
      }
      ssh_message_free(message);
    }
#ifdef DEBUG
    else Serial.println("NULL message");
#endif
    patience++;
  } while(again && patience < impatience);

  if (patience == impatience) {
#ifdef DEBUG
    Serial.println("Too much failure");
#endif
    goto ssh_failed_channel;
  }

#ifdef DEBUG
  Serial.print(configBANNER);
  Serial.println(sizeBANNER);
#endif

  ssh_channel_write( chan, configBANNER, sizeBANNER);

#ifdef DEBUG
  Serial.println("Start of pseudo Shell");
#endif

  ssh_set_blocking(session, 0);
  Serial.setTimeout(5);

  do {
// From console to SSH
    i=Serial.readBytes(buf, sizeof(buf));
    if (i > 0) ssh_channel_write( chan, buf, i);

// From SSH to console
    i=ssh_channel_read_nonblocking(chan, buf, sizeof(buf), 0);
    if (i > 0) Serial.write(buf, i);

#ifdef DEBUG
    if (i > 0 && buf[0] == '\r') Serial.print("\n");
#endif

  } while (ssh_channel_is_open(chan) && !ssh_channel_is_eof(chan));

#ifdef DEBUG
  Serial.println("End of pseudo Shell");
#endif

ssh_failed_channel:

#ifdef DEBUG
  Serial.println("Close channel");
#endif
  ssh_channel_close(chan);

ssh_failed_session:

#ifdef DEBUG
  if (ssh_get_error_code(session) != SSH_NO_ERROR) {
    Serial.print("Error session: ");
    Serial.println(ssh_get_error(session));
  }
  Serial.println("Disconnect session");
#endif

  ssh_disconnect(session);

ssh_failed_bind:

#ifdef DEBUG
  if (ssh_get_error_code(sshbind) != SSH_NO_ERROR) {
    Serial.print("Error ssh bind: ");
    Serial.println(ssh_get_error(sshbind));
  }
  Serial.println("Free sshbind");
#endif

  ssh_bind_free(sshbind);

ssh_failed_bind_new:

  delay(100);
}
