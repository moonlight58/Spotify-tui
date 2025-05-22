#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <cjson/cJSON.h>

#include "utils.h"

/**
 * @brief Initialize a string structure for dynamic memory allocation.
 *
 * This function initializes a string structure by allocating memory for it.
 * The initial length is set to 0, and the pointer is allocated with 1 byte
 * to accommodate the null terminator.
 *
 * @param s Pointer to the string structure to be initialized.
 */
void init_string(struct string *s)
{
    s->len = 0;
    s->ptr = malloc(1);
    if (s->ptr)
        s->ptr[0] = '\0';
};

/**
 * @brief Callback function for libcurl to write received data into a dynamic buffer.
 *
 * This function is used by libcurl's CURLOPT_WRITEFUNCTION option.
 * It appends the received data chunk to a dynamically growing buffer
 * (struct string), ensuring the buffer is always null-terminated.
 *
 * @param ptr Pointer to the received data.
 * @param size Size of each data element.
 * @param nmemb Number of data elements.
 * @param s Pointer to a struct string where the data will be appended.
 * @return The number of bytes actually handled (size * nmemb).
 */
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
    size_t new_len = s->len + size * nmemb; // Calculate new total length
    s->ptr = realloc(s->ptr, new_len + 1);  // Reallocate buffer (+1 for null terminator)
    if (s->ptr)
    {
        // Copy new data to the end of the buffer
        memcpy(s->ptr + s->len, ptr, size * nmemb);
        s->ptr[new_len] = '\0'; // Null-terminate the buffer
        s->len = new_len;       // Update the length
    }
    return size * nmemb; // Return the number of bytes handled
};

/**
 * @brief Parse a JSON string and print it in a pretty format.
 *
 * This function uses cJSON to parse a JSON string and print it in a
 * human-readable format. It also handles memory cleanup for the cJSON
 * object.
 *
 * @param json The JSON string to be parsed.
 */
void parse_JSON(const char *json)
{
    cJSON *root = cJSON_Parse(json);
    if (root == NULL)
    {
        printf("Invalid JSON\n");
        return;
    }
    char *pretty = cJSON_Print(root);
    if (pretty)
    {
        printf("Parsed JSON:\n%s\n", pretty);
        free(pretty);
    }
    cJSON_Delete(root);
}