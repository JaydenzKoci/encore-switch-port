#ifndef SONG_H
#define SONG_H

#include <raylib.h>
#include "cJSON.h"

#define MAX_FILES 1000 // Maximum number of valid song folders
#define FOLDER_PATH "sdmc:/switch/encore/Songs"

typedef struct {
    char* folder_name; // Name of the folder
    char* title;       // Song Title
    char* artist;      // Artist
    char* midi_path;   // MIDI
    char** stems;      // Stems
    int stem_count;
    Music* music;      // Array of raylib Music streams
    char* album;       // Album name
    char* release_year;// Release year
    char** genres;     // Array of genres
    int genre_count;   // Number of genres
    char** charters;   // Array of charters
    int charter_count; // Number of charters
    int length;        // Song length in seconds
    int diff_guitar;   // Guitar difficulty
    int diff_vocals;   // Vocals difficulty
    int diff_drums;    // Drums difficulty
    int diff_bass;     // Bass difficulty
    float preview_start_time; // Preview start time in seconds
    bool is_playing; // Track playback state
} Song;

// Initialize and load songs from FOLDER_PATH
int init_songs(Song* songs, int* song_count);

// Play or pause the selected song
void toggle_song_playback(Song* song, bool* playing);

// Play preview starting at preview_start_time
void play_song_preview(Song* song, bool* playing);

// Update music streams
void update_songs(Song* songs, int song_count,int playing_song_index);

// Free song resources
void free_songs(Song* songs, int song_count);

// Format song details
void get_song_details(Song* song, char* buffer, int buffer_size);

#endif