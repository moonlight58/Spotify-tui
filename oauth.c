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

#include "utils.h"

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

    // printf("RAT Response: %s\n", response.ptr);

    // Parse the complete JSON response
    struct TokenResponse token_data;
    if (parse_token_response(response.ptr, &token_data) == 0)
    {
        // printf("Access Token: %s\n", token_data.access_token);
        // printf("Token Type: %s\n", token_data.token_type);
        // printf("Expires In: %d seconds\n", token_data.expires_in);
        // printf("Refresh Token: %s\n", token_data.refresh_token);
        // printf("Scope: %s\n", token_data.scope);

        // Store tokens in environment variables
        setenv("ACCESS_TOKEN", token_data.access_token, 1);
        setenv("SCOPE", token_data.scope, 1);
        setenv("REFRESH_TOKEN", token_data.refresh_token, 1);
        setenv("TOKEN_TYPE", token_data.token_type, 1);

        // Calculate and store expiration time
        char expires_str[32];
        snprintf(expires_str, sizeof(expires_str), "%d", token_data.expires_in);
        setenv("TOKEN_EXPIRES_IN", expires_str, 1);

        // printf("Env var:\nACCESS_TOKEN: %s\nSCOPE: %s\nREFRESH_TOKEN: %s\nTOKEN_TYPE: %s\n", getenv("ACCESS_TOKEN"), getenv("SCOPE"), getenv("REFRESH_TOKEN"), getenv("TOKEN_TYPE"));
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
 * @brief Refresh the Spotify access token using the refresh token.
 *
 * This function sends a POST request to the Spotify API to obtain a new access token
 * using the refresh token. It updates the ACCESS_TOKEN and optionally the REFRESH_TOKEN
 * environment variables.
 *
 * @return 0 on success, non-zero on failure.
 */
int RefreshAccessToken()
{
    const char *client_id = getenv("CLIENT_ID");
    const char *refresh_token = getenv("REFRESH_TOKEN");

    if (!client_id || !refresh_token) {
        fprintf(stderr, "Missing CLIENT_ID or REFRESH_TOKEN\n");
        return 1;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to init curl\n");
        return 1;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

    char postfields[1024];
    snprintf(postfields, sizeof(postfields),
        "grant_type=refresh_token"
        "&refresh_token=%s"
        "&client_id=%s",
        refresh_token, client_id);

    struct string response;
    init_string(&response);

    curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return 1;
    }

    // Parse and update tokens
    struct TokenResponse token_data;
    if (parse_token_response(response.ptr, &token_data) == 0) {
        setenv("ACCESS_TOKEN", token_data.access_token, 1);
        if (strlen(token_data.refresh_token) > 0)
            setenv("REFRESH_TOKEN", token_data.refresh_token, 1);
        setenv("SCOPE", token_data.scope, 1);
        setenv("TOKEN_TYPE", token_data.token_type, 1);
        char expires_str[32];
        snprintf(expires_str, sizeof(expires_str), "%d", token_data.expires_in);
        setenv("TOKEN_EXPIRES_IN", expires_str, 1);
    } else {
        fprintf(stderr, "Failed to parse refresh token response\n");
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
 * @brief Request user authorization for Spotify API.
 *
 * This function generates a random code verifier, constructs the
 * authorization URL, and opens it in the default web browser.
 * It waits for the redirect with the authorization code and
 * requests an access token using that code.
 */
char RequestUserAuth()
{
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