#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <switch.h>
#include "song.h"
#include "stdbool.h"

int init_songs(Song* songs, int* song_count) {
    *song_count = 0;
    bool dir_error = false;

    struct stat st;
    if (stat(FOLDER_PATH, &st) != 0) {
        mkdir("sdmc:/switch", 0777);
        mkdir("sdmc:/switch/encore", 0777);
        mkdir(FOLDER_PATH, 0777);
    }

    DIR* dir = opendir(FOLDER_PATH);
    if (!dir) {
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL && *song_count < MAX_FILES) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char info_path[512];
        snprintf(info_path, sizeof(info_path), "%s/%s/info.json", FOLDER_PATH, entry->d_name);

        if (stat(info_path, &st) != 0) {
            continue;
        }

        FILE* fp = fopen(info_path, "r");
        if (!fp) continue;

        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char* json_data = (char*)malloc(file_size + 1);
        fread(json_data, 1, file_size, fp);
        json_data[file_size] = '\0';
        fclose(fp);

        cJSON* json = cJSON_Parse(json_data);
        free(json_data);
        if (!json) continue;

        cJSON* midi = cJSON_GetObjectItem(json, "midi");
        if (!cJSON_IsString(midi) || !midi->valuestring) {
            cJSON_Delete(json);
            continue;
        }

        char midi_path[512];
        snprintf(midi_path, sizeof(midi_path), "%s/%s/%s", FOLDER_PATH, entry->d_name, midi->valuestring);
        if (stat(midi_path, &st) != 0) {
            cJSON_Delete(json);
            continue;
        }

        cJSON* title = cJSON_GetObjectItem(json, "title");
        cJSON* artist = cJSON_GetObjectItem(json, "artist");
        if (!cJSON_IsString(title) || !cJSON_IsString(artist)) {
            cJSON_Delete(json);
            continue;
        }

        cJSON* album = cJSON_GetObjectItem(json, "album");
        cJSON* release_year = cJSON_GetObjectItem(json, "release_year");
        cJSON* length = cJSON_GetObjectItem(json, "length");
        cJSON* preview_start_time = cJSON_GetObjectItem(json, "preview_start_time");
        if (!cJSON_IsString(album) || !cJSON_IsString(release_year) || !cJSON_IsNumber(length)) {
            cJSON_Delete(json);
            continue;
        }

        cJSON* genres = cJSON_GetObjectItem(json, "genres");
        int genre_count = 0;
        char* genre_list[16];
        if (cJSON_IsArray(genres)) {
            genre_count = cJSON_GetArraySize(genres);
            if (genre_count > 16) genre_count = 16;
            for (int i = 0; i < genre_count; i++) {
                cJSON* genre = cJSON_GetArrayItem(genres, i);
                if (cJSON_IsString(genre)) {
                    genre_list[i] = strdup(genre->valuestring);
                } else {
                    genre_list[i] = strdup("Unknown");
                }
            }
        }

        cJSON* charters = cJSON_GetObjectItem(json, "charters");
        int charter_count = 0;
        char* charter_list[16];
        if (cJSON_IsArray(charters)) {
            charter_count = cJSON_GetArraySize(charters);
            if (charter_count > 16) charter_count = 16;
            for (int i = 0; i < charter_count; i++) {
                cJSON* charter = cJSON_GetArrayItem(charters, i);
                if (cJSON_IsString(charter)) {
                    charter_list[i] = strdup(charter->valuestring);
                } else {
                    charter_list[i] = strdup("Unknown");
                }
            }
        }

        cJSON* diff = cJSON_GetObjectItem(json, "diff");
        int diff_guitar = 0, diff_vocals = 0, diff_drums = 0, diff_bass = 0;
        if (cJSON_IsObject(diff)) {
            cJSON* guitar = cJSON_GetObjectItem(diff, "guitar");
            cJSON* vocals = cJSON_GetObjectItem(diff, "vocals");
            cJSON* drums = cJSON_GetObjectItem(diff, "drums");
            cJSON* bass = cJSON_GetObjectItem(diff, "bass");
            if (cJSON_IsNumber(guitar)) diff_guitar = (int)guitar->valuedouble;
            if (cJSON_IsNumber(vocals)) diff_vocals = (int)vocals->valuedouble;
            if (cJSON_IsNumber(drums)) diff_drums = (int)drums->valuedouble;
            if (cJSON_IsNumber(bass)) diff_bass = (int)bass->valuedouble;
        }

        cJSON* stems = cJSON_GetObjectItem(json, "stems");
        if (!cJSON_IsObject(stems)) {
            cJSON_Delete(json);
            continue;
        }

        int stem_count = 0;
        char* stem_list[32];
        cJSON* stem = stems->child;
        while (stem && stem_count < 32) {
            if (cJSON_IsString(stem)) {
                char stem_path[512];
                snprintf(stem_path, sizeof(stem_path), "%s/%s/%s", FOLDER_PATH, entry->d_name, stem->valuestring);
                if (stat(stem_path, &st) == 0) {
                    stem_list[stem_count] = strdup(stem_path);
                    stem_count++;
                }
            } else if (cJSON_IsArray(stem)) {
                int array_size = cJSON_GetArraySize(stem);
                for (int i = 0; i < array_size && stem_count < 32; i++) {
                    cJSON* sub_stem = cJSON_GetArrayItem(stem, i);
                    if (cJSON_IsString(sub_stem)) {
                        char stem_path[512];
                        snprintf(stem_path, sizeof(stem_path), "%s/%s/%s", FOLDER_PATH, entry->d_name, sub_stem->valuestring);
                        if (stat(stem_path, &st) == 0) {
                            stem_list[stem_count] = strdup(stem_path);
                            stem_count++;
                        }
                    }
                }
            }
            stem = stem->next;
        }

        Song* song = &songs[*song_count];
        song->folder_name = strdup(entry->d_name);
        song->title = strdup(title->valuestring);
        song->artist = strdup(artist->valuestring);
        song->midi_path = strdup(midi_path);
        song->album = strdup(album->valuestring);
        song->release_year = strdup(release_year->valuestring);
        song->length = (int)length->valuedouble;
        song->preview_start_time = cJSON_IsNumber(preview_start_time) ? (float)preview_start_time->valuedouble : 0.0f;
        song->diff_guitar = diff_guitar;
        song->diff_vocals = diff_vocals;
        song->diff_drums = diff_drums;
        song->diff_bass = diff_bass;
        song->genres = (char**)malloc(genre_count * sizeof(char*));
        song->genre_count = genre_count;
        for (int i = 0; i < genre_count; i++) {
            song->genres[i] = genre_list[i];
        }
        song->charters = (char**)malloc(charter_count * sizeof(char*));
        song->charter_count = charter_count;
        for (int i = 0; i < charter_count; i++) {
            song->charters[i] = charter_list[i];
        }
        song->stems = (char**)malloc(stem_count * sizeof(char*));
        song->stem_count = stem_count;
        song->music = (Music*)malloc(stem_count * sizeof(Music));
        for (int i = 0; i < stem_count; i++) {
            song->stems[i] = stem_list[i];
            song->music[i] = LoadMusicStream(stem_list[i]);
        }
        song->is_playing = false;
        (*song_count)++;

        cJSON_Delete(json);
    }
    closedir(dir);
    return true;
}

void toggle_song_playback(Song* song, bool* playing) {
    if (*playing) {
        for (int i = 0; i < song->stem_count; i++) {
            StopMusicStream(song->music[i]);
        }
        song->is_playing = false;
        *playing = false;
    } else {
        for (int i = 0; i < song->stem_count; i++) {
            PlayMusicStream(song->music[i]);
        }
        song->is_playing = true;
        *playing = true;
    }
}

void play_song_preview(Song* song, bool* playing) {
    if (*playing) {
        for (int i = 0; i < song->stem_count; i++) {
            StopMusicStream(song->music[i]);
        }
        song->is_playing = false;
        *playing = false;
    }
    for (int i = 0; i < song->stem_count; i++) {
        PlayMusicStream(song->music[i]);
        SeekMusicStream(song->music[i], song->preview_start_time);
    }
    song->is_playing = true;
    *playing = true;
}

void update_songs(Song* songs, int song_count, int playing_song_index) {
    if (playing_song_index >= 0 && playing_song_index < song_count && songs[playing_song_index].is_playing) {
        Song* song = &songs[playing_song_index];
        for (int i = 0; i < song->stem_count; i++) {
            if (song->music[i].stream.buffer != NULL) {
                UpdateMusicStream(song->music[i]);
            }
        }
    }
}

void free_songs(Song* songs, int song_count) {
    for (int i = 0; i < song_count; i++) {
        for (int j = 0; j < songs[i].stem_count; j++) {
            UnloadMusicStream(songs[i].music[j]);
            free(songs[i].stems[j]);
        }
        free(songs[i].stems);
        free(songs[i].music);
        for (int j = 0; j < songs[i].genre_count; j++) {
            free(songs[i].genres[j]);
        }
        free(songs[i].genres);
        for (int j = 0; j < songs[i].charter_count; j++) {
            free(songs[i].charters[j]);
        }
        free(songs[i].charters);
        free(songs[i].folder_name);
        free(songs[i].title);
        free(songs[i].artist);
        free(songs[i].midi_path);
        free(songs[i].album);
        free(songs[i].release_year);
    }
}

void get_song_details(Song* song, char* buffer, int buffer_size) {
    char temp[512] = {0};
    snprintf(temp, sizeof(temp), "Title: %s\nArtist: %s\nAlbum: %s\nYear: %s\nLength: %d:%02d\nPreview Start: %.1f sec\n",
             song->title, song->artist, song->album, song->release_year,
             song->length / 60, song->length % 60, song->preview_start_time);
    strncat(buffer, temp, buffer_size - strlen(buffer) - 1);

    snprintf(temp, sizeof(temp), "Genres: ");
    strncat(buffer, temp, buffer_size - strlen(buffer) - 1);
    for (int i = 0; i < song->genre_count; i++) {
        strncat(buffer, song->genres[i], buffer_size - strlen(buffer) - 1);
        if (i < song->genre_count - 1) {
            strncat(buffer, ", ", buffer_size - strlen(buffer) - 1);
        }
    }
    strncat(buffer, "\n", buffer_size - strlen(buffer) - 1);

    snprintf(temp, sizeof(temp), "Charters: ");
    strncat(buffer, temp, buffer_size - strlen(buffer) - 1);
    for (int i = 0; i < song->charter_count; i++) {
        strncat(buffer, song->charters[i], buffer_size - strlen(buffer) - 1);
        if (i < song->charter_count - 1) {
            strncat(buffer, ", ", buffer_size - strlen(buffer) - 1);
        }
    }
    strncat(buffer, "\n", buffer_size - strlen(buffer) - 1);

    snprintf(temp, sizeof(temp), "Difficulty:\n  Guitar: %d\n  Vocals: %d\n  Drums: %d\n  Bass: %d",
             song->diff_guitar, song->diff_vocals, song->diff_drums, song->diff_bass);
    strncat(buffer, temp, buffer_size - strlen(buffer) - 1);
}