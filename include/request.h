#ifndef REQUEST_H
#define REQUEST_H

char *get_user_profile(const char *access_token);
char *get_user_playlists(const char *access_token);
char *get_user_liked_songs(const char *access_token);
char *get_user_playlist_items(const char *access_token, const char *playlist_id);

#endif