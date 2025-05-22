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

#endif