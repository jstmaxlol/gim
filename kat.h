// kat.cfg (kat.h)
// with love by czjstmax <jstmaxlol@disroot.org>
// header version 1

#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef KAT
#define KAT

struct kat_api {
    int (*create)(const char*);
    int (*add)(const char*, const char*, const char*);
    int (*read)(const char*, const char*, char*, size_t);
    int (*del)(const char*, const char*);
    int (*edit)(const char*, const char*, const char*);
    int (*addComment)(const char*, const char*);
};

// Creates a file
int katcreate(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return 1;
    fclose(f);
    return 0; // success
}

// Appends a key=val to file
int katadd(const char *key, const char *val, const char *path) {
    FILE *f = fopen(path, "a");
    if (f) {
        fprintf(f, "%s=%s\n", key, val);
        fclose(f);
        return 0;
    }
    return 1;
}

// Appends a comment to file
int kataddcomment(const char *str, const char *path) {
    FILE *f = fopen(path, "a");
    if (f) {
        fprintf(f, "#%s\n", str);
        fclose(f);
        return 0;
    }
    return 1;
}

// Reads a key from file
int katreadkey(const char *key, const char *path, char *buffer, size_t bufsize) {
    FILE *f = fopen(path, "r");
    if (!f) return 1;

    char line[1024];
    size_t key_len = strlen(key);

    while (fgets(line, sizeof(line), f)) {
        // TODO: skip leading whitespace maybe?
        // check if line starts with key and '=' right after
        if (strncmp(line, key, key_len) == 0 && line[key_len] == '=') {
            if (line[0] == '#' || line[0] == ';' || line[0] == '\n') continue;
            // copy everything after '=' until newline or buffer full (aka bad)
            char *val_start = line + key_len + 1;
            size_t len = strcspn(val_start, "\r\n"); // length until newline
            if (len >= bufsize) len = bufsize - 1;
            strncpy(buffer, val_start, len);
            buffer[len] = '\0';
            fclose(f);
            return 0;  // success
        }
    }

    fclose(f);
    return 1;
}

// Deletes a key from file
int katdelkey(const char *key, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return 1;

    char temp_path[1024];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", path);
    FILE *temp = fopen(temp_path, "w");
    if (!temp) {
        fclose(f);
        return 1;
    }

    char line[1024];
    size_t key_len = strlen(key);
    int deleted = 0;

    while (fgets(line, sizeof(line), f)) {
        // skip line if it matches 'key='
        if (strncmp(line, key, key_len) == 0 && line[key_len] == '=') {
            deleted = 1;
            continue;
        }
        fputs(line, temp);
    }

    fclose(f);
    fclose(temp);

    if (deleted) {
        // replace original file with temp
        if (remove(path) != 0) return 1;
        if (rename(temp_path, path) != 0) return 1;
    } else {
        remove(temp_path);
    }

    return deleted ? 0 : 1; // 0 if deleted, 1 if not found
}

// Edits a key's value from file
int kateditkey(const char *key, const char *new_val, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return 1;

    char temp_path[1024];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", path);
    FILE *temp = fopen(temp_path, "w");
    if (!temp) {
        fclose(f);
        return 1;
    }

    char line[1024];
    size_t key_len = strlen(key);
    int found = 0;

    while (fgets(line, sizeof(line), f)) {
        // skip leading whitespace for accurate matching
        char *start = line;
        while (*start == ' ' || *start == '\t') start++;

        if (strncmp(start, key, key_len) == 0 && start[key_len] == '=') {
            // replace line with new key=val
            fprintf(temp, "%s=%s\n", key, new_val);
            found = 1;
        } else {
            fputs(line, temp);
        }
    }

    if (!found) {
        // key not found, add it at the end
        fprintf(temp, "%s=%s\n", key, new_val);
    }

    fclose(f);
    fclose(temp);

    if (remove(path) != 0) return 1;
    if (rename(temp_path, path) != 0) return 1;

    return 0;
}

struct kat_api kat = {
    .create = katcreate,
    .add = katadd,
    .read = katreadkey,
    .del = katdelkey,
    .edit = kateditkey,
    .addComment = kataddcomment,
};

#endif
