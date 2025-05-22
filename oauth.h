#ifndef OAUTH_H
#define OAUTH_H

int RequestUserAuth();
int RequestAccessToken(const char *code, const char *code_verifier);

void generate_code_challenge(const char *code_verifier, char *code_challenge, int max_len);
void base64_url_encode(const unsigned char *input, int len, char *output, int out_len);
char *generateRandomString(int length);
void loadEnv(const char *filename);

#endif