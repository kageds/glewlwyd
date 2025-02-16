/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <check.h>
#include <ulfius.h>
#include <orcania.h>
#include <yder.h>

#include "unit-tests.h"

#define SERVER_URI "http://localhost:4593/api"
#define USERNAME "user1"
#define PASSWORD "password"
#define ADMIN_USERNAME "admin"
#define ADMIN_PASSWORD "password"

#define PLUGIN_MODULE "oidc"
#define PLUGIN_NAME "register"
#define PLUGIN_ISS "https://glewlwyd.tld"
#define PLUGIN_PAIRWISE "pairwise"
#define PLUGIN_DISPLAY_NAME "Client registration test"
#define PLUGIN_JWT_TYPE "sha"
#define PLUGIN_JWT_KEY_SIZE "256"
#define PLUGIN_KEY "secret"
#define PLUGIN_CODE_DURATION 600
#define PLUGIN_REFRESH_TOKEN_DURATION 1209600
#define PLUGIN_ACCESS_TOKEN_DURATION 3600
#define PLUGIN_REGISTER_AUTH_SCOPE "g_profile"
#define PLUGIN_REGISTER_DEFAULT_SCOPE "scope3"

#define CLIENT_NAME                             "New Client"
#define CLIENT_REDIRECT_URI                     "https://client.tld/callback"
#define CLIENT_TOKEN_AUTH_NONE                  "none"
#define CLIENT_TOKEN_AUTH_SECRET_POST           "client_secret_post"
#define CLIENT_TOKEN_AUTH_SECRET_BASIC          "client_secret_basic"
#define CLIENT_TOKEN_AUTH_SECRET_JWT            "client_secret_jwt"
#define CLIENT_TOKEN_AUTH_PRIVATE_KEY_JWT       "private_key_jwt"
#define CLIENT_RESPONSE_TYPE_CODE               "code"
#define CLIENT_RESPONSE_TYPE_TOKEN              "token"
#define CLIENT_RESPONSE_TYPE_ID_TOKEN           "id_token"
#define CLIENT_GRANT_TYPE_AUTH_CODE             "authorization_code"
#define CLIENT_GRANT_TYPE_PASSWORD              "password"
#define CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS    "client_credentials"
#define CLIENT_GRANT_TYPE_REFRESH_TOKEN         "refresh_token"
#define CLIENT_GRANT_TYPE_DELETE_TOKEN          "delete_token"
#define CLIENT_GRANT_TYPE_DEVICE_AUTH           "device_authorization"
#define CLIENT_APP_TYPE_WEB                     "web"
#define CLIENT_APP_TYPE_NATIVE                  "native"
#define CLIENT_LOGO_URI                         "https://client.tld/logo.png"
#define CLIENT_CONTACT                          "contact@client.tld"
#define CLIENT_URI                              "https://client.tld/"
#define CLIENT_POLICY_URI                       "https://client.tld/policy"
#define CLIENT_TOS_URI                          "https://client.tld/tos"
#define CLIENT_JWKS_URI                         "https://client.tld/jwks"
#define CLIENT_RESOURCE_IDENTIFIER              "https://resource.tld/"
#define CLIENT_DEFAULT_KEY_1                    "key1"
#define CLIENT_DEFAULT_KEY_2                    "key2"
#define CLIENT_DEFAULT_KEY_OVERWRITTEN          "redirect_uri"
#define CLIENT_DEFAULT_VALUE_1                  "value1"
#define CLIENT_DEFAULT_VALUE_2                  "value2"
#define CLIENT_DEFAULT_VALUE_3                  "value3"
#define CLIENT_DEFAULT_VALUE_OVERWRITTEN        "overwrite-me"
#define CLIENT_SECTOR_IDENTIFIER_URI            "https://localhost:7344/siu"
#define CLIENT_SECTOR_IDENTIFIER_URI_2          "https://localhost:7344/siu/invalid_content_type"
#define CLIENT_SECTOR_IDENTIFIER_URI_3          "https://localhost:7344/siu/invalid_format"
#define CLIENT_SECTOR_IDENTIFIER_URI_4          "https://localhost:7344/siu/invalid_redirect_uri"
#define CB_KEY "cert/server.key"
#define CB_CRT "cert/server.crt"

const char jwk_pubkey_ecdsa_str[] = "{\"keys\":[{\"kty\":\"EC\",\"crv\":\"P-256\",\"x\":\"MKBCTNIcKUSDii11ySs3526iDZ8AiTo7Tu6KPAqv7D4\","\
                                    "\"y\":\"4Etl6SRW2YiLUrN5vfvVHuhp7x8PxltmWWlbbM4IFyM\",\"use\":\"enc\",\"kid\":\"1\"}]}";

struct _u_request admin_req;

static int callback_sector_identifier_uri_valid(const struct _u_request * request, struct _u_response * response, void * user_data) {
  json_t * j_response = json_pack("[ss]", CLIENT_REDIRECT_URI, "https://example.com");
  ulfius_set_json_body_response(response, 200, j_response);
  json_decref(j_response);
  return U_CALLBACK_CONTINUE;
}

static int callback_sector_identifier_uri_invalid_content_type(const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 200, "[\""CLIENT_REDIRECT_URI"\",\"https://example.com\"]");
  return U_CALLBACK_CONTINUE;
}

static int callback_sector_identifier_uri_invalid_format(const struct _u_request * request, struct _u_response * response, void * user_data) {
  json_t * j_response = json_pack("{s[ss]}", "redirect_uri", CLIENT_REDIRECT_URI, "https://example.com");
  ulfius_set_json_body_response(response, 200, j_response);
  json_decref(j_response);
  return U_CALLBACK_CONTINUE;
}

static int callback_sector_identifier_uri_invalid_redirect_uri(const struct _u_request * request, struct _u_response * response, void * user_data) {
  json_t * j_response = json_pack("[ss]", CLIENT_REDIRECT_URI "/error", "https://example.com");
  ulfius_set_json_body_response(response, 200, j_response);
  json_decref(j_response);
  return U_CALLBACK_CONTINUE;
}

START_TEST(test_oidc_registration_plugin_add_using_no_auth_scope)
{
  json_t * j_parameters = json_pack("{sssssssos{sssssssssisisisososososososos[]s[s]s{s{ss}s{s[ss]}s{s[s]}}}}",
                                "module", PLUGIN_MODULE,
                                "name", PLUGIN_NAME,
                                "display_name", PLUGIN_DISPLAY_NAME,
                                "enabled", json_true(),
                                "parameters",
                                  "iss", PLUGIN_ISS,
                                  "jwt-type", PLUGIN_JWT_TYPE,
                                  "jwt-key-size", PLUGIN_JWT_KEY_SIZE,
                                  "key", PLUGIN_KEY,
                                  "code-duration", PLUGIN_CODE_DURATION,
                                  "refresh-token-duration", PLUGIN_REFRESH_TOKEN_DURATION,
                                  "access-token-duration", PLUGIN_ACCESS_TOKEN_DURATION,
                                  "allow-non-oidc", json_true(),
                                  "auth-type-client-enabled", json_true(),
                                  "auth-type-code-enabled", json_true(),
                                  "auth-type-implicit-enabled", json_true(),
                                  "auth-type-password-enabled", json_true(),
                                  "auth-type-refresh-enabled", json_true(),
                                  "register-client-allowed", json_true(),
                                  "register-client-auth-scope",
                                  "register-client-credentials-scope", PLUGIN_REGISTER_DEFAULT_SCOPE,
                                  "register-default-properties",
                                    CLIENT_DEFAULT_KEY_1, 
                                      "value",
                                      CLIENT_DEFAULT_VALUE_1,
                                    CLIENT_DEFAULT_KEY_2,
                                      "value",
                                        CLIENT_DEFAULT_VALUE_2,
                                        CLIENT_DEFAULT_VALUE_3,
                                    CLIENT_DEFAULT_KEY_OVERWRITTEN,
                                      "value",
                                        CLIENT_DEFAULT_VALUE_OVERWRITTEN);

  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_parameters, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_parameters);
}
END_TEST

START_TEST(test_oidc_registration_plugin_add_using_auth_scope)
{
  json_t * j_parameters = json_pack("{sssssssos{sssssssssisisisososososososos[s]s[s]s{s{ss}s{s[ss]}}}}",
                                "module", PLUGIN_MODULE,
                                "name", PLUGIN_NAME,
                                "display_name", PLUGIN_DISPLAY_NAME,
                                "enabled", json_true(),
                                "parameters",
                                  "iss", PLUGIN_ISS,
                                  "jwt-type", PLUGIN_JWT_TYPE,
                                  "jwt-key-size", PLUGIN_JWT_KEY_SIZE,
                                  "key", PLUGIN_KEY,
                                  "code-duration", PLUGIN_CODE_DURATION,
                                  "refresh-token-duration", PLUGIN_REFRESH_TOKEN_DURATION,
                                  "access-token-duration", PLUGIN_ACCESS_TOKEN_DURATION,
                                  "allow-non-oidc", json_true(),
                                  "auth-type-client-enabled", json_true(),
                                  "auth-type-code-enabled", json_true(),
                                  "auth-type-implicit-enabled", json_true(),
                                  "auth-type-password-enabled", json_true(),
                                  "auth-type-refresh-enabled", json_true(),
                                  "register-client-allowed", json_true(),
                                  "register-client-auth-scope", PLUGIN_REGISTER_AUTH_SCOPE,
                                  "register-client-credentials-scope", PLUGIN_REGISTER_DEFAULT_SCOPE,
                                  "register-default-properties",
                                    CLIENT_DEFAULT_KEY_1, 
                                      "value",
                                      CLIENT_DEFAULT_VALUE_1,
                                    CLIENT_DEFAULT_KEY_2,
                                      "value",
                                        CLIENT_DEFAULT_VALUE_2,
                                        CLIENT_DEFAULT_VALUE_3);

  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_parameters, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_parameters);
}
END_TEST

START_TEST(test_oidc_registration_plugin_add_using_no_auth_scope_allow_add_resource)
{
  json_t * j_parameters = json_pack("{sssssssos{sssssssssisisisososososososos[]s[s]sos{s{ss}s{s[ss]}}}}",
                                "module", PLUGIN_MODULE,
                                "name", PLUGIN_NAME,
                                "display_name", PLUGIN_DISPLAY_NAME,
                                "enabled", json_true(),
                                "parameters",
                                  "iss", PLUGIN_ISS,
                                  "jwt-type", PLUGIN_JWT_TYPE,
                                  "jwt-key-size", PLUGIN_JWT_KEY_SIZE,
                                  "key", PLUGIN_KEY,
                                  "code-duration", PLUGIN_CODE_DURATION,
                                  "refresh-token-duration", PLUGIN_REFRESH_TOKEN_DURATION,
                                  "access-token-duration", PLUGIN_ACCESS_TOKEN_DURATION,
                                  "allow-non-oidc", json_true(),
                                  "auth-type-client-enabled", json_true(),
                                  "auth-type-code-enabled", json_true(),
                                  "auth-type-implicit-enabled", json_true(),
                                  "auth-type-password-enabled", json_true(),
                                  "auth-type-refresh-enabled", json_true(),
                                  "register-client-allowed", json_true(),
                                  "register-client-auth-scope",
                                  "register-client-credentials-scope", PLUGIN_REGISTER_DEFAULT_SCOPE,
                                  "register-resource-specify-allowed", json_true(),
                                  "register-default-properties",
                                    CLIENT_DEFAULT_KEY_1, 
                                      "value",
                                      CLIENT_DEFAULT_VALUE_1,
                                    CLIENT_DEFAULT_KEY_2,
                                      "value",
                                        CLIENT_DEFAULT_VALUE_2,
                                        CLIENT_DEFAULT_VALUE_3);

  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_parameters, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_parameters);
}
END_TEST

START_TEST(test_oidc_registration_plugin_add_using_no_auth_scope_resource_default)
{
  json_t * j_parameters = json_pack("{sssssssos{sssssssssisisisososososososos[]s[s]sos[s]s{s{ss}s{s[ss]}}}}",
                                "module", PLUGIN_MODULE,
                                "name", PLUGIN_NAME,
                                "display_name", PLUGIN_DISPLAY_NAME,
                                "enabled", json_true(),
                                "parameters",
                                  "iss", PLUGIN_ISS,
                                  "jwt-type", PLUGIN_JWT_TYPE,
                                  "jwt-key-size", PLUGIN_JWT_KEY_SIZE,
                                  "key", PLUGIN_KEY,
                                  "code-duration", PLUGIN_CODE_DURATION,
                                  "refresh-token-duration", PLUGIN_REFRESH_TOKEN_DURATION,
                                  "access-token-duration", PLUGIN_ACCESS_TOKEN_DURATION,
                                  "allow-non-oidc", json_true(),
                                  "auth-type-client-enabled", json_true(),
                                  "auth-type-code-enabled", json_true(),
                                  "auth-type-implicit-enabled", json_true(),
                                  "auth-type-password-enabled", json_true(),
                                  "auth-type-refresh-enabled", json_true(),
                                  "register-client-allowed", json_true(),
                                  "register-client-auth-scope",
                                  "register-client-credentials-scope", PLUGIN_REGISTER_DEFAULT_SCOPE,
                                  "register-resource-specify-allowed", json_false(),
                                  "register-resource-default", CLIENT_RESOURCE_IDENTIFIER,
                                  "register-default-properties",
                                    CLIENT_DEFAULT_KEY_1, 
                                      "value",
                                      CLIENT_DEFAULT_VALUE_1,
                                    CLIENT_DEFAULT_KEY_2,
                                      "value",
                                        CLIENT_DEFAULT_VALUE_2,
                                        CLIENT_DEFAULT_VALUE_3);

  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_parameters, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_parameters);
}
END_TEST

START_TEST(test_oidc_registration_plugin_add_using_no_auth_scope_subject_type_pairwise)
{
  json_t * j_parameters = json_pack("{sssssssos{sssssssssssisisisosososososososos[]s[s]s{s{ss}s{s[ss]}s{s[s]}}}}",
                                "module", PLUGIN_MODULE,
                                "name", PLUGIN_NAME,
                                "display_name", PLUGIN_DISPLAY_NAME,
                                "enabled", json_true(),
                                "parameters",
                                  "iss", PLUGIN_ISS,
                                  "subject-type", PLUGIN_PAIRWISE,
                                  "jwt-type", PLUGIN_JWT_TYPE,
                                  "jwt-key-size", PLUGIN_JWT_KEY_SIZE,
                                  "key", PLUGIN_KEY,
                                  "code-duration", PLUGIN_CODE_DURATION,
                                  "refresh-token-duration", PLUGIN_REFRESH_TOKEN_DURATION,
                                  "access-token-duration", PLUGIN_ACCESS_TOKEN_DURATION,
                                  "allow-non-oidc", json_true(),
                                  "auth-type-client-enabled", json_true(),
                                  "auth-type-code-enabled", json_true(),
                                  "auth-type-implicit-enabled", json_true(),
                                  "auth-type-password-enabled", json_true(),
                                  "auth-type-refresh-enabled", json_true(),
                                  "request-uri-allow-https-non-secure", json_true(),
                                  "register-client-allowed", json_true(),
                                  "register-client-auth-scope",
                                  "register-client-credentials-scope", PLUGIN_REGISTER_DEFAULT_SCOPE,
                                  "register-default-properties",
                                    CLIENT_DEFAULT_KEY_1, 
                                      "value",
                                      CLIENT_DEFAULT_VALUE_1,
                                    CLIENT_DEFAULT_KEY_2,
                                      "value",
                                        CLIENT_DEFAULT_VALUE_2,
                                        CLIENT_DEFAULT_VALUE_3,
                                    CLIENT_DEFAULT_KEY_OVERWRITTEN,
                                      "value",
                                        CLIENT_DEFAULT_VALUE_OVERWRITTEN);

  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/plugin/", NULL, NULL, j_parameters, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_parameters);
}
END_TEST

START_TEST(test_oidc_revocation_plugin_remove)
{
  ck_assert_int_eq(run_simple_test(&admin_req, "DELETE", SERVER_URI "/mod/plugin/" PLUGIN_NAME, NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
}
END_TEST

START_TEST(test_oidc_registration_no_auth_register_client_error_parameters)
{
  json_t * j_client, * j_jwks = json_loads(jwk_pubkey_ecdsa_str, JSON_DECODE_ANY, NULL);
  
  ck_assert_ptr_ne(j_jwks, NULL);
  
  // No redirect_uri
  j_client = json_pack("{sssss[ssssss]s[sss]sss[s]ssssssss}",
                       "client_name", CLIENT_NAME,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, "\"redirect_uris is mandatory and must be an array of strings\"", NULL), 1);
  json_decref(j_client);
  
  // invalid response_types
  j_client = json_pack("{sss[s]sss[ssssss]s[s]sss[s]ssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         "error",
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, "\"response_types must have one of the following values: 'code', 'token', 'id_token'\"", NULL), 1);
  json_decref(j_client);
  
  // invalid grant_types
  j_client = json_pack("{sss[s]sss[s]s[sss]sss[s]sssssssssO}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_PRIVATE_KEY_JWT,
                       "grant_types",
                         "error",
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI,
                       "jwks", j_jwks);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, "\"grant_types must have one of the following values: 'authorization_code', 'implicit', 'password', 'client_credentials', 'refresh_token', 'delete_token', 'device_authorization', 'urn:openid:params:grant-type:ciba'\"", NULL), 1);
  json_decref(j_client);
  
  // Invaid application_type
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]ssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", "error",
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, "\"application_type is optional and must have one of the following values: 'web', 'native'\"", NULL), 1);
  json_decref(j_client);
  
  // Invalid contacts
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[i]ssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         42,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, "\"contact value must be a non empty string\"", NULL), 1);
  json_decref(j_client);
  
  // Invalid logo_uri
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]ssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", "error",
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, "\"logo_uri is optional and must be a string\"", NULL), 1);
  json_decref(j_client);
  
  // Invalid client_uri
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]ssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", "error",
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, "\"client_uri is optional and must be a string\"", NULL), 1);
  json_decref(j_client);
  
  // Invalid policy_uri
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]ssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", "error",
                       "tos_uri", CLIENT_TOS_URI);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, "\"policy_uri is optional and must be a string\"", NULL), 1);
  json_decref(j_client);
  
  // Invalid tos_uri
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]ssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", "error");
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, "\"tos_uri is optional and must be a string\"", NULL), 1);
  json_decref(j_client);
  
  // Invalid jwks
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]ssssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_PRIVATE_KEY_JWT,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI,
                       "jwks", "error");
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, "\"Invalid JWKS\"", NULL), 1);
  json_decref(j_client);
  
  // Invalid jwks_uri
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]ssssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_PRIVATE_KEY_JWT,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI,
                       "jwks_uri", "error");
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, "\"jwks_uri is optional and must be an https:// uri\"", NULL), 1);
  json_decref(j_client);
  
  // Invalid jwks_uri and jwks
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]sssssssssOss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_PRIVATE_KEY_JWT,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI,
                       "jwks", j_jwks,
                       "jwks_uri", CLIENT_JWKS_URI);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, "\"jwks_uri and jwks can't coexist\"", NULL), 1);
  json_decref(j_client);
  
  json_decref(j_jwks);
}
END_TEST

START_TEST(test_oidc_registration_no_auth_register_client_ok)
{
  json_t * j_client;
  
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]ssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 200, j_client, NULL, NULL), 1);
  json_decref(j_client);
}
END_TEST

START_TEST(test_oidc_registration_no_auth_register_client_properties_validated)
{
  json_t * j_client, * j_jwks = json_loads(jwk_pubkey_ecdsa_str, JSON_DECODE_ANY, NULL), * j_result;
  struct _u_request req;
  struct _u_response resp;
  
  ck_assert_ptr_ne(j_jwks, NULL);
  ck_assert_int_eq(ulfius_init_request(&req), U_OK);
  ck_assert_int_eq(ulfius_init_response(&resp), U_OK);
  
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]sssssssssO}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_PRIVATE_KEY_JWT,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI,
                       "jwks", j_jwks);
  ck_assert_ptr_ne(j_client, NULL);
  req.http_verb = o_strdup("POST");
  req.http_url = o_strdup(SERVER_URI "/" PLUGIN_NAME "/register");
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_client), U_OK);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  j_result = ulfius_get_json_body_response(&resp, NULL);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_ptr_ne(json_object_get(j_result, "client_id"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "client_secret"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "registration_access_token"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "registration_client_uri"), NULL);
  admin_req.http_url = msprintf(SERVER_URI "/client/%s", json_string_value(json_object_get(j_result, "client_id")));
  admin_req.http_verb = o_strdup("GET");
  json_decref(j_result);
  ulfius_clean_response(&resp);
  ck_assert_int_eq(ulfius_init_response(&resp), U_OK);
  ck_assert_int_eq(ulfius_send_http_request(&admin_req, &resp), U_OK);
  o_free(admin_req.http_url);
  o_free(admin_req.http_verb);
  ck_assert_int_eq(resp.status, 200);
  j_result = ulfius_get_json_body_response(&resp, NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "enabled"), json_true());
  ck_assert_ptr_ne(json_object_get(j_result, "client_id"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "client_secret"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "registration_access_token"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "registration_client_uri"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "ignored"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "jwks_uri"), NULL);
  ck_assert_int_eq(json_string_length(json_object_get(j_result, "client_id")), 16);
  ck_assert_int_eq(json_array_size(json_object_get(j_result, "scope")), 1);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, "scope"), 0)), PLUGIN_REGISTER_DEFAULT_SCOPE);
  ck_assert_int_eq(json_array_size(json_object_get(j_result, "redirect_uri")), 1);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, "redirect_uri"), 0)), CLIENT_REDIRECT_URI);
  ck_assert_int_eq(json_array_size(json_object_get(j_result, "authorization_type")), 8);
  ck_assert_str_eq(json_string_value(json_object_get(j_result, "application_type")), CLIENT_APP_TYPE_WEB);
  ck_assert_int_eq(json_array_size(json_object_get(j_result, "contacts")), 1);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, "contacts"), 0)), CLIENT_CONTACT);
  ck_assert_str_eq(json_string_value(json_object_get(j_result, "logo_uri")), CLIENT_LOGO_URI);
  ck_assert_str_eq(json_string_value(json_object_get(j_result, "client_uri")), CLIENT_URI);
  ck_assert_str_eq(json_string_value(json_object_get(j_result, "policy_uri")), CLIENT_POLICY_URI);
  ck_assert_str_eq(json_string_value(json_object_get(j_result, "tos_uri")), CLIENT_TOS_URI);
  ck_assert_ptr_eq(json_object_get(j_result, "enabled"), json_true());
  ck_assert_int_eq(json_equal(json_object_get(j_result, "jwks"), j_jwks), 1);
  ck_assert_str_eq(json_string_value(json_object_get(j_result, CLIENT_DEFAULT_KEY_1)), CLIENT_DEFAULT_VALUE_1);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, CLIENT_DEFAULT_KEY_2), 0)), CLIENT_DEFAULT_VALUE_2);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, CLIENT_DEFAULT_KEY_2), 1)), CLIENT_DEFAULT_VALUE_3);
  ck_assert_str_ne(json_string_value(json_array_get(json_object_get(j_result, CLIENT_DEFAULT_KEY_OVERWRITTEN), 0)), CLIENT_DEFAULT_VALUE_OVERWRITTEN);
  json_decref(j_result);
  
  json_decref(j_client);
  json_decref(j_jwks);
  ulfius_clean_request(&req);
  ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_oidc_registration_no_auth_register_minimal_client_properties_validated)
{
  json_t * j_client, * j_jwks = json_loads(jwk_pubkey_ecdsa_str, JSON_DECODE_ANY, NULL), * j_result;
  struct _u_request req;
  struct _u_response resp;
  
  ck_assert_ptr_ne(j_jwks, NULL);
  ck_assert_int_eq(ulfius_init_request(&req), U_OK);
  ck_assert_int_eq(ulfius_init_response(&resp), U_OK);
  
  j_client = json_pack("{sss[s]}", "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_SECRET_BASIC, "redirect_uris", CLIENT_REDIRECT_URI);
  ck_assert_ptr_ne(j_client, NULL);
  req.http_verb = o_strdup("POST");
  req.http_url = o_strdup(SERVER_URI "/" PLUGIN_NAME "/register");
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_client), U_OK);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  j_result = ulfius_get_json_body_response(&resp, NULL);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_ptr_ne(json_object_get(j_result, "client_id"), NULL);
  ck_assert_ptr_ne(json_object_get(j_result, "client_secret"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "registration_access_token"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "registration_client_uri"), NULL);
  admin_req.http_url = msprintf(SERVER_URI "/client/%s", json_string_value(json_object_get(j_result, "client_id")));
  admin_req.http_verb = o_strdup("GET");
  json_decref(j_result);
  ulfius_clean_response(&resp);
  ck_assert_int_eq(ulfius_init_response(&resp), U_OK);
  ck_assert_int_eq(ulfius_send_http_request(&admin_req, &resp), U_OK);
  o_free(admin_req.http_url);
  admin_req.http_url = NULL;
  o_free(admin_req.http_verb);
  admin_req.http_verb = NULL;
  ck_assert_int_eq(resp.status, 200);
  j_result = ulfius_get_json_body_response(&resp, NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "jwks_uri"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "contacts"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "logo_uri"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "client_uri"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "policy_uri"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "tos_uri"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "jwks"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "enabled"), json_true());
  ck_assert_ptr_ne(json_object_get(j_result, "client_id"), NULL);
  ck_assert_ptr_ne(json_object_get(j_result, "client_secret"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "registration_access_token"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "registration_client_uri"), NULL);
  ck_assert_int_eq(json_string_length(json_object_get(j_result, "client_id")), 16);
  ck_assert_int_eq(json_string_length(json_object_get(j_result, "client_secret")), 32);
  ck_assert_int_eq(json_array_size(json_object_get(j_result, "scope")), 1);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, "scope"), 0)), PLUGIN_REGISTER_DEFAULT_SCOPE);
  ck_assert_int_eq(json_array_size(json_object_get(j_result, "redirect_uri")), 1);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, "redirect_uri"), 0)), CLIENT_REDIRECT_URI);
  ck_assert_int_eq(json_array_size(json_object_get(j_result, "authorization_type")), 1);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, "authorization_type"), 0)), CLIENT_RESPONSE_TYPE_CODE);
  ck_assert_str_eq(json_string_value(json_object_get(j_result, "application_type")), CLIENT_APP_TYPE_WEB);
  ck_assert_str_eq(json_string_value(json_object_get(j_result, CLIENT_DEFAULT_KEY_1)), CLIENT_DEFAULT_VALUE_1);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, CLIENT_DEFAULT_KEY_2), 0)), CLIENT_DEFAULT_VALUE_2);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, CLIENT_DEFAULT_KEY_2), 1)), CLIENT_DEFAULT_VALUE_3);
  json_decref(j_result);
  
  json_decref(j_client);
  json_decref(j_jwks);
  ulfius_clean_request(&req);
  ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_oidc_registration_no_auth_register_public_client_properties_validated)
{
  json_t * j_client, * j_jwks = json_loads(jwk_pubkey_ecdsa_str, JSON_DECODE_ANY, NULL), * j_result;
  struct _u_request req;
  struct _u_response resp;
  
  ck_assert_ptr_ne(j_jwks, NULL);
  ck_assert_int_eq(ulfius_init_request(&req), U_OK);
  ck_assert_int_eq(ulfius_init_response(&resp), U_OK);
  
  j_client = json_pack("{s[s]so}", "redirect_uris", CLIENT_REDIRECT_URI, "confidential", json_false());
  ck_assert_ptr_ne(j_client, NULL);
  req.http_verb = o_strdup("POST");
  req.http_url = o_strdup(SERVER_URI "/" PLUGIN_NAME "/register");
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_client), U_OK);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  j_result = ulfius_get_json_body_response(&resp, NULL);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_ptr_ne(json_object_get(j_result, "client_id"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "client_secret"), NULL);
  admin_req.http_url = msprintf(SERVER_URI "/client/%s", json_string_value(json_object_get(j_result, "client_id")));
  admin_req.http_verb = o_strdup("GET");
  json_decref(j_result);
  ulfius_clean_response(&resp);
  ck_assert_int_eq(ulfius_init_response(&resp), U_OK);
  ck_assert_int_eq(ulfius_send_http_request(&admin_req, &resp), U_OK);
  o_free(admin_req.http_url);
  admin_req.http_url = NULL;
  o_free(admin_req.http_verb);
  admin_req.http_verb = NULL;
  ck_assert_int_eq(resp.status, 200);
  j_result = ulfius_get_json_body_response(&resp, NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "jwks_uri"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "contacts"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "logo_uri"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "client_uri"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "policy_uri"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "tos_uri"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "jwks"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "enabled"), json_true());
  ck_assert_ptr_ne(json_object_get(j_result, "client_id"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "client_secret"), NULL);
  ck_assert_int_eq(json_string_length(json_object_get(j_result, "client_id")), 16);
  ck_assert_int_eq(json_array_size(json_object_get(j_result, "scope")), 1);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, "scope"), 0)), PLUGIN_REGISTER_DEFAULT_SCOPE);
  ck_assert_int_eq(json_array_size(json_object_get(j_result, "redirect_uri")), 1);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, "redirect_uri"), 0)), CLIENT_REDIRECT_URI);
  ck_assert_int_eq(json_array_size(json_object_get(j_result, "authorization_type")), 1);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, "authorization_type"), 0)), CLIENT_RESPONSE_TYPE_CODE);
  ck_assert_str_eq(json_string_value(json_object_get(j_result, "application_type")), CLIENT_APP_TYPE_WEB);
  ck_assert_str_eq(json_string_value(json_object_get(j_result, CLIENT_DEFAULT_KEY_1)), CLIENT_DEFAULT_VALUE_1);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, CLIENT_DEFAULT_KEY_2), 0)), CLIENT_DEFAULT_VALUE_2);
  ck_assert_str_eq(json_string_value(json_array_get(json_object_get(j_result, CLIENT_DEFAULT_KEY_2), 1)), CLIENT_DEFAULT_VALUE_3);
  json_decref(j_result);
  
  json_decref(j_client);
  json_decref(j_jwks);
  ulfius_clean_request(&req);
  ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_oidc_registration_no_auth_register_then_code_flow)
{
  json_t * j_client, * j_result, * j_body;
  struct _u_request req;
  struct _u_response resp;
  char * client_id, * client_secret, * code, * cookie, * refresh_token;
  
  ck_assert_int_eq(ulfius_init_request(&req), U_OK);
  ck_assert_int_eq(ulfius_init_response(&resp), U_OK);
  
  j_client = json_pack("{sss[s]s[ss]}", "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_SECRET_BASIC, "redirect_uris", CLIENT_REDIRECT_URI, "grant_types", "authorization_code", "refresh_token");
  ck_assert_ptr_ne(j_client, NULL);
  req.http_verb = o_strdup("POST");
  req.http_url = o_strdup(SERVER_URI "/" PLUGIN_NAME "/register");
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_client), U_OK);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  j_result = ulfius_get_json_body_response(&resp, NULL);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_ptr_ne(json_object_get(j_result, "client_id"), NULL);
  ck_assert_ptr_ne(json_object_get(j_result, "client_secret"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "registration_access_token"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "registration_client_uri"), NULL);
  client_id = o_strdup(json_string_value(json_object_get(j_result, "client_id")));
  client_secret = o_strdup(json_string_value(json_object_get(j_result, "client_secret")));
  json_decref(j_result);
  
  json_decref(j_client);
  ulfius_clean_request(&req);
  ulfius_clean_response(&resp);
  
  ck_assert_int_eq(ulfius_init_request(&req), U_OK);
  ck_assert_int_eq(ulfius_init_response(&resp), U_OK);
  req.http_verb = o_strdup("POST");
  req.http_url = o_strdup(SERVER_URI "/auth/");
  j_body = json_pack("{ssss}", "username", USERNAME, "password", PASSWORD);
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_body), U_OK);
  json_decref(j_body);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  cookie = msprintf("%s=%s", resp.map_cookie[0].key, resp.map_cookie[0].value);
  ck_assert_int_eq(u_map_put(req.map_header, "Cookie", cookie), U_OK);
  o_free(cookie);
  ulfius_clean_response(&resp);
  
  // Set grant
  ulfius_init_response(&resp);
  o_free(req.http_verb);
  o_free(req.http_url);
  req.http_verb = strdup("PUT");
  req.http_url = msprintf("%s/auth/grant/%s", SERVER_URI, client_id);
  j_body = json_pack("{ss}", "scope", "openid");
  ulfius_set_json_body_request(&req, j_body);
  json_decref(j_body);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  ulfius_clean_response(&resp);

  ck_assert_int_eq(ulfius_init_response(&resp), U_OK);
  o_free(req.http_verb);
  o_free(req.http_url);
  req.http_verb = o_strdup("GET");
  req.http_url = msprintf(SERVER_URI "/" PLUGIN_NAME "/auth?response_type=code&g_continue&client_id=%s&redirect_uri=" CLIENT_REDIRECT_URI "&state=xyzabcd&nonce=nonce1234&scope=openid", client_id);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 302);
  ck_assert_ptr_ne(o_strstr(u_map_get(resp.map_header, "Location"), "code="), NULL);
  code = o_strdup(o_strstr(u_map_get(resp.map_header, "Location"), "code=")+o_strlen("code="));
  if (o_strstr(code, "&") != NULL) {
    *o_strstr(code, "&") = '\0';
  }
  ulfius_clean_response(&resp);
  
  // Clean grant
  ulfius_init_response(&resp);
  o_free(req.http_verb);
  o_free(req.http_url);
  req.http_verb = strdup("PUT");
  req.http_url = msprintf("%s/auth/grant/%s", SERVER_URI, client_id);
  j_body = json_pack("{ss}", "scope", "");
  ulfius_set_json_body_request(&req, j_body);
  json_decref(j_body);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  ulfius_clean_request(&req);
  ulfius_clean_response(&resp);
  
  // Get tokens
  ck_assert_int_eq(ulfius_init_request(&req), U_OK);
  ck_assert_int_eq(ulfius_init_response(&resp), U_OK);
  req.http_verb = o_strdup("POST");
  req.http_url = o_strdup(SERVER_URI "/" PLUGIN_NAME "/token");
  u_map_put(req.map_post_body, "grant_type", "authorization_code");
  u_map_put(req.map_post_body, "code", code);
  u_map_put(req.map_post_body, "redirect_uri", CLIENT_REDIRECT_URI);
  req.auth_basic_user = o_strdup(client_id);
  req.auth_basic_password = o_strdup(client_secret);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  j_body = ulfius_get_json_body_response(&resp, NULL);
  ck_assert_ptr_ne(json_object_get(j_body, "refresh_token"), NULL);
  ck_assert_ptr_ne(json_object_get(j_body, "access_token"), NULL);
  refresh_token = o_strdup(json_string_value(json_object_get(j_body, "refresh_token")));
  json_decref(j_body);
  ulfius_clean_request(&req);
  ulfius_clean_response(&resp);

  // Refresh token
  ck_assert_int_eq(ulfius_init_request(&req), U_OK);
  ck_assert_int_eq(ulfius_init_response(&resp), U_OK);
  req.http_verb = o_strdup("POST");
  req.http_url = o_strdup(SERVER_URI "/" PLUGIN_NAME "/token");
  u_map_put(req.map_post_body, "grant_type", "refresh_token");
  u_map_put(req.map_post_body, "refresh_token", refresh_token);
  req.auth_basic_user = o_strdup(client_id);
  req.auth_basic_password = o_strdup(client_secret);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  j_body = ulfius_get_json_body_response(&resp, NULL);
  ck_assert_ptr_ne(json_object_get(j_body, "access_token"), NULL);
  json_decref(j_body);
  ulfius_clean_request(&req);
  ulfius_clean_response(&resp);

  o_free(client_id);
  o_free(client_secret);
  o_free(code);
  o_free(refresh_token);
}
END_TEST

START_TEST(test_oidc_registration_auth_register_client_without_credentials)
{
  json_t * j_client, * j_jwks = json_loads(jwk_pubkey_ecdsa_str, JSON_DECODE_ANY, NULL);
  
  ck_assert_ptr_ne(j_jwks, NULL);
  
  j_client = json_pack("{s[s]}", "redirect_uris", CLIENT_REDIRECT_URI);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 401, NULL, NULL, NULL), 1);
  json_decref(j_client);
  json_decref(j_jwks);
}
END_TEST

START_TEST(test_oidc_registration_auth_register_client_with_incorrect_credentials)
{
  struct _u_request req;
  struct _u_response resp;
  json_t * j_body, * j_client;
  const char * token;
  char * tmp;
  
  ulfius_init_request(&req);
  ulfius_init_response(&resp);
  req.http_verb = o_strdup("POST");
  req.http_url = o_strdup(SERVER_URI "/" PLUGIN_NAME "/token");
  u_map_put(req.map_post_body, "grant_type", "password");
  u_map_put(req.map_post_body, "scope", "openid");
  u_map_put(req.map_post_body, "username", USERNAME);
  u_map_put(req.map_post_body, "password", PASSWORD);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  j_body = ulfius_get_json_body_response(&resp, NULL);
  token = json_string_value(json_object_get(j_body, "access_token"));
  ck_assert_ptr_ne(token, NULL);
  ulfius_clean_response(&resp);
  
  ulfius_init_response(&resp);
  tmp = msprintf("Bearer %s", token);
  u_map_put(req.map_header, "Authorization", tmp);
  o_free(tmp);
  o_free(req.http_url);
  j_client = json_pack("{s[s]}", "redirect_uris", CLIENT_REDIRECT_URI);
  ck_assert_ptr_ne(j_client, NULL);
  req.http_url = o_strdup(SERVER_URI "/" PLUGIN_NAME "/register");
  ck_assert_int_eq(ulfius_set_json_body_request(&req, j_client), U_OK);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 401);
  json_decref(j_client);
  json_decref(j_body);
  ulfius_clean_request(&req);
  ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_oidc_registration_auth_register_client_with_valid_credentials)
{
  struct _u_request req, req_reg;
  struct _u_response resp;
  json_t * j_body, * j_client, * j_result;
  const char * token;
  char * tmp;
  
  ulfius_init_request(&req);
  ulfius_init_response(&resp);
  req.http_verb = o_strdup("POST");
  req.http_url = o_strdup(SERVER_URI "/" PLUGIN_NAME "/token");
  u_map_put(req.map_post_body, "grant_type", "password");
  u_map_put(req.map_post_body, "scope", PLUGIN_REGISTER_AUTH_SCOPE);
  u_map_put(req.map_post_body, "username", USERNAME);
  u_map_put(req.map_post_body, "password", PASSWORD);
  ck_assert_int_eq(ulfius_send_http_request(&req, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  j_body = ulfius_get_json_body_response(&resp, NULL);
  token = json_string_value(json_object_get(j_body, "access_token"));
  ck_assert_ptr_ne(token, NULL);
  ulfius_clean_response(&resp);
  ulfius_clean_request(&req);
  
  ulfius_init_request(&req_reg);
  ulfius_init_response(&resp);
  tmp = msprintf("Bearer %s", token);
  u_map_put(req_reg.map_header, "Authorization", tmp);
  o_free(tmp);
  j_client = json_pack("{sss[s]s[s]}", "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_SECRET_BASIC, "redirect_uris", CLIENT_REDIRECT_URI, "grant_types", "client_credentials");
  ck_assert_ptr_ne(j_client, NULL);
  req_reg.http_verb = o_strdup("POST");
  req_reg.http_url = o_strdup(SERVER_URI "/" PLUGIN_NAME "/register");
  ck_assert_int_eq(ulfius_set_json_body_request(&req_reg, j_client), U_OK);
  ck_assert_int_eq(ulfius_send_http_request(&req_reg, &resp), U_OK);
  ck_assert_int_eq(resp.status, 200);
  j_result = ulfius_get_json_body_response(&resp, NULL);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_ptr_ne(json_object_get(j_result, "client_id"), NULL);
  ck_assert_ptr_ne(json_object_get(j_result, "client_secret"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "registration_access_token"), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "registration_client_uri"), NULL);
  admin_req.http_url = msprintf(SERVER_URI "/client/%s", json_string_value(json_object_get(j_result, "client_id")));
  admin_req.http_verb = o_strdup("GET");
  json_decref(j_result);
  json_decref(j_client);
  json_decref(j_body);
  ulfius_clean_request(&req_reg);
  ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_oidc_registration_no_auth_register_client_specify_resource_invalid)
{
  json_t * j_client;
  
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]sssssssss[s]}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI,
                       "resource", "error");
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_client);
}
END_TEST

START_TEST(test_oidc_registration_no_auth_register_client_specify_resource_ok)
{
  json_t * j_client;
  
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]sssssssss[s]}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI,
                       "resource", CLIENT_RESOURCE_IDENTIFIER);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 200, j_client, NULL, NULL), 1);
  json_decref(j_client);
}
END_TEST

START_TEST(test_oidc_registration_no_auth_register_client_specify_resource_ignored)
{
  json_t * j_client, * j_result;
  
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]sssssssss[s]}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI,
                       "resource", CLIENT_RESOURCE_IDENTIFIER "ignore_me");
  j_result = json_pack("{s[s]}", "resource", CLIENT_RESOURCE_IDENTIFIER);
  
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 200, j_result, NULL, NULL), 1);
  json_decref(j_client);
  json_decref(j_result);
}
END_TEST

START_TEST(test_oidc_registration_no_auth_register_client_no_specify_resource)
{
  json_t * j_client, * j_result;
  
  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]ssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI);
  j_result = json_pack("{s[s]}", "resource", CLIENT_RESOURCE_IDENTIFIER);
  
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 200, j_result, NULL, NULL), 1);
  json_decref(j_client);
  json_decref(j_result);
}
END_TEST

START_TEST(test_oidc_registration_no_auth_register_client_with_sector_identifier_uri)
{
  json_t * j_client;
  struct _u_instance instance;
  
  ck_assert_int_eq(ulfius_init_instance(&instance, 7344, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", NULL, "siu", 0, &callback_sector_identifier_uri_valid, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", NULL, "siu/invalid_content_type", 0, &callback_sector_identifier_uri_invalid_content_type, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", NULL, "siu/invalid_format", 0, &callback_sector_identifier_uri_invalid_format, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", NULL, "siu/invalid_redirect_uri", 0, &callback_sector_identifier_uri_invalid_redirect_uri, NULL), U_OK);

  char * key_pem = read_file(CB_KEY);
  char * cert_pem = read_file(CB_CRT);
  ck_assert_ptr_ne(NULL, key_pem);
  ck_assert_ptr_ne(NULL, cert_pem);
  ck_assert_int_eq(ulfius_start_secure_framework(&instance, key_pem, cert_pem), U_OK);
  o_free(key_pem);
  o_free(cert_pem);

  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]ssssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI,
                       "sector_identifier_uri", CLIENT_SECTOR_IDENTIFIER_URI_2);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_client);

  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]ssssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI,
                       "sector_identifier_uri", CLIENT_SECTOR_IDENTIFIER_URI_3);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 400, NULL, NULL, NULL), 1);
  json_decref(j_client);

  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]ssssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI,
                       "sector_identifier_uri", CLIENT_SECTOR_IDENTIFIER_URI);
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 200, j_client, NULL, NULL), 1);
  json_decref(j_client);

  j_client = json_pack("{sss[s]sss[ssssss]s[sss]sss[s]ssssssssssssssss}",
                       "client_name", CLIENT_NAME,
                       "redirect_uris", CLIENT_REDIRECT_URI,
                       "token_endpoint_auth_method", CLIENT_TOKEN_AUTH_NONE,
                       "grant_types",
                         CLIENT_GRANT_TYPE_AUTH_CODE,
                         CLIENT_GRANT_TYPE_PASSWORD,
                         CLIENT_GRANT_TYPE_CLIENT_CREDENTIALS,
                         CLIENT_GRANT_TYPE_REFRESH_TOKEN,
                         CLIENT_GRANT_TYPE_DELETE_TOKEN,
                         CLIENT_GRANT_TYPE_DEVICE_AUTH,
                       "response_types",
                         CLIENT_RESPONSE_TYPE_CODE,
                         CLIENT_RESPONSE_TYPE_TOKEN,
                         CLIENT_RESPONSE_TYPE_ID_TOKEN,
                       "application_type", CLIENT_APP_TYPE_WEB,
                       "contacts",
                         CLIENT_CONTACT,
                       "logo_uri", CLIENT_LOGO_URI,
                       "client_uri", CLIENT_URI,
                       "policy_uri", CLIENT_POLICY_URI,
                       "tos_uri", CLIENT_TOS_URI,
                       "sector_identifier_uri", CLIENT_SECTOR_IDENTIFIER_URI,
                       "request_object_signing_alg", "RS256",
                       "request_object_encryption_alg", "RSA-OAEP-256",
                       "request_object_encryption_enc", "A128CBC-HS256");
  ck_assert_ptr_ne(j_client, NULL);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/" PLUGIN_NAME "/register", NULL, NULL, j_client, NULL, 200, j_client, NULL, NULL), 1);
  json_decref(j_client);

  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);
}
END_TEST

static Suite *glewlwyd_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Glewlwyd oidc token revocation");
  tc_core = tcase_create("test_oidc_token_revocation");
  tcase_add_test(tc_core, test_oidc_registration_plugin_add_using_no_auth_scope);
  tcase_add_test(tc_core, test_oidc_registration_no_auth_register_client_error_parameters);
  tcase_add_test(tc_core, test_oidc_registration_no_auth_register_client_ok);
  tcase_add_test(tc_core, test_oidc_registration_no_auth_register_client_properties_validated);
  tcase_add_test(tc_core, test_oidc_registration_no_auth_register_public_client_properties_validated);
  tcase_add_test(tc_core, test_oidc_registration_no_auth_register_minimal_client_properties_validated);
  tcase_add_test(tc_core, test_oidc_registration_no_auth_register_then_code_flow);
  tcase_add_test(tc_core, test_oidc_revocation_plugin_remove);
  tcase_add_test(tc_core, test_oidc_registration_plugin_add_using_auth_scope);
  tcase_add_test(tc_core, test_oidc_registration_auth_register_client_without_credentials);
  tcase_add_test(tc_core, test_oidc_registration_auth_register_client_with_incorrect_credentials);
  tcase_add_test(tc_core, test_oidc_registration_auth_register_client_with_valid_credentials);
  tcase_add_test(tc_core, test_oidc_revocation_plugin_remove);
  tcase_add_test(tc_core, test_oidc_registration_plugin_add_using_no_auth_scope_allow_add_resource);
  tcase_add_test(tc_core, test_oidc_registration_no_auth_register_client_specify_resource_invalid);
  tcase_add_test(tc_core, test_oidc_registration_no_auth_register_client_specify_resource_ok);
  tcase_add_test(tc_core, test_oidc_revocation_plugin_remove);
  tcase_add_test(tc_core, test_oidc_registration_plugin_add_using_no_auth_scope_resource_default);
  tcase_add_test(tc_core, test_oidc_registration_no_auth_register_client_specify_resource_ignored);
  tcase_add_test(tc_core, test_oidc_registration_no_auth_register_client_no_specify_resource);
  tcase_add_test(tc_core, test_oidc_revocation_plugin_remove);
  tcase_add_test(tc_core, test_oidc_registration_plugin_add_using_no_auth_scope_subject_type_pairwise);
  tcase_add_test(tc_core, test_oidc_registration_no_auth_register_client_with_sector_identifier_uri);
  tcase_add_test(tc_core, test_oidc_revocation_plugin_remove);
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
  json_t * j_body;
  int res, do_test = 0, i;
  
  y_init_logs("Glewlwyd test", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Glewlwyd test");
  
  // Getting a valid session id for authenticated http requests
  ulfius_init_request(&admin_req);
  
  ulfius_init_request(&auth_req);
  ulfius_init_response(&auth_resp);
  auth_req.http_verb = strdup("POST");
  auth_req.http_url = msprintf("%s/auth/", SERVER_URI);
  j_body = json_pack("{ssss}", "username", ADMIN_USERNAME, "password", ADMIN_PASSWORD);
  ulfius_set_json_body_request(&auth_req, j_body);
  json_decref(j_body);
  j_body = NULL;
  res = ulfius_send_http_request(&auth_req, &auth_resp);
  if (res == U_OK && auth_resp.status == 200) {
    for (i=0; i<auth_resp.nb_cookies; i++) {
      char * cookie = msprintf("%s=%s", auth_resp.map_cookie[i].key, auth_resp.map_cookie[i].value);
      u_map_put(admin_req.map_header, "Cookie", cookie);
      o_free(cookie);
    }
    y_log_message(Y_LOG_LEVEL_INFO, "User %s authenticated", ADMIN_USERNAME);
    do_test = 1;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error authentication");
    do_test = 0;
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
  json_decref(j_body);
  
  ulfius_clean_request(&admin_req);
  
  y_close_logs();

  return (do_test && number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
