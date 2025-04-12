#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_PATH 1024

void downloadClues();
void filterFiles();
void combineFiles();
void decodeFile();
void printUsage(char *programName);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        downloadClues();
        return 0;
    }

    if (strcmp(argv[1], "Filter") == 0) {
        filterFiles();
    } else if (strcmp(argv[1], "Combine") == 0) {
        combineFiles();
    } else if (strcmp(argv[1], "Decode") == 0) {
        decodeFile();
    } else {
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}

void downloadClues() {
    // Check if Clues directory already exists
    struct stat st = {0};
    if (stat("Clues", &st) == 0) {
        printf("Clues directory already exists. Skipping download.\n");
        return;
    }

    printf("Downloading Clues.zip...\n");
    system("wget --no-check-certificate 'https://drive.google.com/uc?export=download&id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK' -O Clues.zip");

    printf("Unzipping Clues.zip...\n");
    system("unzip Clues.zip");

    printf("Removing Clues.zip...\n");
    remove("Clues.zip");
}

void filterFiles() {
    //  Buat direktori yang difilter jika belum ada
    mkdir("Filtered", 0755);

    DIR *dir;
    struct dirent *ent;
    char path[MAX_PATH];
    char dest[MAX_PATH];

    // Array of clue directories
    char *clueDirs[] = {"Clues/ClueA", "Clues/ClueB", "Clues/ClueC", "Clues/ClueD"};
    int numDirs = sizeof(clueDirs) / sizeof(clueDirs[0]);

    for (int i = 0; i < numDirs; i++) {
        if ((dir = opendir(clueDirs[i])) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                // Skip . and ..
                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                    continue;
                }

                // Periksa apakah nama file cocok dengan pola (1 karakter: huruf atau angka)
                if (strlen(ent->d_name) == 5) { // e.g., "a.txt" is 5 chars
                    char *dot = strrchr(ent->d_name, '.');
                    if (dot && strcmp(dot, ".txt") == 0) {
                        char nameWithoutExt[2];
                        strncpy(nameWithoutExt, ent->d_name, 1);
                        nameWithoutExt[1] = '\0';

                        if (isalnum(nameWithoutExt[0]) && !ispunct(nameWithoutExt[0])) {
                            // Buat source and destination paths
                            snprintf(path, sizeof(path), "%s/%s", clueDirs[i], ent->d_name);
                            snprintf(dest, sizeof(dest), "Filtered/%s", ent->d_name);

                            // Move file
                            rename(path, dest);
                        }
                    }
                }
            }
            closedir(dir);
        }
    }

    // Hapus unfiltered files
    for (int i = 0; i < numDirs; i++) {
        if ((dir = opendir(clueDirs[i])) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                    continue;
                }

                snprintf(path, sizeof(path), "%s/%s", clueDirs[i], ent->d_name);
                remove(path);
            }
            closedir(dir);
        }
    }
}

void combineFiles() {
    DIR *dir;
    struct dirent *ent;
    FILE *combined, *file;
    char content[256];
    char path[MAX_PATH];

    // Open Combined.txt untuk writing
    combined = fopen("Combined.txt", "w");
    if (!combined) {
        perror("Failed to create Combined.txt");
        return;
    }

    // Buka Filtered directory
    if ((dir = opendir("Filtered")) == NULL) {
        perror("Failed to open Filtered directory");
        fclose(combined);
        return;
    }

    // First pass: process number files (1-9)
    for (int i = 1; i <= 9; i++) {
        char filename[10];
        snprintf(filename, sizeof(filename), "%d.txt", i);
        
        // Cek file
        snprintf(path, sizeof(path), "Filtered/%s", filename);
        if (access(path, F_OK) == 0) {
            file = fopen(path, "r");
            if (file) {
                if (fgets(content, sizeof(content), file)) {
                    fputs(content, combined);
                }
                fclose(file);
                remove(path);
            }
        }

        // Proses berkas surat yang sesuai (a-z)
        char letterFile[10];
        snprintf(letterFile, sizeof(letterFile), "%c.txt", 'a' + i - 1);
        
        snprintf(path, sizeof(path), "Filtered/%s", letterFile);
        if (access(path, F_OK) == 0) {
            file = fopen(path, "r");
            if (file) {
                if (fgets(content, sizeof(content), file)) {
                    fputs(content, combined);
                }
                fclose(file);
                remove(path);
            }
        }
    }

    closedir(dir);
    fclose(combined);
}

void decodeFile() {
    FILE *combined, *decoded;
    char ch;

    combined = fopen("Combined.txt", "r");
    if (!combined) {
        perror("Failed to open Combined.txt");
        return;
    }

    decoded = fopen("Decoded.txt", "w");
    if (!decoded) {
        perror("Failed to create Decoded.txt");
        fclose(combined);
        return;
    }

    while ((ch = fgetc(combined)) != EOF) {
        if (isalpha(ch)) {
            char base = isupper(ch) ? 'A' : 'a';
            ch = ((ch - base + 13) % 26) + base;
        }
        fputc(ch, decoded);
    }

    fclose(combined);
    fclose(decoded);
}

void printUsage(char *programName) {
    printf("Usage:\n");
    printf("  %s                   - Download and extract Clues.zip\n", programName);
    printf("  %s Filter            - Filter files\n", programName);
    printf("  %s Combine           - Combine filtered files\n", programName);
    printf("  %s Decode            - Decode Combined.txt\n", programName);
}