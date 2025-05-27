#ifndef REQUEST_H
#define REQUEST_H

char *getProfile(const char *access_token);
char *getPlaylists(const char *access_token);
char *getLikedSong(const char *access_token);
char *getPlaylistItems(const char *access_token, const char *playlist_id);

#endif