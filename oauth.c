#include <openssl/sha.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <curl/curl.h>

/**
 * @brief Structure to hold a dynamically growing string.
 * 
 * This structure is used to store data received from the Spotify API.
 * It contains a pointer to the string data and its current length.
 * The string is dynamically allocated and can grow as needed.
 * The string is null-terminated to facilitate string operations.
 */
struct string
{
    char *ptr;
    size_t len;
};

/**
 * @brief Structure to hold the token response data.
 *
 * This structure is used to store the access token, token type,
 * expiration time, refresh token, and scope received from the
 * Spotify API after a successful authorization code exchange.
 */
struct TokenResponse
{
    char access_token[512];
    char token_type[64];
    int expires_in;
    char refresh_token[512];
    char scope[256];
};

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
 * @brief Parse the JSON response from the token endpoint.
 *
 * This function extracts the access token, token type, expiration time,
 * refresh token, and scope from the JSON response.
 *
 * @param json_response The JSON response string.
 * @param token_data Pointer to a TokenResponse structure to store the parsed data.
 * @return 0 on success, non-zero on failure.
 */
int parse_token_response(const char *json_response, struct TokenResponse *token_data)
{
    if (!json_response || !token_data)
    {
        return -1;
    }

    // Initialize the structure
    memset(token_data, 0, sizeof(struct TokenResponse));

    // Extract access_token
    if (!extract_json_string(json_response, "access_token", token_data->access_token, sizeof(token_data->access_token)))
    {
        fprintf(stderr, "Failed to parse access_token\n");
        return -1;
    }

    // Extract token_type
    extract_json_string(json_response, "token_type", token_data->token_type, sizeof(token_data->token_type));

    // Extract expires_in
    token_data->expires_in = extract_json_int(json_response, "expires_in");

    // Extract refresh_token
    extract_json_string(json_response, "refresh_token", token_data->refresh_token, sizeof(token_data->refresh_token));

    // Extract scope
    extract_json_string(json_response, "scope", token_data->scope, sizeof(token_data->scope));

    return 0;
}

/**
 * @brief Request an access token from Spotify using the authorization code.
 *
 * This function sends a POST request to the Spotify API to exchange the
 * authorization code for an access token. It uses libcurl for HTTP requests.
 *
 * @param code The authorization code received from the redirect.
 * @param code_verifier The code verifier used in the PKCE flow.
 * @return 0 on success, non-zero on failure.
 */
int RequestAccessToken(const char *code, const char *code_verifier)
{
    const char *client_id = getenv("CLIENT_ID");
    const char *redirect_uri = getenv("REDIRECT_URI");

    CURL *curl = curl_easy_init();
    if (!curl)
    {
        fprintf(stderr, "Failed to init curl\n");
        return 1;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

    char postfields[1024];
    snprintf(postfields, sizeof(postfields),
             "client_id=%s"
             "&grant_type=authorization_code"
             "&code=%s"
             "&redirect_uri=%s"
             "&code_verifier=%s",
             client_id, code, redirect_uri, code_verifier);

    struct string response;
    init_string(&response);

    curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return 1;
    }

    printf("RAT Response: %s\n", response.ptr);

    // Parse the complete JSON response
    struct TokenResponse token_data;
    if (parse_token_response(response.ptr, &token_data) == 0)
    {
        printf("Access Token: %s\n", token_data.access_token);
        printf("Token Type: %s\n", token_data.token_type);
        printf("Expires In: %d seconds\n", token_data.expires_in);
        printf("Refresh Token: %s\n", token_data.refresh_token);
        printf("Scope: %s\n", token_data.scope);

        // Store tokens in environment variables
        setenv("ACCESS_TOKEN", token_data.access_token, 1);
        setenv("REFRESH_TOKEN", token_data.refresh_token, 1);
        setenv("TOKEN_TYPE", token_data.token_type, 1);

        // Calculate and store expiration time
        char expires_str[32];
        snprintf(expires_str, sizeof(expires_str), "%d", token_data.expires_in);
        setenv("TOKEN_EXPIRES_IN", expires_str, 1);
    }
    else
    {
        fprintf(stderr, "Failed to parse token response\n");
        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return 1;
    }

    free(response.ptr);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return 0;
}

/**
 * @brief Load environment variables from a .env file.
 * 
 * This function reads a .env file and sets the environment variables
 * accordingly. It ignores comments and empty lines.
 * 
 * @param filename The name of the .env file to load.
 */
void loadEnv(const char *filename)
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
char *generateRandomString(int length)
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

        printf("Captured code: %s\n", code);
        RequestAccessToken(code, code_verifier);
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

/**
 * @brief Request user authorization for Spotify API.
 * 
 * This function generates a random code verifier, constructs the
 * authorization URL, and opens it in the default web browser.
 * It waits for the redirect with the authorization code and
 * requests an access token using that code.
 */
char RequestUserAuth()
{
    // Load environment variables from .env file
    loadEnv(".env");

    const char *client_id = getenv("CLIENT_ID");
    // printf("Client ID: %s\n", client_id);
    const char *redirect_uri = getenv("REDIRECT_URI");
    // printf("Redirect URI: %s\n", redirect_uri);
    const char *scope = "user-read-private user-read-email";

    char *code_verifier = generateRandomString(64);

    char code_challenge[128];
    generate_code_challenge(code_verifier, code_challenge, sizeof(code_challenge));

    // Build the authorization URL
    char auth_url[1024];
    snprintf(auth_url, sizeof(auth_url),
             "https://accounts.spotify.com/authorize"
             "?response_type=code"
             "&client_id=%s"
             "&scope=%s"
             "&code_challenge_method=S256"
             "&code_challenge=%s"
             "&redirect_uri=%s",
             client_id, scope, code_challenge, redirect_uri);

    printf("Open this URL in your browser:\n%s\n", auth_url);

    // Optionally, open the URL in the default browser (Linux)
    char command[1100];
    snprintf(command, sizeof(command), "xdg-open \"%s\"", auth_url);
    system(command);

    wait_for_code_and_request_token(code_verifier);

    free(code_verifier);
}