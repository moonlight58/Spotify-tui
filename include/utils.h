#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <string.h>

struct string {
    char *ptr;
    size_t len;
};

void init_string(struct string *s);
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s);
void parse_JSON(const char *json);
char *extract_json_string(const char *json, const char *key, char *output, size_t output_size);
int extract_json_int(const char *json, const char *key);
void base64_url_encode(const unsigned char *input, int len, char *output, int out_len);
void loadEnv(const char *filename);
char *generateRandomString(int length);
void generate_code_challenge(const char *code_verifier, char *code_challenge, int max_len);
void wait_for_code_and_request_token(const char *code_verifier);

#endif