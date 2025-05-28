#ifndef OAUTH_H
#define OAUTH_H

int request_user_auth();
int refresh_access_token();
int request_access_token(const char *code, const char *code_verifier);
void connect_user_auth();
void check_and_refresh_token();

void generate_code_challenge(const char *code_verifier, char *code_challenge, int max_len);
void base64_url_encode(const unsigned char *input, int len, char *output, int out_len);
char *generate_random_string(int length);
#endif