cmake_minimum_required(VERSION 3.29)
project(Spotify_tui C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(Spotify_tui
    main.c
    oauth.c
    oauth.h)
