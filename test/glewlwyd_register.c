/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>


#include <check.h>
#include <ulfius.h>
#include <orcania.h>
#include <yder.h>

#include "unit-tests.h"

#define SERVER_URI "http://localhost:4593/api"
#define USERNAME_ADMIN "admin"
#define USERNAME "user1"
#define EMAIL "dev1@glewlwyd"
#define PASSWORD "password"
#define MOD_TYPE "register"
#define MOD_NAME "register"
#define MOD_DISPLAY_NAME "Register"

#define SESSION_KEY "G_REGISTER_SESSION"
#define SESSION_RESET_CREDENTIALS_KEY "G_CREDENTIALS_SESSION"
#define SESSION_DURATION 3600
#define SCOPE "g_profile"
#define SCOPE_SCHEME "scope2"
#define SCHEME_TYPE "mock"
#define SCHEME_NAME "mock_scheme_95"
#define SCHEME_DISPLAY_NAME "Mock 95"

#define NEW_USERNAME "semias"
#define NEW_USERNAME_CANCELLED "esras"
#define NEW_USERNAME_INVALID_SESSION "morfessa"
#define NEW_NAME "Semias from somewhere"
#define NEW_PASSWORD "newpassword"
#define NEW_EMAIL "esras@glewlwyd.tld"

#define MAIL_CODE_DURATION 600
#define MAIL_CODE_LEGTH 6
#define MAIL_HOST "localhost"
#define MAIL_PORT_WITH_USERNAME 2526
#define MAIL_PORT_WITHOUT_USERNAME 2527
#define MAIL_PORT_CODE_EXPIRED 2528
#define MAIL_PORT_MULTILANG 2529
#define MAIL_PORT_UPDATE_EMAIL_INVALID_TOKEN 2530
#define MAIL_PORT_UPDATE_EMAIL_VALID_TOKEN 2531
#define MAIL_PORT_RESET_CREDENTIALS_EMAIL_INVALID_TOKEN 2532
#define MAIL_PORT_RESET_CREDENTIALS_EMAIL_VALID_TOKEN 2533
#define MAIL_PORT_RESET_CREDENTIALS_EMAIL_FLOW 2533
#define MAIL_FROM "glewlwyd"
#define MAIL_SUBJECT "Authorization Code"
#define MAIL_SUBJECT_FR "Code d'autorisation"
#define MAIL_CONTENT_TYPE "plain/text"
#define MAIL_BODY_PATTERN "The code or token is "
#define MAIL_BODY_PATTERN_FR "Le code ou token se trouve être le suivant "
#define MAIL_BODY_CODE "{CODE}"
#define MAIL_BODY_TOKEN "{TOKEN}"

#define RESET_CREDENTIALS_CODE_PROPERTY "reset-credentials-code-property"
#define RESET_CREDENTIALS_CODE_LIST_SIZE 4

#define GLEWLWYD_TOKEN_LENGTH 32

char * mail_host = NULL;

#define BACKLOG_MAX  (10)
#define BUF_SIZE  4096
#define STREQU(a,b)  (strcmp(a, b) == 0)

struct _u_request admin_req;
struct _u_request user_req;

struct smtp_manager {
  char * mail_data;
  unsigned int port;
  int sockfd;
  const char * body_pattern;
};

/**
 * 
 * Function that emulates a very simple SMTP server
 * Taken from Kenneth Finnegan's ccsmtp program
 * https://gist.github.com/PhirePhly/2914635
 * This function is under the GPL2 license
 * 
 */
static void handle_smtp (struct smtp_manager * manager) {
  int rc, i;
  char buffer[BUF_SIZE], bufferout[BUF_SIZE];
  int buffer_offset = 0;
  buffer[BUF_SIZE-1] = '\0';

  // Flag for being inside of DATA verb
  int inmessage = 0;

  sprintf(bufferout, "220 ulfius.tld SMTP CCSMTP\r\n");
  send(manager->sockfd, bufferout, strlen(bufferout), 0);

  while (1) {
    fd_set sockset;
    struct timeval tv;

    FD_ZERO(&sockset);
    FD_SET(manager->sockfd, &sockset);
    tv.tv_sec = 120; // Some SMTP servers pause for ~15s per message
    tv.tv_usec = 0;

    // Wait tv timeout for the server to send anything.
    select(manager->sockfd+1, &sockset, NULL, NULL, &tv);

    if (!FD_ISSET(manager->sockfd, &sockset)) {
      y_log_message(Y_LOG_LEVEL_DEBUG, "%d: Socket timed out", manager->sockfd);
      break;
    }

    int buffer_left = BUF_SIZE - buffer_offset - 1;
    if (buffer_left == 0) {
      y_log_message(Y_LOG_LEVEL_DEBUG, "%d: Command line too long", manager->sockfd);
      sprintf(bufferout, "500 Too long\r\n");
      send(manager->sockfd, bufferout, strlen(bufferout), 0);
      buffer_offset = 0;
      continue;
    }

    rc = recv(manager->sockfd, buffer + buffer_offset, buffer_left, 0);
    if (rc == 0) {
      y_log_message(Y_LOG_LEVEL_DEBUG, "%d: Remote host closed socket", manager->sockfd);
      break;
    }
    if (rc == -1) {
      y_log_message(Y_LOG_LEVEL_DEBUG, "%d: Error on socket", manager->sockfd);
      break;
    }

    buffer_offset += rc;

    char *eol;

    // Only process one line of the received buffer at a time
    // If multiple lines were received in a single recv(), goto
    // back to here for each line
    //
processline:
    eol = strstr(buffer, "\r\n");
    if (eol == NULL) {
      y_log_message(Y_LOG_LEVEL_DEBUG, "%d: Haven't found EOL yet", manager->sockfd);
      continue;
    }

    // Null terminate each line to be processed individually
    eol[0] = '\0';

    if (!inmessage) { // Handle system verbs
      // Replace all lower case letters so verbs are all caps
      for (i=0; i<4; i++) {
        if (islower(buffer[i])) {
          buffer[i] += 'A' - 'a';
        }
      }
      // Null-terminate the verb for strcmp
      buffer[4] = '\0';

      // Respond to each verb accordingly.
      // You should replace these with more meaningful
      // actions than simply printing everything.
      //
      if (STREQU(buffer, "HELO")) { // Initial greeting
        sprintf(bufferout, "250 Ok\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
      } else if (STREQU(buffer, "MAIL")) { // New mail from...
        sprintf(bufferout, "250 Ok\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
      } else if (STREQU(buffer, "RCPT")) { // Mail addressed to...
        sprintf(bufferout, "250 Ok recipient\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
      } else if (STREQU(buffer, "DATA")) { // Message contents...
        sprintf(bufferout, "354 Continue\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
        inmessage = 1;
      } else if (STREQU(buffer, "RSET")) { // Reset the connection
        sprintf(bufferout, "250 Ok reset\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
      } else if (STREQU(buffer, "NOOP")) { // Do nothing.
        sprintf(bufferout, "250 Ok noop\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
      } else if (STREQU(buffer, "QUIT")) { // Close the connection
        sprintf(bufferout, "221 Ok\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
        break;
      } else { // The verb used hasn't been implemented.
        sprintf(bufferout, "502 Command Not Implemented\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
      }
    } else { // We are inside the message after a DATA verb.
      if (0 == o_strncmp(manager->body_pattern, buffer, o_strlen(manager->body_pattern))) {
        manager->mail_data = o_strdup(buffer+o_strlen(manager->body_pattern));
      }
      if (STREQU(buffer, ".")) { // A single "." signifies the end
        sprintf(bufferout, "250 Ok\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
        inmessage = 0;
      }
    }

    // Shift the rest of the buffer to the front
    memmove(buffer, eol+2, BUF_SIZE - (eol + 2 - buffer));
    buffer_offset -= (eol - buffer) + 2;

    // Do we already have additional lines to process? If so,
    // commit a horrid sin and goto the line processing section again.
    if (strstr(buffer, "\r\n")) 
      goto processline;
  }

  // All done. Clean up everything and exit.
  shutdown(manager->sockfd, SHUT_WR);
  close(manager->sockfd);
}

static void * simple_smtp(void * args) {
  struct smtp_manager * manager = (struct smtp_manager *)args;
  int server_fd; 
  struct sockaddr_in address; 
  int opt = 1; 
  int addrlen = sizeof(address); 
  
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) != 0) {
    if (!setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = INADDR_ANY;
      address.sin_port = htons( manager->port );
         
      if (!bind(server_fd, (struct sockaddr *)&address, sizeof(address))) {
        if (listen(server_fd, 3) >= 0) {
          if ((manager->sockfd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) >= 0) {
            handle_smtp(manager);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "simple_smtp - Error accept");
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "simple_smtp - Error listen");
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "simple_smtp - Error bind");
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "simple_smtp - Error setsockopt");
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "simple_smtp - Error socket");
  }
  
  shutdown(server_fd, SHUT_RDWR);
  close(server_fd);

  pthread_exit(NULL);
}

START_TEST(test_glwd_register_add_mod_error)
{
  json_t * j_body = json_pack("{ss ss ss s{si si s[ss] ss s[{ss ss ss ss}] so} so}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "parameters",
                                "session-key", SESSION_DURATION,
                                "session-duration", SESSION_DURATION,
                                "scope",
                                  SCOPE,
                                  SCOPE_SCHEME,
                                "set-password", "always",
                                "schemes",
                                  "module", SCHEME_TYPE,
                                  "name", SCHEME_NAME,
                                  "register", "always",
                                  "display_name", SCHEME_DISPLAY_NAME,
                                "verify-email", json_false(),
                              "enabled", json_true()),
         * j_error = json_string("session-key is mandatory and must be a non empty string");
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 400, j_error, NULL, NULL), 1);
  json_decref(j_body);
  json_decref(j_error);
}
END_TEST

START_TEST(test_glwd_register_add_mod_noverify)
{
  /* 
  {
    "module":"register",
    "name":"register",
    "display_name":"Register",
    "parameters":{
      "session-key":"G_REGISTER_SESSIONE",
      "session-duration":3600,
      "scope":["g_profile"],
      "set-password":"no",
      "schemes":[
        {"module":"otp","name":"otp","register":"always","display_name":"OTP"},
        {"module":"webauthn","name":"webauthn","register":"yes","display_name":"Webauthn"}
      ],
      
      "verify-email":true,
      "email-is-username":true,
      
      "verification-code-length":8,
      "verification-code-duration":600,
      
      "host":"localhost",
      "port":0,
      "user":"",
      "password":"",
      "from":"glewlwyd@localhost",
      "subject":"Term",
      "content-type":"",
      "body-pattern":"Le code est {CODE}",
    },
    "enabled":true
  }*/
  json_t * j_body = json_pack("{ss ss ss s{ss si s[ss] ss s[{ss ss ss ss}] so} so}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "parameters",
                                "session-key", SESSION_KEY,
                                "session-duration", SESSION_DURATION,
                                "scope",
                                  SCOPE,
                                  SCOPE_SCHEME,
                                "set-password", "always",
                                "schemes",
                                  "module", SCHEME_TYPE,
                                  "name", SCHEME_NAME,
                                  "register", "always",
                                  "display_name", SCHEME_DISPLAY_NAME,
                                "verify-email", json_false(),
                              "enabled", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_add_mod_verify_with_username)
{
  json_t * j_body = json_pack("{ss ss ss s{ss si s[ss] ss s[{ss ss ss ss}] so so si si ss si ss ss ss ss} so}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "parameters",
                                "session-key", SESSION_KEY,
                                "session-duration", SESSION_DURATION,
                                "scope",
                                  SCOPE,
                                  SCOPE_SCHEME,
                                "set-password", "always",
                                "schemes",
                                  "module", SCHEME_TYPE,
                                  "name", SCHEME_NAME,
                                  "register", "always",
                                  "display_name", SCHEME_DISPLAY_NAME,
                                "verify-email", json_true(),
                                "email-is-username", json_false(),
                                "verification-code-length", MAIL_CODE_LEGTH,
                                "verification-code-duration", MAIL_CODE_DURATION,
                                "host", MAIL_HOST,
                                "port", MAIL_PORT_WITH_USERNAME,
                                "from", MAIL_FROM,
                                "subject", MAIL_SUBJECT,
                                "content-type", MAIL_CONTENT_TYPE,
                                "body-pattern", MAIL_BODY_PATTERN MAIL_BODY_CODE,
                              "enabled", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_add_mod_verify_without_username)
{
  json_t * j_body = json_pack("{ss ss ss s{ss si s[ss] ss s[{ss ss ss ss}] so so si si ss si ss ss ss ss} so}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "parameters",
                                "session-key", SESSION_KEY,
                                "session-duration", SESSION_DURATION,
                                "scope",
                                  SCOPE,
                                  SCOPE_SCHEME,
                                "set-password", "always",
                                "schemes",
                                  "module", SCHEME_TYPE,
                                  "name", SCHEME_NAME,
                                  "register", "always",
                                  "display_name", SCHEME_DISPLAY_NAME,
                                "verify-email", json_true(),
                                "email-is-username", json_true(),
                                "verification-code-length", MAIL_CODE_LEGTH,
                                "verification-code-duration", MAIL_CODE_DURATION,
                                "host", MAIL_HOST,
                                "port", MAIL_PORT_WITHOUT_USERNAME,
                                "from", MAIL_FROM,
                                "subject", MAIL_SUBJECT,
                                "content-type", MAIL_CONTENT_TYPE,
                                "body-pattern", MAIL_BODY_PATTERN MAIL_BODY_CODE,
                              "enabled", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_add_mod_verify_with_username_token)
{
  json_t * j_body = json_pack("{ss ss ss s{ss si s[ss] ss s[{ss ss ss ss}] so so si si ss si ss ss ss ss} so}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "parameters",
                                "session-key", SESSION_KEY,
                                "session-duration", SESSION_DURATION,
                                "scope",
                                  SCOPE,
                                  SCOPE_SCHEME,
                                "set-password", "always",
                                "schemes",
                                  "module", SCHEME_TYPE,
                                  "name", SCHEME_NAME,
                                  "register", "always",
                                  "display_name", SCHEME_DISPLAY_NAME,
                                "verify-email", json_true(),
                                "email-is-username", json_false(),
                                "verification-code-length", MAIL_CODE_LEGTH,
                                "verification-code-duration", MAIL_CODE_DURATION,
                                "host", MAIL_HOST,
                                "port", MAIL_PORT_WITH_USERNAME,
                                "from", MAIL_FROM,
                                "subject", MAIL_SUBJECT,
                                "content-type", MAIL_CONTENT_TYPE,
                                "body-pattern", MAIL_BODY_PATTERN MAIL_BODY_TOKEN,
                              "enabled", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_add_mod_noverify_session_expired)
{
  json_t * j_body = json_pack("{ss ss ss s{ss si s[ss] ss s[{ss ss ss ss}] so} so}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "parameters",
                                "session-key", SESSION_KEY,
                                "session-duration", 4,
                                "scope",
                                  SCOPE,
                                  SCOPE_SCHEME,
                                "set-password", "always",
                                "schemes",
                                  "module", SCHEME_TYPE,
                                  "name", SCHEME_NAME,
                                  "register", "always",
                                  "display_name", SCHEME_DISPLAY_NAME,
                                "verify-email", json_false(),
                              "enabled", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_add_mod_verify_without_username_code_expired)
{
  json_t * j_body = json_pack("{ss ss ss s{ss si s[ss] ss s[{ss ss ss ss}] so so si si ss si ss ss ss ss} so}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "parameters",
                                "session-key", SESSION_KEY,
                                "session-duration", SESSION_DURATION,
                                "scope",
                                  SCOPE,
                                  SCOPE_SCHEME,
                                "set-password", "always",
                                "schemes",
                                  "module", SCHEME_TYPE,
                                  "name", SCHEME_NAME,
                                  "register", "always",
                                  "display_name", SCHEME_DISPLAY_NAME,
                                "verify-email", json_true(),
                                "email-is-username", json_true(),
                                "verification-code-length", MAIL_CODE_LEGTH,
                                "verification-code-duration", 1,
                                "host", MAIL_HOST,
                                "port", MAIL_PORT_CODE_EXPIRED,
                                "from", MAIL_FROM,
                                "subject", MAIL_SUBJECT,
                                "content-type", MAIL_CONTENT_TYPE,
                                "body-pattern", MAIL_BODY_PATTERN MAIL_BODY_CODE,
                              "enabled", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_add_mod_verify_multilang_without_username)
{
  json_t * j_body = json_pack("{ss ss ss so s{ss si s[ss] ss s[{ss ss ss ss}] so so si si ss si ss ss s{s{sossss}s{sossss}}}}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "enabled", json_true(),
                              "parameters",
                                "session-key", SESSION_KEY,
                                "session-duration", SESSION_DURATION,
                                "scope",
                                  SCOPE,
                                  SCOPE_SCHEME,
                                "set-password", "always",
                                "schemes",
                                  "module", SCHEME_TYPE,
                                  "name", SCHEME_NAME,
                                  "register", "always",
                                  "display_name", SCHEME_DISPLAY_NAME,
                                "verify-email", json_true(),
                                "email-is-username", json_true(),
                                "verification-code-length", MAIL_CODE_LEGTH,
                                "verification-code-duration", MAIL_CODE_DURATION,
                                "host", MAIL_HOST,
                                "port", MAIL_PORT_MULTILANG,
                                "from", MAIL_FROM,
                                "content-type", MAIL_CONTENT_TYPE,
                                "templates",
                                  "en",
                                    "defaultLang", json_true(),
                                    "subject", MAIL_SUBJECT,
                                    "body-pattern", MAIL_BODY_PATTERN MAIL_BODY_CODE,
                                  "fr",
                                    "defaultLang", json_false(),
                                    "subject", MAIL_SUBJECT_FR,
                                    "body-pattern", MAIL_BODY_PATTERN_FR MAIL_BODY_CODE);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_add_mod_noverify_registration_enabled)
{
  json_t * j_body = json_pack("{ss ss ss s{so ss si s[ss] ss s[{ss ss ss ss}] so} so}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "parameters",
                                "registration", json_true(),
                                "session-key", SESSION_KEY,
                                "session-duration", SESSION_DURATION,
                                "scope",
                                  SCOPE,
                                  SCOPE_SCHEME,
                                "set-password", "always",
                                "schemes",
                                  "module", SCHEME_TYPE,
                                  "name", SCHEME_NAME,
                                  "register", "always",
                                  "display_name", SCHEME_DISPLAY_NAME,
                                "verify-email", json_false(),
                              "enabled", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_add_mod_registration_disabled_update_email_enabled_for_token_invalid)
{
  json_t * j_body = json_pack("{ss ss ss so s{so so si ss ss ss si s{s{sossss}s{sossss}}}}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "enabled", json_true(),
                              "parameters",
                                "registration", json_false(),
                                "update-email", json_true(),
                                "update-email-token-duration", GLEWLWYD_TOKEN_LENGTH,
                                "update-email-from", MAIL_FROM,
                                "update-email-content-type", MAIL_CONTENT_TYPE,
                                "host", MAIL_HOST,
                                "port", MAIL_PORT_UPDATE_EMAIL_INVALID_TOKEN,
                                "templatesUpdateEmail",
                                  "en",
                                    "defaultLang", json_true(),
                                    "subject", MAIL_SUBJECT,
                                    "body-pattern", MAIL_BODY_PATTERN MAIL_BODY_TOKEN,
                                  "fr",
                                    "defaultLang", json_false(),
                                    "subject", MAIL_SUBJECT_FR,
                                    "body-pattern", MAIL_BODY_PATTERN_FR MAIL_BODY_TOKEN);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_add_mod_registration_disabled_update_email_enabled_for_token_valid)
{
  json_t * j_body = json_pack("{ss ss ss so s{so so si ss ss ss si s{s{sossss}s{sossss}}}}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "enabled", json_true(),
                              "parameters",
                                "registration", json_false(),
                                "update-email", json_true(),
                                "update-email-token-duration", GLEWLWYD_TOKEN_LENGTH,
                                "update-email-from", MAIL_FROM,
                                "update-email-content-type", MAIL_CONTENT_TYPE,
                                "host", MAIL_HOST,
                                "port", MAIL_PORT_UPDATE_EMAIL_VALID_TOKEN,
                                "templatesUpdateEmail",
                                  "en",
                                    "defaultLang", json_true(),
                                    "subject", MAIL_SUBJECT,
                                    "body-pattern", MAIL_BODY_PATTERN MAIL_BODY_TOKEN,
                                  "fr",
                                    "defaultLang", json_false(),
                                    "subject", MAIL_SUBJECT_FR,
                                    "body-pattern", MAIL_BODY_PATTERN_FR MAIL_BODY_TOKEN);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_add_mod_reset_credentials_enabled_email_for_token_invalid)
{
  json_t * j_body = json_pack("{ss ss ss so s{so so so so so si ss ss ss si ss si s{s{sossss}s{sossss}}}}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "enabled", json_true(),
                              "parameters",
                                "registration", json_false(),
                                "update-email", json_false(),
                                "reset-credentials", json_true(),
                                "reset-credentials-email", json_true(),
                                "reset-credentials-code", json_false(),
                                "reset-credentials-session-duration", SESSION_DURATION,
                                "reset-credentials-session-key", SESSION_RESET_CREDENTIALS_KEY,
                                "reset-credentials-from", MAIL_FROM,
                                "reset-credentials-content-type", MAIL_CONTENT_TYPE,
                                "reset-credentials-token-duration", MAIL_CODE_DURATION,
                                "host", MAIL_HOST,
                                "port", MAIL_PORT_RESET_CREDENTIALS_EMAIL_INVALID_TOKEN,
                                "templatesResetCredentials",
                                  "en",
                                    "defaultLang", json_true(),
                                    "subject", MAIL_SUBJECT,
                                    "body-pattern", MAIL_BODY_PATTERN MAIL_BODY_TOKEN,
                                  "fr",
                                    "defaultLang", json_false(),
                                    "subject", MAIL_SUBJECT_FR,
                                    "body-pattern", MAIL_BODY_PATTERN_FR MAIL_BODY_TOKEN);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_add_mod_reset_credentials_enabled_email_for_token_valid)
{
  json_t * j_body = json_pack("{ss ss ss so s{so so so so so si ss ss ss si ss si s{s{sossss}s{sossss}}}}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "enabled", json_true(),
                              "parameters",
                                "registration", json_false(),
                                "update-email", json_false(),
                                "reset-credentials", json_true(),
                                "reset-credentials-email", json_true(),
                                "reset-credentials-code", json_false(),
                                "reset-credentials-session-duration", SESSION_DURATION,
                                "reset-credentials-session-key", SESSION_RESET_CREDENTIALS_KEY,
                                "reset-credentials-from", MAIL_FROM,
                                "reset-credentials-content-type", MAIL_CONTENT_TYPE,
                                "reset-credentials-token-duration", MAIL_CODE_DURATION,
                                "host", MAIL_HOST,
                                "port", MAIL_PORT_RESET_CREDENTIALS_EMAIL_VALID_TOKEN,
                                "templatesResetCredentials",
                                  "en",
                                    "defaultLang", json_true(),
                                    "subject", MAIL_SUBJECT,
                                    "body-pattern", MAIL_BODY_PATTERN MAIL_BODY_TOKEN,
                                  "fr",
                                    "defaultLang", json_false(),
                                    "subject", MAIL_SUBJECT_FR,
                                    "body-pattern", MAIL_BODY_PATTERN_FR MAIL_BODY_TOKEN);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_add_mod_reset_credentials_enabled_email_flow)
{
  json_t * j_body = json_pack("{ss ss ss so s{so so so so so si ss ss ss si ss si s{s{sossss}s{sossss}}}}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "enabled", json_true(),
                              "parameters",
                                "registration", json_false(),
                                "update-email", json_false(),
                                "reset-credentials", json_true(),
                                "reset-credentials-email", json_true(),
                                "reset-credentials-code", json_false(),
                                "reset-credentials-session-duration", SESSION_DURATION,
                                "reset-credentials-session-key", SESSION_RESET_CREDENTIALS_KEY,
                                "reset-credentials-from", MAIL_FROM,
                                "reset-credentials-content-type", MAIL_CONTENT_TYPE,
                                "reset-credentials-token-duration", MAIL_CODE_DURATION,
                                "host", MAIL_HOST,
                                "port", MAIL_PORT_RESET_CREDENTIALS_EMAIL_FLOW,
                                "templatesResetCredentials",
                                  "en",
                                    "defaultLang", json_true(),
                                    "subject", MAIL_SUBJECT,
                                    "body-pattern", MAIL_BODY_PATTERN MAIL_BODY_TOKEN,
                                  "fr",
                                    "defaultLang", json_false(),
                                    "subject", MAIL_SUBJECT_FR,
                                    "body-pattern", MAIL_BODY_PATTERN_FR MAIL_BODY_TOKEN);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_add_mod_reset_credentials_enabled_code)
{
  json_t * j_body = json_pack("{ss ss ss so s{so so so so so si ss ss si}}",
                              "module", MOD_TYPE,
                              "name", MOD_NAME,
                              "display_name", MOD_DISPLAY_NAME,
                              "enabled", json_true(),
                              "parameters",
                                "registration", json_false(),
                                "update-email", json_false(),
                                "reset-credentials", json_true(),
                                "reset-credentials-email", json_false(),
                                "reset-credentials-code", json_true(),
                                "reset-credentials-session-duration", SESSION_DURATION,
                                "reset-credentials-session-key", SESSION_RESET_CREDENTIALS_KEY,
                                "reset-credentials-code-property", RESET_CREDENTIALS_CODE_PROPERTY,
                                "reset-credentials-code-list-size", RESET_CREDENTIALS_CODE_LIST_SIZE);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_noverify_check_username)
{
  json_t * j_body = json_pack("{}");
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  j_body = json_pack("{si}", "username", 42);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  j_body = json_pack("{ss}", "usernameError", NEW_USERNAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  j_body = json_pack("[{ss}]", "username", NEW_USERNAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  j_body = json_pack("{ss}", "username", USERNAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  j_body = json_pack("{ss}", "username", NEW_USERNAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_profile_empty)
{
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);
}
END_TEST

START_TEST(test_glwd_register_noverify_username_exists)
{
  json_t * j_body;
  
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);

  j_body = json_pack("{ss}", "username", USERNAME_ADMIN);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_register_noverify_cancel_registration)
{
  json_t * j_body;
  struct _u_request req;
  struct _u_response resp;
  int res;
  char * cookie;
  
  ulfius_init_request(&req);
  ulfius_init_response(&resp);

  j_body = json_pack("{ss}", "username", NEW_USERNAME_CANCELLED);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  
  // Registration with the new username
  req.http_url = o_strdup(SERVER_URI "/" MOD_NAME "/register");
  req.http_verb = o_strdup("POST");
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_body), U_OK);
  json_decref(j_body);
  res = ulfius_send_http_request(&req, &resp);
  ck_assert_int_eq(res, U_OK);
  ck_assert_int_eq(resp.status, 200);
  ck_assert_int_eq(resp.nb_cookies, 1);
  cookie = msprintf("%s=%s", resp.map_cookie[0].key, resp.map_cookie[0].value);
  ck_assert_ptr_ne(cookie, NULL);
  u_map_put(req.map_header, "Cookie", cookie);
  o_free(cookie);
  ulfius_clean_response(&resp);
  
  // Set password
  j_body = json_pack("{ss}", "password", NEW_PASSWORD);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/password", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify canuse response is 401 for scheme mock
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME_CANCELLED);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 402, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Register scheme mock
  j_body = json_pack("{sssssss{so}}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME_CANCELLED, "value", "register", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/scheme/register", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify canuse response is 200 for scheme mock
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME_CANCELLED);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Cancel registration
  ck_assert_int_eq(run_simple_test(&req, "DELETE", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
  
  // Verify session is disabled
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);

  ulfius_clean_request(&req);
}
END_TEST

START_TEST(test_glwd_register_noverify_registration_error_session)
{
  json_t * j_body, * j_response;
  struct _u_request req;
  struct _u_response resp;
  int res;
  char * cookie, * cookie_invalid;
  
  ulfius_init_request(&req);
  ulfius_init_response(&resp);

  j_body = json_pack("{ss}", "username", NEW_USERNAME_INVALID_SESSION);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  
  // Registration with the new username
  req.http_url = o_strdup(SERVER_URI "/" MOD_NAME "/register");
  req.http_verb = o_strdup("POST");
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_body), U_OK);
  json_decref(j_body);
  res = ulfius_send_http_request(&req, &resp);
  ck_assert_int_eq(res, U_OK);
  ck_assert_int_eq(resp.status, 200);
  ck_assert_int_eq(resp.nb_cookies, 1);
  cookie = msprintf("%s=%s", resp.map_cookie[0].key, resp.map_cookie[0].value);
  cookie_invalid = msprintf("%s=%s", resp.map_cookie[0].key, (resp.map_cookie[0].value+1));
  ck_assert_ptr_ne(cookie, NULL);
  u_map_put(req.map_header, "Cookie", cookie);
  ulfius_clean_response(&resp);
  
  // Check profile
  j_response = json_pack("{sssososo}", "username", NEW_USERNAME_INVALID_SESSION, "name", json_null(), "email", json_null(), "password_set", json_false());
  ck_assert_ptr_ne(j_response, NULL);
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 200, j_response, NULL, NULL), 1);
  json_decref(j_response);
  
  // Change cookie value
  u_map_remove_from_key(req.map_header, "Cookie");
  u_map_put(req.map_header, "Cookie", cookie_invalid);
  
  // Check profile invalid
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);
  
  // Complete registration invalid
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/complete", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);
  
  // Update name invalid
  j_body = json_pack("{ss}", "name", NEW_NAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Set password invalid
  j_body = json_pack("{ss}", "password", NEW_PASSWORD);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/password", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
    // Check canuse invalid
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME_INVALID_SESSION);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  json_decref(j_body);

  // Cancel registration invalid
  ck_assert_int_eq(run_simple_test(&req, "DELETE", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);
  
  // Change cookie value
  u_map_remove_from_key(req.map_header, "Cookie");
  u_map_put(req.map_header, "Cookie", cookie);
  
  // Cancel registration
  ck_assert_int_eq(run_simple_test(&req, "DELETE", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
  
  o_free(cookie_invalid);
  o_free(cookie);
  ulfius_clean_request(&req);
}
END_TEST

START_TEST(test_glwd_register_noverify_full_registration)
{
  json_t * j_body, * j_response;
  struct _u_request req;
  struct _u_response resp;
  int res;
  char * cookie;
  
  ck_assert_int_eq(run_simple_test(NULL, "GET", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);

  j_body = json_pack("{ss}", "username", NEW_USERNAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  ulfius_init_request(&req);
  ulfius_init_response(&resp);

  // Registration with input errors
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/register", NULL, NULL, NULL, NULL, 400, NULL, NULL, NULL), 1);
  j_body = json_pack("{si}", "username", 42);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/register", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  j_body = json_pack("[{ss}]", "username", NEW_USERNAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/register", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  j_body = json_pack("{ss}", "usernameError", NEW_USERNAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/register", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Registration with the new username
  req.http_url = o_strdup(SERVER_URI "/" MOD_NAME "/register");
  req.http_verb = o_strdup("POST");
  j_body = json_pack("{ss}", "username", NEW_USERNAME);
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_body), U_OK);
  json_decref(j_body);
  res = ulfius_send_http_request(&req, &resp);
  ck_assert_int_eq(res, U_OK);
  ck_assert_int_eq(resp.status, 200);
  ck_assert_int_eq(resp.nb_cookies, 1);
  cookie = msprintf("%s=%s", resp.map_cookie[0].key, resp.map_cookie[0].value);
  ck_assert_ptr_ne(cookie, NULL);
  u_map_put(req.map_header, "Cookie", cookie);
  o_free(cookie);
  ulfius_clean_response(&resp);
  
  // Complete registration impossible
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/complete", NULL, NULL, NULL, NULL, 400, NULL, NULL, NULL), 1);
  
  // Check profile
  j_response = json_pack("{sssososo}", "username", NEW_USERNAME, "name", json_null(), "email", json_null(), "password_set", json_false());
  ck_assert_ptr_ne(j_response, NULL);
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 200, j_response, NULL, NULL), 1);
  json_decref(j_response);
  
  // Update profile with input errors
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 400, NULL, NULL, NULL), 1);
  j_body = json_pack("{si}", "name", 42);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  j_body = json_pack("[{ss}]", "name", NEW_NAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  j_body = json_pack("{ss}", "nameError", NEW_NAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Update name
  j_body = json_pack("{ss}", "name", NEW_NAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Check profile
  j_response = json_pack("{sssssoso}", "username", NEW_USERNAME, "name", NEW_NAME, "email", json_null(), "password_set", json_false());
  ck_assert_ptr_ne(j_response, NULL);
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 200, j_response, NULL, NULL), 1);
  json_decref(j_response);
  
  // Complete registration impossible
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/complete", NULL, NULL, NULL, NULL, 400, NULL, NULL, NULL), 1);
  
  // Check profile agin with new name
  j_response = json_pack("{sssssoso}", "username", NEW_USERNAME, "name", NEW_NAME, "email", json_null(), "password_set", json_false());
  ck_assert_ptr_ne(j_response, NULL);
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 200, j_response, NULL, NULL), 1);
  json_decref(j_response);
  
  // Set password with input errors
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/password", NULL, NULL, NULL, NULL, 400, NULL, NULL, NULL), 1);
  j_body = json_pack("{si}", "password", 42);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/password", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  j_body = json_pack("[{ss}]", "password", NEW_PASSWORD);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/password", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  j_body = json_pack("{ss}", "passwordError", NEW_PASSWORD);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/password", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Set password
  j_body = json_pack("{ss}", "password", NEW_PASSWORD);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/password", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Check profile
  j_response = json_pack("{sssssoso}", "username", NEW_USERNAME, "name", NEW_NAME, "email", json_null(), "password_set", json_true());
  ck_assert_ptr_ne(j_response, NULL);
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 200, j_response, NULL, NULL), 1);
  json_decref(j_response);
  
  // Complete registration impossible
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/complete", NULL, NULL, NULL, NULL, 400, NULL, NULL, NULL), 1);
  
  // Check canuse with input errors
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, NULL, NULL, 400, NULL, NULL, NULL), 1);
  j_body = json_pack("{sssssi}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", 42);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  j_body = json_pack("{sissss}", "scheme_name", 42, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify canuse response is 401 for scheme mock
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 402, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Register scheme mock with input errors
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/scheme/register", NULL, NULL, NULL, NULL, 400, NULL, NULL, NULL), 1);
  j_body = json_pack("{sssssis{so}}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", 42, "value", "register", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/scheme/register", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  j_body = json_pack("{sisssss{so}}", "scheme_name", 42, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME, "value", "register", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/scheme/register", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  j_body = json_pack("{sssssss{si}}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME, "value", "register", 42);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/scheme/register", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Register scheme mock
  j_body = json_pack("{sssssss{so}}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME, "value", "register", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/scheme/register", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify canuse response is 200 for scheme mock
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Complete registration
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/complete", NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
  
  // Test authentication with new user
  j_body = json_pack("{ssss}", "username", NEW_USERNAME, "password", NEW_PASSWORD);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/auth", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  ulfius_clean_request(&req);
}
END_TEST

START_TEST(test_glwd_register_verify_with_username_cancel_registration)
{
  struct smtp_manager manager;
  json_t * j_body;
  struct _u_request req;
  struct _u_response resp;
  int res, i;
  char * cookie, error_code[MAIL_CODE_LEGTH+1];
  pthread_t thread;

  manager.mail_data = NULL;
  manager.port = MAIL_PORT_WITH_USERNAME;
  manager.sockfd = 0;
  manager.body_pattern = MAIL_BODY_PATTERN;
  pthread_create(&thread, NULL, simple_smtp, &manager);

  ulfius_init_request(&req);
  ulfius_init_response(&resp);

  j_body = json_pack("{ss}", "username", NEW_USERNAME_CANCELLED);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verification with the new username
  j_body = json_pack("{ssss}", "username", NEW_USERNAME_CANCELLED, "email", NEW_EMAIL);
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/verify", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Send invalid length verification code
  for (i=0; i<MAIL_CODE_LEGTH; i++) {
    error_code[i] = 'e';
  }
  error_code[MAIL_CODE_LEGTH-1] = '\0';
  j_body = json_pack("{ssssss}", "username", NEW_USERNAME_CANCELLED, "email", NEW_EMAIL, "code", error_code);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/verify", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Send invalid verification code
  for (i=0; i<MAIL_CODE_LEGTH; i++) {
    error_code[i] = 'e';
  }
  error_code[MAIL_CODE_LEGTH] = '\0';
  j_body = json_pack("{ssssss}", "username", NEW_USERNAME_CANCELLED, "email", NEW_EMAIL, "code", error_code);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/verify", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Send verification code
  j_body = json_pack("{ssssss}", "username", NEW_USERNAME_CANCELLED, "email", NEW_EMAIL, "code", manager.mail_data);
  req.http_url = o_strdup(SERVER_URI "/" MOD_NAME "/verify");
  req.http_verb = o_strdup("POST");
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_body), U_OK);
  json_decref(j_body);
  res = ulfius_send_http_request(&req, &resp);
  ck_assert_int_eq(res, U_OK);
  ck_assert_int_eq(resp.status, 200);
  ck_assert_int_eq(resp.nb_cookies, 1);
  cookie = msprintf("%s=%s", resp.map_cookie[0].key, resp.map_cookie[0].value);
  ck_assert_ptr_ne(cookie, NULL);
  u_map_put(req.map_header, "Cookie", cookie);
  o_free(cookie);
  ulfius_clean_response(&resp);
  
  pthread_join(thread, NULL);
  
  o_free(manager.mail_data);

  // Set password
  j_body = json_pack("{ss}", "password", NEW_PASSWORD);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/password", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify canuse response is 401 for scheme mock
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME_CANCELLED);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 402, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Register scheme mock
  j_body = json_pack("{sssssss{so}}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME_CANCELLED, "value", "register", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/scheme/register", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify canuse response is 200 for scheme mock
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME_CANCELLED);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Cancel registration
  ck_assert_int_eq(run_simple_test(&req, "DELETE", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
  
  // Verify session is disabled
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);

  ulfius_clean_request(&req);
}
END_TEST

START_TEST(test_glwd_register_verify_without_username_cancel_registration)
{
  struct smtp_manager manager;
  json_t * j_body;
  struct _u_request req;
  struct _u_response resp;
  int res;
  char * cookie;
  pthread_t thread;

  manager.mail_data = NULL;
  manager.port = MAIL_PORT_WITHOUT_USERNAME;
  manager.sockfd = 0;
  manager.body_pattern = MAIL_BODY_PATTERN;
  pthread_create(&thread, NULL, simple_smtp, &manager);

  ulfius_init_request(&req);
  ulfius_init_response(&resp);

  j_body = json_pack("{ss}", "username", NEW_EMAIL);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verification with the new username
  j_body = json_pack("{ss}", "email", NEW_EMAIL);
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/verify", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Send verification code
  j_body = json_pack("{ssss}", "email", NEW_EMAIL, "code", manager.mail_data);
  req.http_url = o_strdup(SERVER_URI "/" MOD_NAME "/verify");
  req.http_verb = o_strdup("POST");
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_body), U_OK);
  json_decref(j_body);
  res = ulfius_send_http_request(&req, &resp);
  ck_assert_int_eq(res, U_OK);
  ck_assert_int_eq(resp.status, 200);
  ck_assert_int_eq(resp.nb_cookies, 1);
  cookie = msprintf("%s=%s", resp.map_cookie[0].key, resp.map_cookie[0].value);
  ck_assert_ptr_ne(cookie, NULL);
  u_map_put(req.map_header, "Cookie", cookie);
  o_free(cookie);
  ulfius_clean_response(&resp);
  
  pthread_join(thread, NULL);
  
  o_free(manager.mail_data);

  // Set password
  j_body = json_pack("{ss}", "password", NEW_PASSWORD);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/password", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify canuse response is 401 for scheme mock
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_EMAIL);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 402, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Register scheme mock
  j_body = json_pack("{sssssss{so}}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_EMAIL, "value", "register", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/scheme/register", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify canuse response is 200 for scheme mock
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_EMAIL);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Cancel registration
  ck_assert_int_eq(run_simple_test(&req, "DELETE", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
  
  // Verify session is disabled
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);

  ulfius_clean_request(&req);
}
END_TEST

START_TEST(test_glwd_register_verify_with_username_token_cancel_registration)
{
  struct smtp_manager manager;
  json_t * j_body;
  struct _u_request req;
  struct _u_response resp;
  int res, i;
  char * cookie, error_token[GLEWLWYD_TOKEN_LENGTH+1];
  pthread_t thread;

  manager.mail_data = NULL;
  manager.port = MAIL_PORT_WITH_USERNAME;
  manager.sockfd = 0;
  manager.body_pattern = MAIL_BODY_PATTERN;
  pthread_create(&thread, NULL, simple_smtp, &manager);

  ulfius_init_request(&req);
  ulfius_init_response(&resp);

  j_body = json_pack("{ss}", "username", NEW_USERNAME_CANCELLED);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verification with the new username
  j_body = json_pack("{ssss}", "username", NEW_USERNAME_CANCELLED, "email", NEW_EMAIL);
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/verify", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Send invalid length verification code
  for (i=0; i<GLEWLWYD_TOKEN_LENGTH; i++) {
    error_token[i] = 'e';
  }
  error_token[GLEWLWYD_TOKEN_LENGTH-1] = '\0';
  j_body = json_pack("{ss}", "token", error_token);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/verify", NULL, NULL, j_body, NULL, 403, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Send invalid verification code
  for (i=0; i<GLEWLWYD_TOKEN_LENGTH; i++) {
    error_token[i] = 'e';
  }
  error_token[GLEWLWYD_TOKEN_LENGTH] = '\0';
  j_body = json_pack("{ss}", "token", error_token);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/verify", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Send verification code
  j_body = json_pack("{ss}", "token", manager.mail_data);
  req.http_url = o_strdup(SERVER_URI "/" MOD_NAME "/verify");
  req.http_verb = o_strdup("POST");
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_body), U_OK);
  json_decref(j_body);
  res = ulfius_send_http_request(&req, &resp);
  ck_assert_int_eq(res, U_OK);
  ck_assert_int_eq(resp.status, 200);
  ck_assert_int_eq(resp.nb_cookies, 1);
  cookie = msprintf("%s=%s", resp.map_cookie[0].key, resp.map_cookie[0].value);
  ck_assert_ptr_ne(cookie, NULL);
  u_map_put(req.map_header, "Cookie", cookie);
  o_free(cookie);
  ulfius_clean_response(&resp);
  
  pthread_join(thread, NULL);
  
  o_free(manager.mail_data);

  // Set password
  j_body = json_pack("{ss}", "password", NEW_PASSWORD);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/password", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify canuse response is 401 for scheme mock
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME_CANCELLED);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 402, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Register scheme mock
  j_body = json_pack("{sssssss{so}}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME_CANCELLED, "value", "register", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/scheme/register", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify canuse response is 200 for scheme mock
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_USERNAME_CANCELLED);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Cancel registration
  ck_assert_int_eq(run_simple_test(&req, "DELETE", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
  
  // Verify session is disabled
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);

  ulfius_clean_request(&req);
}
END_TEST

START_TEST(test_glwd_register_noverify_session_expired)
{
  json_t * j_body;
  struct _u_request req;
  struct _u_response resp;
  char * cookie;
  
  ulfius_init_request(&req);
  ulfius_init_response(&resp);

  j_body = json_pack("{ss}", "username", NEW_USERNAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  
  // Registration with the new username
  req.http_url = o_strdup(SERVER_URI "/" MOD_NAME "/register");
  req.http_verb = o_strdup("POST");
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_body), U_OK);
  json_decref(j_body);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  ck_assert_int_eq(resp.nb_cookies, 1);
  cookie = msprintf("%s=%s", resp.map_cookie[0].key, resp.map_cookie[0].value);
  ck_assert_ptr_ne(cookie, NULL);
  u_map_put(req.map_header, "Cookie", cookie);
  o_free(cookie);
  ulfius_clean_response(&resp);
  
  // Set password
  j_body = json_pack("{ss}", "password", NEW_PASSWORD);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/password", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  sleep(6);
  
  // Set password after session expiration
  j_body = json_pack("{ss}", "password", NEW_PASSWORD);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/password", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify session is disabled
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);

  ulfius_clean_request(&req);
  
  ck_assert_int_eq(run_simple_test(&admin_req, "DELETE", SERVER_URI "/user/" NEW_USERNAME, NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
}
END_TEST

START_TEST(test_glwd_register_verify_without_username_code_expired)
{
  struct smtp_manager manager;
  json_t * j_body;
  struct _u_request req;
  struct _u_response resp;
  pthread_t thread;

  manager.mail_data = NULL;
  manager.port = MAIL_PORT_CODE_EXPIRED;
  manager.sockfd = 0;
  manager.body_pattern = MAIL_BODY_PATTERN;
  pthread_create(&thread, NULL, simple_smtp, &manager);

  ulfius_init_request(&req);
  ulfius_init_response(&resp);

  j_body = json_pack("{ss}", "username", NEW_EMAIL);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verification with the new username
  j_body = json_pack("{ss}", "email", NEW_EMAIL);
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/verify", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  sleep(6);
  
  // Send verification code after code expiration
  j_body = json_pack("{ssss}", "email", NEW_EMAIL, "code", manager.mail_data);
  req.http_url = o_strdup(SERVER_URI "/" MOD_NAME "/verify");
  req.http_verb = o_strdup("POST");
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_body), U_OK);
  json_decref(j_body);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 401);
  ck_assert_int_eq(resp.nb_cookies, 0);
  ulfius_clean_response(&resp);
  
  pthread_join(thread, NULL);
  
  o_free(manager.mail_data);

  ulfius_clean_request(&req);
}
END_TEST

START_TEST(test_glwd_register_delete_mod)
{
  ck_assert_int_eq(run_simple_test(&admin_req, "DELETE", SERVER_URI "/mod/plugin/" MOD_NAME, NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
}
END_TEST

START_TEST(test_glwd_register_delete_new_user_with_scheme)
{
  json_t * j_body = json_pack("{sssssss{so}}", "username", NEW_USERNAME, "scheme_type", SCHEME_TYPE, "scheme_name", SCHEME_NAME, "value", "register", json_false());
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/delegate/" NEW_USERNAME "/profile/scheme/register/", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  ck_assert_int_eq(run_simple_test(&admin_req, "DELETE", SERVER_URI "/user/" NEW_USERNAME, NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
}
END_TEST

START_TEST(test_glwd_register_verify_without_username_multilang_fr_cancel_registration)
{
  struct smtp_manager manager;
  json_t * j_body;
  struct _u_request req;
  struct _u_response resp;
  int res;
  char * cookie;
  pthread_t thread;

  manager.mail_data = NULL;
  manager.port = MAIL_PORT_MULTILANG;
  manager.sockfd = 0;
  manager.body_pattern = MAIL_BODY_PATTERN_FR;
  pthread_create(&thread, NULL, simple_smtp, &manager);

  ulfius_init_request(&req);
  ulfius_init_response(&resp);

  j_body = json_pack("{ss}", "username", NEW_EMAIL);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verification with the new username
  j_body = json_pack("{ssss}", "email", NEW_EMAIL, "lang", "fr");
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/verify", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Send verification code
  j_body = json_pack("{ssss}", "email", NEW_EMAIL, "code", manager.mail_data);
  req.http_url = o_strdup(SERVER_URI "/" MOD_NAME "/verify");
  req.http_verb = o_strdup("POST");
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_body), U_OK);
  json_decref(j_body);
  res = ulfius_send_http_request(&req, &resp);
  ck_assert_int_eq(res, U_OK);
  ck_assert_int_eq(resp.status, 200);
  ck_assert_int_eq(resp.nb_cookies, 1);
  cookie = msprintf("%s=%s", resp.map_cookie[0].key, resp.map_cookie[0].value);
  ck_assert_ptr_ne(cookie, NULL);
  u_map_put(req.map_header, "Cookie", cookie);
  o_free(cookie);
  ulfius_clean_response(&resp);
  
  pthread_join(thread, NULL);
  
  o_free(manager.mail_data);

  // Set password
  j_body = json_pack("{ss}", "password", NEW_PASSWORD);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/password", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify canuse response is 401 for scheme mock
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_EMAIL);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 402, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Register scheme mock
  j_body = json_pack("{sssssss{so}}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_EMAIL, "value", "register", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/scheme/register", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify canuse response is 200 for scheme mock
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_EMAIL);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Cancel registration
  ck_assert_int_eq(run_simple_test(&req, "DELETE", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
  
  // Verify session is disabled
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);

  ulfius_clean_request(&req);
}
END_TEST

START_TEST(test_glwd_register_verify_without_username_multilang_default_cancel_registration)
{
  struct smtp_manager manager;
  json_t * j_body;
  struct _u_request req;
  struct _u_response resp;
  int res;
  char * cookie;
  pthread_t thread;

  manager.mail_data = NULL;
  manager.port = MAIL_PORT_MULTILANG;
  manager.sockfd = 0;
  manager.body_pattern = MAIL_BODY_PATTERN;
  pthread_create(&thread, NULL, simple_smtp, &manager);

  ulfius_init_request(&req);
  ulfius_init_response(&resp);

  j_body = json_pack("{ss}", "username", NEW_EMAIL);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verification with the new username
  j_body = json_pack("{ss}", "email", NEW_EMAIL);
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/verify", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Send verification code
  j_body = json_pack("{ssss}", "email", NEW_EMAIL, "code", manager.mail_data);
  req.http_url = o_strdup(SERVER_URI "/" MOD_NAME "/verify");
  req.http_verb = o_strdup("POST");
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_body), U_OK);
  json_decref(j_body);
  res = ulfius_send_http_request(&req, &resp);
  ck_assert_int_eq(res, U_OK);
  ck_assert_int_eq(resp.status, 200);
  ck_assert_int_eq(resp.nb_cookies, 1);
  cookie = msprintf("%s=%s", resp.map_cookie[0].key, resp.map_cookie[0].value);
  ck_assert_ptr_ne(cookie, NULL);
  u_map_put(req.map_header, "Cookie", cookie);
  o_free(cookie);
  ulfius_clean_response(&resp);
  
  pthread_join(thread, NULL);
  
  o_free(manager.mail_data);

  // Set password
  j_body = json_pack("{ss}", "password", NEW_PASSWORD);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/password", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify canuse response is 401 for scheme mock
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_EMAIL);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 402, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Register scheme mock
  j_body = json_pack("{sssssss{so}}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_EMAIL, "value", "register", json_true());
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/profile/scheme/register", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Verify canuse response is 200 for scheme mock
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", NEW_EMAIL);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Cancel registration
  ck_assert_int_eq(run_simple_test(&req, "DELETE", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
  
  // Verify session is disabled
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);

  ulfius_clean_request(&req);
}
END_TEST

START_TEST(test_glwd_register_noverify_registration_disabled)
{
  json_t * j_body;
  struct _u_request req;
  
  ulfius_init_request(&req);

  j_body = json_pack("{ss}", "username", NEW_USERNAME_CANCELLED);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/username", NULL, NULL, j_body, NULL, 404, NULL, NULL, NULL), 1);
  
  ulfius_clean_request(&req);
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_update_email_invalid_params)
{
  json_t * j_body;
  
  j_body = json_pack("{si}", "email", 42);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&user_req, "POST", SERVER_URI "/" MOD_NAME "/update-email", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&user_req, "POST", SERVER_URI "/" MOD_NAME "/update-email", NULL, NULL, NULL, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  j_body = json_pack("{ss}", "emailerror", NEW_EMAIL);
  ck_assert_int_eq(run_simple_test(&user_req, "POST", SERVER_URI "/" MOD_NAME "/update-email", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_update_email_invalid_token)
{
  json_t * j_body;
  struct smtp_manager manager;
  pthread_t thread;
  char * url;

  manager.mail_data = NULL;
  manager.port = MAIL_PORT_UPDATE_EMAIL_INVALID_TOKEN;
  manager.sockfd = 0;
  manager.body_pattern = MAIL_BODY_PATTERN;
  pthread_create(&thread, NULL, simple_smtp, &manager);
  
  j_body = json_pack("{ss}", "email", NEW_EMAIL);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&user_req, "POST", SERVER_URI "/" MOD_NAME "/update-email", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  
  ck_assert_int_eq(run_simple_test(&user_req, "PUT", SERVER_URI "/" MOD_NAME "/update-email/error", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  url = msprintf(SERVER_URI "/" MOD_NAME "/update-email/%s", manager.mail_data);
  url[o_strlen(url)-2] = '-';
  ck_assert_int_eq(run_simple_test(&user_req, "PUT", url, NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  o_free(url);
  
  json_decref(j_body);
  
  pthread_join(thread, NULL);
  
  o_free(manager.mail_data);
}
END_TEST

START_TEST(test_glwd_update_email_valid_token)
{
  json_t * j_body, * j_user;
  struct smtp_manager manager;
  pthread_t thread;
  char * url;
  struct _u_request req;
  struct _u_response resp;

  manager.mail_data = NULL;
  manager.port = MAIL_PORT_UPDATE_EMAIL_VALID_TOKEN;
  manager.sockfd = 0;
  manager.body_pattern = MAIL_BODY_PATTERN;
  pthread_create(&thread, NULL, simple_smtp, &manager);
  
  j_body = json_pack("{ss}", "email", NEW_EMAIL);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(&user_req, "POST", SERVER_URI "/" MOD_NAME "/update-email", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  j_user = json_pack("{ssss}", "username", USERNAME, "email", EMAIL);
  ck_assert_ptr_ne(j_user, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "GET", SERVER_URI "/user/" USERNAME, NULL, NULL, NULL, NULL, 200, j_user, NULL, NULL), 1);
  json_decref(j_user);
  
  url = msprintf(SERVER_URI "/" MOD_NAME "/update-email/%s", manager.mail_data);
  ck_assert_int_eq(run_simple_test(&user_req, "PUT", url, NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
  o_free(url);
  
  j_user = json_pack("{ssss}", "username", USERNAME, "email", NEW_EMAIL);
  ck_assert_ptr_ne(j_user, NULL);
  ck_assert_int_eq(run_simple_test(&admin_req, "GET", SERVER_URI "/user/" USERNAME, NULL, NULL, NULL, NULL, 200, j_user, NULL, NULL), 1);
  json_decref(j_user);
  
  pthread_join(thread, NULL);
  
  o_free(manager.mail_data);
  
  ulfius_init_request(&req);
  ulfius_init_response(&resp);
  ulfius_copy_request(&req, &admin_req);
  ulfius_set_request_properties(&req, U_OPT_HTTP_VERB, "GET", U_OPT_HTTP_URL, SERVER_URI "/user/" USERNAME, U_OPT_NONE);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  j_user = ulfius_get_json_body_response(&resp, NULL);
  ulfius_clean_response(&resp);
  
  ulfius_init_response(&resp);
  json_object_set_new(j_user, "email", json_string(EMAIL));
  ulfius_set_request_properties(&req, U_OPT_HTTP_VERB, "PUT", U_OPT_JSON_BODY, j_user, U_OPT_NONE);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  ulfius_clean_request(&req);
  ulfius_clean_response(&resp);
  json_decref(j_user);
}
END_TEST

START_TEST(test_glwd_reset_credentials_email_invalid_params)
{
  json_t * j_body;
  
  j_body = json_pack("{si}", "username", 42);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials-email", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials-email", NULL, NULL, NULL, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  j_body = json_pack("{ss}", "usernameerror", USERNAME);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials-email", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  
  json_decref(j_body);
}
END_TEST

START_TEST(test_glwd_reset_credentials_email_invalid_token)
{
  json_t * j_body;
  struct smtp_manager manager;
  pthread_t thread;
  char * url;

  manager.mail_data = NULL;
  manager.port = MAIL_PORT_RESET_CREDENTIALS_EMAIL_INVALID_TOKEN;
  manager.sockfd = 0;
  manager.body_pattern = MAIL_BODY_PATTERN;
  pthread_create(&thread, NULL, simple_smtp, &manager);
  
  j_body = json_pack("{ss}", "username", USERNAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials-email", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials-email/error", NULL, NULL, j_body, NULL, 403, NULL, NULL, NULL), 1);
  url = msprintf(SERVER_URI "/" MOD_NAME "/reset-credentials-email/%s", manager.mail_data);
  url[o_strlen(url)-2] = '-';
  ck_assert_int_eq(run_simple_test(NULL, "PUT", url, NULL, NULL, j_body, NULL, 403, NULL, NULL, NULL), 1);
  o_free(url);
  
  json_decref(j_body);
  
  pthread_join(thread, NULL);
  
  o_free(manager.mail_data);
}
END_TEST

START_TEST(test_glwd_reset_credentials_email_valid_token)
{
  json_t * j_body;
  struct smtp_manager manager;
  pthread_t thread;
  char * url;

  manager.mail_data = NULL;
  manager.port = MAIL_PORT_RESET_CREDENTIALS_EMAIL_VALID_TOKEN;
  manager.sockfd = 0;
  manager.body_pattern = MAIL_BODY_PATTERN;
  pthread_create(&thread, NULL, simple_smtp, &manager);
  
  j_body = json_pack("{ss}", "username", USERNAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials-email", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  url = msprintf(SERVER_URI "/" MOD_NAME "/reset-credentials-email/%s", manager.mail_data);
  ck_assert_int_eq(run_simple_test(NULL, "PUT", url, NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
  o_free(url);
  
  pthread_join(thread, NULL);
  
  o_free(manager.mail_data);
  
}
END_TEST

START_TEST(test_glwd_reset_credentials_email_flow)
{
  json_t * j_body;
  struct smtp_manager manager;
  pthread_t thread;
  char * url, * cookie;
  struct _u_request req;
  struct _u_response resp;

  manager.mail_data = NULL;
  manager.port = MAIL_PORT_RESET_CREDENTIALS_EMAIL_VALID_TOKEN;
  manager.sockfd = 0;
  manager.body_pattern = MAIL_BODY_PATTERN;
  pthread_create(&thread, NULL, simple_smtp, &manager);
  
  j_body = json_pack("{ss}", "username", USERNAME);
  ck_assert_ptr_ne(j_body, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials-email", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  ulfius_init_request(&req);
  ulfius_init_response(&resp);
  url = msprintf(SERVER_URI "/" MOD_NAME "/reset-credentials-email/%s", manager.mail_data);
  ulfius_set_request_properties(&req, U_OPT_HTTP_VERB, "PUT", U_OPT_HTTP_URL, url, U_OPT_NONE);
  o_free(url);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  ck_assert_int_eq(resp.nb_cookies, 1);
  ulfius_clean_request(&req);
  
  ulfius_init_request(&req);
  cookie = msprintf("%s=%s", resp.map_cookie[0].key, resp.map_cookie[0].value);
  u_map_put(req.map_header, "Cookie", cookie);
  o_free(cookie);
  ulfius_clean_response(&resp);
  
  // Test profile/password endpoint
  j_body = json_pack("{ss}", "password", NEW_PASSWORD);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/password", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/password", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Test profile endpoint
  j_body = json_string(USERNAME);
  ck_assert_int_eq(run_simple_test(NULL, "GET", SERVER_URI "/" MOD_NAME "/reset-credentials/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/reset-credentials/profile", NULL, NULL, NULL, NULL, 200, j_body, NULL, NULL), 1);
  json_decref(j_body);
  
  // Test PUT profile/scheme/register endpoint
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", USERNAME);
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Test PUT profile/scheme/register/canuse endpoint
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", USERNAME);
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 402, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Test POST profile/scheme/register endpoint
  j_body = json_pack("{sssssss{so}}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", USERNAME, "value", "register", json_true());
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Test PUT profile/scheme/register endpoint
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", USERNAME);
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Test PUT profile/scheme/register/canuse endpoint
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", USERNAME);
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Reset password
  j_body = json_pack("{ss}", "password", PASSWORD);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/password", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Reset scheme
  j_body = json_pack("{sssssss{so}}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", USERNAME, "value", "register", json_false());
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Test POST profile/complete endpoint
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/complete", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/complete", NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/reset-credentials/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);
  
  ulfius_clean_request(&req);
  pthread_join(thread, NULL);
  
  o_free(manager.mail_data);
  
}
END_TEST

START_TEST(test_glwd_reset_credentials_code_reset)
{
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials-code", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&user_req, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials-code", NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
}
END_TEST

START_TEST(test_glwd_reset_credentials_code_verify_invalid)
{
  struct _u_request req;
  struct _u_response resp;
  json_t * j_code, * j_body;
  char * code, * code2;
  
  ulfius_init_request(&req);
  ulfius_init_response(&resp);
  
  ulfius_set_request_properties(&req, U_OPT_HTTP_VERB, "PUT", U_OPT_HTTP_URL, SERVER_URI "/" MOD_NAME "/reset-credentials-code", U_OPT_HEADER_PARAMETER, "Cookie", u_map_get(user_req.map_header, "Cookie"), U_OPT_NONE);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  ck_assert_ptr_ne(j_code = ulfius_get_json_body_response(&resp, NULL), NULL);
  ck_assert_int_eq(RESET_CREDENTIALS_CODE_LIST_SIZE, json_array_size(j_code));
  
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials-code", NULL, NULL, NULL, NULL, 403, NULL, NULL, NULL), 1);
  
  j_body = json_pack("{ssss}", "username", USERNAME, "code", "error");
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials-code", NULL, NULL, j_body, NULL, 403, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  ck_assert_ptr_ne(NULL, code = o_strdup(json_string_value(json_array_get(j_code, 0))));
  ck_assert_ptr_ne(NULL, code2 = o_strdup(json_string_value(json_array_get(j_code, 1))));
  code[o_strlen(code)-2] = '-';

  j_body = json_pack("{ssss}", "username", USERNAME, "code", code);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials-code", NULL, NULL, j_body, NULL, 403, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  json_decref(j_code);
  ulfius_clean_response(&resp);
  ulfius_init_response(&resp);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  ck_assert_ptr_ne(j_code = ulfius_get_json_body_response(&resp, NULL), NULL);
  ck_assert_int_eq(RESET_CREDENTIALS_CODE_LIST_SIZE, json_array_size(j_code));
  
  j_body = json_pack("{ssss}", "username", USERNAME, "code", code2);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials-code", NULL, NULL, j_body, NULL, 403, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  json_decref(j_code);
  ulfius_clean_request(&req);
  ulfius_clean_response(&resp);
  o_free(code);
  o_free(code2);
}
END_TEST

START_TEST(test_glwd_reset_credentials_code_verify_valid)
{
  struct _u_request req;
  struct _u_response resp;
  json_t * j_code, * j_body;
  
  ulfius_init_request(&req);
  ulfius_init_response(&resp);
  
  ulfius_set_request_properties(&req, U_OPT_HTTP_VERB, "PUT", U_OPT_HTTP_URL, SERVER_URI "/" MOD_NAME "/reset-credentials-code", U_OPT_HEADER_PARAMETER, "Cookie", u_map_get(user_req.map_header, "Cookie"), U_OPT_NONE);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  ck_assert_ptr_ne(j_code = ulfius_get_json_body_response(&resp, NULL), NULL);
  ck_assert_int_eq(RESET_CREDENTIALS_CODE_LIST_SIZE, json_array_size(j_code));
  
  j_body = json_pack("{sssO}", "username", USERNAME, "code", json_array_get(j_code, 0));
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials-code", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  json_decref(j_code);
  ulfius_clean_request(&req);
  ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_glwd_reset_credentials_code_flow)
{
  json_t * j_body, * j_code;
  char * cookie;
  struct _u_request req;
  struct _u_response resp;

  ulfius_init_request(&req);
  ulfius_init_response(&resp);
  
  ulfius_set_request_properties(&req, U_OPT_HTTP_VERB, "PUT", U_OPT_HTTP_URL, SERVER_URI "/" MOD_NAME "/reset-credentials-code", U_OPT_HEADER_PARAMETER, "Cookie", u_map_get(user_req.map_header, "Cookie"), U_OPT_NONE);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  ck_assert_ptr_ne(j_code = ulfius_get_json_body_response(&resp, NULL), NULL);
  ck_assert_int_eq(RESET_CREDENTIALS_CODE_LIST_SIZE, json_array_size(j_code));
  ulfius_clean_response(&resp);
  ulfius_clean_request(&req);
  
  ulfius_init_request(&req);
  ulfius_init_response(&resp);
  j_body = json_pack("{sssO}", "username", USERNAME, "code", json_array_get(j_code, 0));
  ulfius_set_request_properties(&req, U_OPT_HTTP_VERB, "POST", U_OPT_HTTP_URL, SERVER_URI "/" MOD_NAME "/reset-credentials-code", U_OPT_JSON_BODY, j_body, U_OPT_NONE);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  json_decref(j_body);
  ulfius_clean_request(&req);
  
  ulfius_init_request(&req);
  cookie = msprintf("%s=%s", resp.map_cookie[0].key, resp.map_cookie[0].value);
  u_map_put(req.map_header, "Cookie", cookie);
  o_free(cookie);
  ulfius_clean_response(&resp);
  
  // Test profile/password endpoint
  j_body = json_pack("{ss}", "password", NEW_PASSWORD);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/password", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/password", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Test profile endpoint
  j_body = json_string(USERNAME);
  ck_assert_int_eq(run_simple_test(NULL, "GET", SERVER_URI "/" MOD_NAME "/reset-credentials/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/reset-credentials/profile", NULL, NULL, NULL, NULL, 200, j_body, NULL, NULL), 1);
  json_decref(j_body);
  
  // Test PUT profile/scheme/register endpoint
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", USERNAME);
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register", NULL, NULL, j_body, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Test PUT profile/scheme/register/canuse endpoint
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", USERNAME);
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 402, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Test POST profile/scheme/register endpoint
  j_body = json_pack("{sssssss{so}}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", USERNAME, "value", "register", json_true());
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Test PUT profile/scheme/register endpoint
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", USERNAME);
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Test PUT profile/scheme/register/canuse endpoint
  j_body = json_pack("{ssssss}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", USERNAME);
  ck_assert_int_eq(run_simple_test(NULL, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "PUT", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register/canuse", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Reset password
  j_body = json_pack("{ss}", "password", PASSWORD);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/password", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Reset scheme
  j_body = json_pack("{sssssss{so}}", "scheme_name", SCHEME_NAME, "scheme_type", SCHEME_TYPE, "username", USERNAME, "value", "register", json_false());
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/scheme/register", NULL, NULL, j_body, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_body);
  
  // Test POST profile/complete endpoint
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/complete", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "POST", SERVER_URI "/" MOD_NAME "/reset-credentials/profile/complete", NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&req, "GET", SERVER_URI "/" MOD_NAME "/reset-credentials/profile", NULL, NULL, NULL, NULL, 401, NULL, NULL, NULL), 1);
  
  ulfius_clean_request(&req);
  json_decref(j_code);
}
END_TEST

static Suite *glewlwyd_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Glewlwyd register new account");
  tc_core = tcase_create("test_glwd_register_new_account");
  tcase_add_test(tc_core, test_glwd_register_add_mod_error);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_add_test(tc_core, test_glwd_register_add_mod_noverify);
  tcase_add_test(tc_core, test_glwd_register_noverify_check_username);
  tcase_add_test(tc_core, test_glwd_register_profile_empty);
  tcase_add_test(tc_core, test_glwd_register_noverify_username_exists);
  tcase_add_test(tc_core, test_glwd_register_noverify_cancel_registration);
  tcase_add_test(tc_core, test_glwd_register_noverify_registration_error_session);
  tcase_add_test(tc_core, test_glwd_register_noverify_full_registration);
  tcase_add_test(tc_core, test_glwd_register_delete_new_user_with_scheme);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_add_test(tc_core, test_glwd_register_add_mod_verify_with_username);
  tcase_add_test(tc_core, test_glwd_register_verify_with_username_cancel_registration);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_add_test(tc_core, test_glwd_register_add_mod_verify_without_username);
  tcase_add_test(tc_core, test_glwd_register_verify_without_username_cancel_registration);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_add_test(tc_core, test_glwd_register_add_mod_noverify_session_expired);
  tcase_add_test(tc_core, test_glwd_register_noverify_session_expired);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_add_test(tc_core, test_glwd_register_add_mod_verify_without_username_code_expired);
  tcase_add_test(tc_core, test_glwd_register_verify_without_username_code_expired);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_add_test(tc_core, test_glwd_register_add_mod_verify_with_username_token);
  tcase_add_test(tc_core, test_glwd_register_verify_with_username_token_cancel_registration);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_add_test(tc_core, test_glwd_register_add_mod_verify_multilang_without_username);
  tcase_add_test(tc_core, test_glwd_register_verify_without_username_multilang_fr_cancel_registration);
  tcase_add_test(tc_core, test_glwd_register_verify_without_username_multilang_default_cancel_registration);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_add_test(tc_core, test_glwd_register_add_mod_noverify_registration_enabled);
  tcase_add_test(tc_core, test_glwd_register_noverify_cancel_registration);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_add_test(tc_core, test_glwd_register_add_mod_registration_disabled_update_email_enabled_for_token_invalid);
  tcase_add_test(tc_core, test_glwd_register_noverify_registration_disabled);
  tcase_add_test(tc_core, test_glwd_update_email_invalid_params);
  tcase_add_test(tc_core, test_glwd_update_email_invalid_token);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_add_test(tc_core, test_glwd_register_add_mod_registration_disabled_update_email_enabled_for_token_valid);
  tcase_add_test(tc_core, test_glwd_update_email_valid_token);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_add_test(tc_core, test_glwd_register_add_mod_reset_credentials_enabled_email_for_token_invalid);
  tcase_add_test(tc_core, test_glwd_reset_credentials_email_invalid_params);
  tcase_add_test(tc_core, test_glwd_reset_credentials_email_invalid_token);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_add_test(tc_core, test_glwd_register_add_mod_reset_credentials_enabled_email_for_token_valid);
  tcase_add_test(tc_core, test_glwd_reset_credentials_email_valid_token);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_add_test(tc_core, test_glwd_register_add_mod_reset_credentials_enabled_email_flow);
  tcase_add_test(tc_core, test_glwd_reset_credentials_email_flow);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_add_test(tc_core, test_glwd_register_add_mod_reset_credentials_enabled_code);
  tcase_add_test(tc_core, test_glwd_reset_credentials_code_reset);
  tcase_add_test(tc_core, test_glwd_reset_credentials_code_verify_invalid);
  tcase_add_test(tc_core, test_glwd_reset_credentials_code_verify_valid);
  tcase_add_test(tc_core, test_glwd_reset_credentials_code_flow);
  tcase_add_test(tc_core, test_glwd_register_delete_mod);
  tcase_set_timeout(tc_core, 30);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(int argc, char *argv[])
{
  int number_failed = 0;
  Suite *s;
  SRunner *sr;
  struct _u_request auth_req;
  struct _u_response auth_resp;
  int res, do_test = 0, i;
  json_t * j_body;
  
  y_init_logs("Glewlwyd test", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Glewlwyd test");
  
  // Getting a valid session id for authenticated http requests
  ulfius_init_request(&auth_req);
  ulfius_init_request(&admin_req);
  ulfius_init_request(&user_req);
  ulfius_init_response(&auth_resp);
  auth_req.http_verb = strdup("POST");
  auth_req.http_url = msprintf("%s/auth/", SERVER_URI);
  j_body = json_pack("{ssss}", "username", USERNAME_ADMIN, "password", PASSWORD);
  ulfius_set_json_body_request(&auth_req, j_body);
  json_decref(j_body);
  res = ulfius_send_http_request(&auth_req, &auth_resp);
  if (res == U_OK && auth_resp.status == 200) {
    for (i=0; i<auth_resp.nb_cookies; i++) {
      char * cookie = msprintf("%s=%s", auth_resp.map_cookie[i].key, auth_resp.map_cookie[i].value);
      u_map_put(admin_req.map_header, "Cookie", cookie);
      o_free(cookie);
      y_log_message(Y_LOG_LEVEL_DEBUG, "User %s authenticated", USERNAME_ADMIN);
      do_test = 1;
    }
    ulfius_clean_response(&auth_resp);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error authentication %s (%d/%d/%d)", USERNAME_ADMIN, res, auth_resp.status, auth_resp.nb_cookies);
  }
  ulfius_clean_request(&auth_req);
  
  ulfius_init_request(&auth_req);
  auth_req.check_server_certificate = 0;
  ulfius_init_response(&auth_resp);
  auth_req.http_verb = strdup("POST");
  auth_req.http_url = msprintf("%s/auth/", SERVER_URI);
  j_body = json_pack("{ssss}", "username", USERNAME, "password", PASSWORD);
  ulfius_set_json_body_request(&auth_req, j_body);
  json_decref(j_body);
  res = ulfius_send_http_request(&auth_req, &auth_resp);
  if (res == U_OK && auth_resp.status == 200) {
    for (i=0; i<auth_resp.nb_cookies; i++) {
      char * cookie = msprintf("%s=%s", auth_resp.map_cookie[i].key, auth_resp.map_cookie[i].value);
      u_map_put(user_req.map_header, "Cookie", cookie);
      o_free(cookie);
      y_log_message(Y_LOG_LEVEL_DEBUG, "User %s authenticated", USERNAME);
      do_test = 1;
    }
  } else {
    do_test = 0;
    y_log_message(Y_LOG_LEVEL_ERROR, "Error authentication %s (%d/%d/%d)", USERNAME, res, auth_resp.status, auth_resp.nb_cookies);
  }
  ulfius_clean_response(&auth_resp);
  ulfius_clean_request(&auth_req);
  
  if (do_test) {
    s = glewlwyd_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
  }
  
  ulfius_clean_request(&admin_req);
  ulfius_clean_request(&user_req);
  
  return (do_test && number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
