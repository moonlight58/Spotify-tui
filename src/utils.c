#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <cjson/cJSON.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ncurses.h>

#include "utils.h"
#include "oauth.h"

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
 * @brief Copy text to system clipboard
 *
 * @param text The text to copy to clipboard
 * @return 1 on success, 0 on failure
 */
int copy_to_clipboard(const char *text)
{
    FILE *pipe;

    // Try xclip first
    pipe = popen("xclip -selection clipboard 2>/dev/null", "w");
    if (pipe)
    {
        fprintf(pipe, "%s", text);
        int result = pclose(pipe);
        if (result == 0)
            return 1; // Success
    }

    // Fallback to xsel
    pipe = popen("xsel --clipboard --input 2>/dev/null", "w");
    if (pipe)
    {
        fprintf(pipe, "%s", text);
        int result = pclose(pipe);
        if (result == 0)
            return 1; // Success
    }

    return 0; // Failed
}

/**
 * @brief Display an error message in a window.
 *
 * This function displays an error red message in a ncurses window.
 * Avoiding to break the UI and allowing the user to continue
 * using the application.
 *
 * @param message The error message to display.
 */
void error_window(const char *message)
{
    WINDOW *error_win = newwin(4, 60, 10, 10);        
    box(error_win, 0, 0);                              
    wattron(error_win, COLOR_PAIR(2));                 
    mvwprintw(error_win, 1, 1, "Error: %s", message); 

    int clipboard_success = copy_to_clipboard(message);

    if (clipboard_success)
    {
        mvwprintw(error_win, 2, 1, "Error copied to clipboard.");
        mvwprintw(error_win, 3, 1, "Press any key to continue...");
    }
    else
    {
        mvwprintw(error_win, 2, 1, "Failed to copy to clipboard.");
        mvwprintw(error_win, 3, 1, "Press any key to continue...");
    }

    wattroff(error_win, COLOR_PAIR(2)); 
    wrefresh(error_win);
    getch();

    delwin(error_win); 
}

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

/**
 * @brief Helper function to extract string value from JSON.
 *
 * This function searches for a key in the JSON string and extracts its value.
 * It assumes the JSON format is valid and the key exists.
 *
 * @param json The JSON string to search.
 * @param key The key to search for in the JSON string.
 * @param output The buffer to store the extracted value.
 * @param output_size The size of the output buffer.
 * @return Pointer to the output buffer, or NULL if not found.
 */
char *extract_json_string(const char *json, const char *key, char *output, size_t output_size)
{
    char search_key[128];
    snprintf(search_key, sizeof(search_key), "\"%s\":\"", key);

    char *start = strstr(json, search_key);
    if (!start)
    {
        output[0] = '\0';
        return NULL;
    }

    start += strlen(search_key);
    char *end = strchr(start, '\"');
    if (!end)
    {
        output[0] = '\0';
        return NULL;
    }

    size_t length = end - start;
    if (length >= output_size)
    {
        length = output_size - 1;
    }

    strncpy(output, start, length);
    output[length] = '\0';
    return output;
}

/**
 * @brief Helper function to extract integer value from JSON.
 *
 * This function searches for a key in the JSON string and extracts its integer value.
 * It assumes the JSON format is valid and the key exists.
 *
 * @param json The JSON string to search.
 * @param key The key to search for in the JSON string.
 * @return The integer value associated with the key, or -1 if not found.
 */
int extract_json_int(const char *json, const char *key)
{
    char search_key[128];
    snprintf(search_key, sizeof(search_key), "\"%s\":", key);

    char *start = strstr(json, search_key);
    if (!start)
    {
        return -1;
    }

    start += strlen(search_key);
    // Skip whitespace
    while (*start == ' ' || *start == '\t')
        start++;

    return atoi(start);
}

/**
 * @brief Load environment variables from a .env file.
 *
 * This function reads a .env file and sets the environment variables
 * accordingly. It ignores comments and empty lines.
 *
 * @param filename The name of the .env file to load.
 */
void load_env(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Failed to open .env file");
        return;
    }
    char line[512];
    while (fgets(line, sizeof(line), file))
    {
        // Ignore comments and empty lines
        if (line[0] == '#' || line[0] == '\n')
            continue;
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        // Find '='
        char *equals = strchr(line, '=');
        if (!equals)
            continue;
        *equals = 0; // Split into key and value
        char *key = line;
        char *value = equals + 1;
        // Set environment variable
        setenv(key, value, 1);
    }
    fclose(file);
}

/**
 * @brief Encode data to Base64 URL format.
 *
 * This function encodes binary data to Base64 URL format, replacing
 * '+' with '-', '/' with '_', and removing '=' padding.
 *
 * @param input Pointer to the input data.
 * @param len Length of the input data.
 * @param output Pointer to the output buffer (must be large enough).
 * @param out_len Size of the output buffer.
 */
void base64_url_encode(const unsigned char *input, int len, char *output, int out_len)
{
    EVP_EncodeBlock((unsigned char *)output, input, len);
    // Replace '+' with '-', '/' with '_', and remove '='
    for (int i = 0; output[i]; i++)
    {
        if (output[i] == '+')
            output[i] = '-';
        else if (output[i] == '/')
            output[i] = '_';
    }
    // Remove '=' padding
    char *p = strchr(output, '=');
    if (p)
        *p = '\0';
}

/**
 * @brief Generate a code challenge from the code verifier.
 *
 * This function computes the SHA-256 hash of the code verifier
 * and encodes it to Base64 URL format. The result is stored in
 * the provided output buffer.
 *
 * @param code_verifier The code verifier string.
 * @param code_challenge The output buffer for the code challenge.
 * @param max_len The maximum length of the output buffer.
 */
void generate_code_challenge(const char *code_verifier, char *code_challenge, int max_len)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)code_verifier, strlen(code_verifier), hash);

    // Base64 buffer must be at least 4/3 the size of input + 4 for null-terminator
    char base64[128];
    base64_url_encode(hash, SHA256_DIGEST_LENGTH, base64, sizeof(base64));
    strncpy(code_challenge, base64, max_len - 1);
    code_challenge[max_len - 1] = '\0';
}

/**
 * @brief Generate a random string of specified length.
 *
 * This function generates a random alphanumeric string of the specified length.
 * The string is dynamically allocated and must be freed by the caller.
 *
 * @param length The length of the random string to generate.
 * @return Pointer to the generated string, or NULL on failure.
 */
char *generate_random_string(int length)
{
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    char *randomString = malloc(length + 1);
    if (randomString)
    {
        for (int i = 0; i < length; i++)
        {
            int key = rand() % (sizeof(charset) - 1);
            randomString[i] = charset[key];
        }
        randomString[length] = '\0';
    }
    return randomString;
}

/**
 * @brief Wait for the authorization code and request an access token.
 *
 * This function sets up a local server to listen for the redirect
 * from the Spotify authorization server. It captures the authorization
 * code and calls RequestAccessToken to exchange it for an access token.
 *
 * @param code_verifier The code verifier used in the PKCE flow.
 */
void wait_for_code_and_request_token(const char *code_verifier)
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[4096] = {0};

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0)
    {
        perror("socket failed");
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        return;
    }
    if (listen(server_fd, 1) < 0)
    {
        perror("listen");
        close(server_fd);
        return;
    }

    printf("Waiting for redirect with code on http://127.0.0.1:8080 ...\n");
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (new_socket < 0)
    {
        perror("accept");
        close(server_fd);
        return;
    }

    read(new_socket, buffer, sizeof(buffer) - 1);

    // Find "GET /?code="
    char *code_start = strstr(buffer, "GET /?code=");
    if (code_start)
    {
        code_start += strlen("GET /?code=");
        char *code_end = code_start;
        while (*code_end && *code_end != ' ' && *code_end != '&')
            code_end++;
        char code[512] = {0};
        strncpy(code, code_start, code_end - code_start);
        code[code_end - code_start] = '\0';

        // Respond to browser
        const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                               "<html><body><h1>You can close this window.</h1></body></html>";
        write(new_socket, response, strlen(response));

        // printf("Captured code: %s\n", code);
        request_access_token(code, code_verifier);
    }
    else
    {
        const char *response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        write(new_socket, response, strlen(response));
        printf("No code found in redirect.\n");
    }

    close(new_socket);
    close(server_fd);
}
