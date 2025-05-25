#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "utils.h"

/**
 * @brief Callback function to handle the response from the server.
 * 
 * This function is called by libcurl when data is received. It appends
 * the received data to a dynamically allocated string.
 * 
 * @param acces_token Pointer to the acces_token stored in the environment.
 * @return Parsed JSON response with all the data of the current profile such as the : 
 *      country, 
 *      username, 
 *      email, 
 *      explicit_content filters, 
 *      followers, 
 *      id, 
 *      api url to the account.
 */
char *getProfile(const char *access_token) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl init failed\n");
        return "";
    }

    struct string response;
    init_string(&response);

    // Préparer le header Authorization
    char auth_header[512];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", access_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.spotify.com/v1/me");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        parse_JSON(response.ptr);
        return response.ptr;
    }

    free(response.ptr);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

char *getPlaylists(const char *access_token) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl init failed\n");
        return "";
    }

    struct string response;
    init_string(&response);

    // Préparer le header Authorization
    char auth_header[512];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", access_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.spotify.com/v1/me/playlists?limit=10&offset=0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        parse_JSON(response.ptr);
        return response.ptr;
    }

    free(response.ptr);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

/**
 * @brief Get items from a specific playlist.
 * 
 * This function retrieves the items from a specified playlist using the Spotify API.
 * 
 * @param access_token The access token for authorization.
 * @param playlist_id The ID of the playlist to retrieve items from.
 * @return A JSON string containing the items in the playlist.
 */
char *getPlaylistItems(const char *access_token, const char *playlist_id) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl init failed\n");
        return "";
    }
    struct string response;
    init_string(&response);
    // Prepare the Authorization header
    char auth_header[512];
    snprintf(auth_header, sizeof(auth_header), "Authorization Bearer %s", access_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);

    char url[256];
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s/tracks", playlist_id);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        parse_JSON(response.ptr);
        return response.ptr;
    }
    free(response.ptr);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

char *getLikedSong(const char *access_token) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl init failed\n");
        return "";
    }

    struct string response;
    init_string(&response);

    // Prepare the Authorization header
    char auth_header[512];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", access_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.spotify.com/v1/me/tracks");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        parse_JSON(response.ptr);
        return response.ptr;
    }

    free(response.ptr);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}