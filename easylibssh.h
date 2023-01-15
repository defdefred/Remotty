#include "libssh_esp32.h"
#include "libssh_esp32_config.h"
#include <libssh/libssh.h>
#include <libssh/server.h>

ssh_session EASYLIBSSH_session = NULL;
ssh_bind EASYLIBSSH_sshbind = NULL;
ssh_key EASYLIBSSH_authorized_key[EASYLIBSSH_NBAUTHKEY] = { NULL };

void easylibssh_begin()
{
  enum ssh_keytypes_e typekey = SSH_KEYTYPE_UNKNOWN;
  int i,rc;

#ifdef EASYLIBSSH_DEBUG
  Serial.println("Start of easylibssh_begin");
#endif

  libssh_begin();

  for(i=0;i<EASYLIBSSH_NBAUTHKEY;i++) {

    #ifdef EASYLIBSSH_DEBUG
    Serial.print("Ssh authorized key type ");
    Serial.println(i);
    #endif

    typekey = ssh_key_type_from_name(EASYLIBSSH_TYPEKEY[i]);

    #ifdef EASYLIBSSH_DEBUG
    if (typekey == SSH_KEYTYPE_UNKNOWN) Serial.println("Failed to decode authorized key type");
    Serial.print("Ssh authorized key ");
    Serial.println(i);
    #endif

    rc = ssh_pki_import_pubkey_base64( EASYLIBSSH_AUTHKEY[i], typekey, &EASYLIBSSH_authorized_key[i]);

#ifdef EASYLIBSSH_DEBUG
    if (rc != SSH_OK) Serial.println("Failed to decode b64 authorized key");
#endif
  }
#ifdef EASYLIBSSH_DEBUG
  Serial.println("End of easylibssh_begin");
#endif
}

ssh_channel easylibssh_loop_start()
{
  ssh_message message = NULL;
  ssh_channel chan = NULL;
  int again = 1;
  int patience = 0;
  const int impatience = 10;
  ssh_key host_key = NULL;
  int i,rc;

#ifdef EASYLIBSSH_DEBUG
  Serial.println("Start of easylibssh_loop_start");
  Serial.println("Ssh bind new");
#endif

  EASYLIBSSH_sshbind=ssh_bind_new();
  if (EASYLIBSSH_sshbind == NULL) goto EASYLIBSSH_failed_bind_new;

#ifdef EASYLIBSSH_DEBUG
  Serial.println("Ssh hostkey");
#endif

  i = ssh_pki_import_privkey_base64( EASYLIBSSH_HOSTKEY, NULL, NULL, NULL, &host_key);
  if (i != SSH_OK) goto EASYLIBSSH_failed_bind;

#ifdef EASYLIBSSH_DEBUG
  Serial.println("Ssh bind import key");
  Serial.println(ssh_key_type_to_char(ssh_key_type(host_key)));
#endif

  if (ssh_bind_options_set(EASYLIBSSH_sshbind, SSH_BIND_OPTIONS_IMPORT_KEY, host_key) != SSH_OK) goto EASYLIBSSH_failed_bind;

#ifdef EASYLIBSSH_DEBUG
  Serial.println("Ssh bind listen");
#endif

  if (ssh_bind_listen(EASYLIBSSH_sshbind) != SSH_OK) goto EASYLIBSSH_failed_bind;

#ifdef EASYLIBSSH_DEBUG
  Serial.println("Ssh new session");
#endif

  EASYLIBSSH_session=ssh_new();
  if (EASYLIBSSH_session == NULL) goto EASYLIBSSH_failed_bind;

#ifdef EASYLIBSSH_DEBUG
  Serial.println("Ssh bind session");
#endif

  if (ssh_bind_accept(EASYLIBSSH_sshbind, EASYLIBSSH_session) != SSH_OK) goto EASYLIBSSH_failed_session;

#ifdef EASYLIBSSH_DEBUG
  Serial.println("Ssh key exchange");
#endif

  if (ssh_handle_key_exchange(EASYLIBSSH_session) != SSH_OK) goto EASYLIBSSH_failed_session;

  ssh_set_auth_methods(EASYLIBSSH_session, SSH_AUTH_METHOD_PUBLICKEY);
  again = 1;
  patience = 0;
  do { // Ssh Authentification
#ifdef EASYLIBSSH_DEBUG
    Serial.print("Authentification - New message - ");
    Serial.println(patience);
#endif
    message = ssh_message_get(EASYLIBSSH_session);
    if(message) {
      if (ssh_message_type(message) == SSH_REQUEST_AUTH && ssh_message_subtype(message) == SSH_AUTH_METHOD_PUBLICKEY) {
#ifdef EASYLIBSSH_DEBUG
        Serial.print(ssh_message_auth_user(message));
        Serial.println(" want to connect with pubkey");
#endif
        for(i=0;i<EASYLIBSSH_NBAUTHKEY;i++) {
          if (!ssh_key_cmp(ssh_message_auth_pubkey(message), EASYLIBSSH_authorized_key[i], SSH_KEY_CMP_PUBLIC)) {
            switch (ssh_message_auth_publickey_state(message)) {
              case SSH_PUBLICKEY_STATE_NONE:
#ifdef EASYLIBSSH_DEBUG
                Serial.print("pubkey ok but not yet validated ");
                Serial.println(i);
#endif
                i=EASYLIBSSH_NBAUTHKEY;
                ssh_message_auth_reply_pk_ok_simple(message);
                break;             
              case SSH_PUBLICKEY_STATE_VALID:
#ifdef EASYLIBSSH_DEBUG
                Serial.print("Pubkey ok and signature validated");
                Serial.println(i);
#endif
                i=EASYLIBSSH_NBAUTHKEY;
                again=0;
                ssh_message_auth_reply_success(message,0);
            }
          }
        }
      }
      else {
        ssh_message_auth_set_methods(message, SSH_AUTH_METHOD_PUBLICKEY);
#ifdef EASYLIBSSH_DEBUG
        Serial.print("Rejected message type: ");
        Serial.println(ssh_message_type(message));
        Serial.print("Message subtype: ");
        Serial.println(ssh_message_subtype(message));
#endif
        ssh_message_reply_default(message);
      }
      ssh_message_free(message);
    }  
#ifdef EASYLIBSSH_DEBUG
    else Serial.println("NULL message");
#endif
    patience++;
  } while(again && patience < impatience);

  if (patience == impatience) {
#ifdef EASYLIBSSH_DEBUG
    Serial.println("Too much failure");
#endif
    goto EASYLIBSSH_failed_session;
  }

  patience=0;
  do { // Ssh chanel
#ifdef EASYLIBSSH_DEBUG
    Serial.print("Ssh Channel - New message - ");
    Serial.println(patience);
#endif
    message = ssh_message_get(EASYLIBSSH_session);
    if(message) {
      if(ssh_message_type(message) == SSH_REQUEST_CHANNEL_OPEN && ssh_message_subtype(message) == SSH_CHANNEL_SESSION) {
#ifdef EASYLIBSSH_DEBUG
        Serial.println("Accepted");
#endif
        chan = ssh_message_channel_request_open_reply_accept(message);
      }
      else {
#ifdef EASYLIBSSH_DEBUG
        Serial.print("Rejected message type: ");
        Serial.println(ssh_message_type(message));
        Serial.print("Message subtype: ");
        Serial.println(ssh_message_subtype(message));
#endif
        ssh_message_reply_default(message);
      }
      ssh_message_free(message);
    }
#ifdef EASYLIBSSH_DEBUG
    else Serial.println("NULL message");
#endif
    patience++;
  } while(!chan && patience < impatience);

  if (patience == impatience) {
#ifdef EASYLIBSSH_DEBUG
    Serial.println("Too much failure");
#endif
    goto EASYLIBSSH_failed_session;
  }

  again = 1;
  patience = 0;
  do { // Ssh Pty
#ifdef EASYLIBSSH_DEBUG
    Serial.print("Ssh Pty - New message - ");
    Serial.println(patience);
#endif
    message = ssh_message_get(EASYLIBSSH_session);
    if (message) {
      if (ssh_message_type(message) == SSH_REQUEST_CHANNEL && ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_PTY) {
        again = 0;
        ssh_message_channel_request_reply_success(message);
      }
      else {
#ifdef EASYLIBSSH_DEBUG
        Serial.print("Rejected message type: ");
        Serial.println(ssh_message_type(message));
        Serial.print("Message subtype: ");
        Serial.println(ssh_message_subtype(message));
#endif
        ssh_message_reply_default(message);
      }
      ssh_message_free(message);
    }
#ifdef EASYLIBSSH_DEBUG
    else Serial.println("NULL message");
#endif
    patience++;
  } while(again && patience < impatience);

  if (patience == impatience) {
#ifdef EASYLIBSSH_DEBUG
    Serial.println("Too much failure");
#endif
    goto EASYLIBSSH_failed_channel;
  }

  again = 1;
  patience = 0;
  do { // Ssh shell
#ifdef EASYLIBSSH_DEBUG
    Serial.print("Ssh Shell - New message - ");
    Serial.println(patience);
#endif
    message = ssh_message_get(EASYLIBSSH_session);
    if(message) {
      if(ssh_message_type(message) == SSH_REQUEST_CHANNEL && ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_SHELL) {
        again = 0;
        ssh_message_channel_request_reply_success(message);
      }
      else {
#ifdef EASYLIBSSH_DEBUG
        Serial.print("Rejected message type: ");
        Serial.println(ssh_message_type(message));
        Serial.print("Message subtype: ");
        Serial.println(ssh_message_subtype(message));
#endif
        ssh_message_reply_default(message);
      }
      ssh_message_free(message);
    }
#ifdef EASYLIBSSH_DEBUG
    else Serial.println("NULL message");
#endif
    patience++;
  } while(again && patience < impatience);

  if (patience == impatience) {
#ifdef EASYLIBSSH_DEBUG
    Serial.println("Too much failure");
#endif
    goto EASYLIBSSH_failed_channel;
  }

  ssh_set_blocking(EASYLIBSSH_session, 0);

#ifdef EASYLIBSSH_DEBUG
  Serial.println("Start of pseudo Shell");
#endif

return chan;

EASYLIBSSH_failed_channel:

#ifdef EASYLIBSSH_DEBUG
  Serial.println("free channel");
#endif
  ssh_channel_free(chan);

EASYLIBSSH_failed_session:

#ifdef EASYLIBSSH_DEBUG
  if (ssh_get_error_code(EASYLIBSSH_session) != SSH_NO_ERROR) {
    Serial.print("Error session: ");
    Serial.println(ssh_get_error(EASYLIBSSH_session));
  }
  Serial.println("Disconnect session");
#endif

  ssh_disconnect(EASYLIBSSH_session);

EASYLIBSSH_failed_bind:

#ifdef EASYLIBSSH_DEBUG
  if (ssh_get_error_code(EASYLIBSSH_sshbind) != SSH_NO_ERROR) {
    Serial.print("Error ssh bind: ");
    Serial.println(ssh_get_error(EASYLIBSSH_sshbind));
  }
  Serial.println("Free sshbind");
#endif

  ssh_bind_free(EASYLIBSSH_sshbind);

EASYLIBSSH_failed_bind_new:
  return NULL;
}

void easylibssh_loop_end(ssh_channel chan)
{
#ifdef EASYLIBSSH_DEBUG
  Serial.println("End of pseudo Shell");
  Serial.println("Free channel");
#endif 

  ssh_channel_free(chan);
    
#ifdef EASYLIBSSH_DEBUG
  Serial.println("Disconnect session");
#endif

  ssh_disconnect(EASYLIBSSH_session);
    
#ifdef EASYLIBSSH_DEBUG
  Serial.println("Free sshbind");
#endif

  ssh_bind_free(EASYLIBSSH_sshbind);
}
