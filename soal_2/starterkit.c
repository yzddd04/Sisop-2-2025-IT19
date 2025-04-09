#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#define MAX_PATH 1024
#define LOG_FILE "activity.log"
#define STARTER_KIT_DIR "starter_kit"
#define QUARANTINE_DIR "quarantine"
#define PID_FILE "decrypt.pid"

pid_t decrypt_pid = -1;

void write_log(const char *action, const char *filename, pid_t pid) {
    time_t now;
    struct tm *tm_info;
    char timestamp[20];
    
    time(&now);
    tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%d-%m-%Y][%H:%M:%S", tm_info);
    
    FILE *log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        perror("Failed to open log file");
        return;
    }
    
    if (strcmp(action, "Decrypt") == 0) {
        fprintf(log_file, "[%s] - Successfully started decryption process with PID %d.\n", 
                timestamp, pid);
    } 
    else if (strcmp(action, "Quarantine") == 0) {
        fprintf(log_file, "[%s] - %s - Successfully moved to quarantine directory.\n", 
                timestamp, filename);
    } 
    else if (strcmp(action, "Return") == 0) {
        fprintf(log_file, "[%s] - %s - Successfully returned to starter kit directory.\n", 
                timestamp, filename);
    } 
    else if (strcmp(action, "Eradicate") == 0) {
        fprintf(log_file, "[%s] - %s - Successfully deleted.\n", 
                timestamp, filename);
    } 
    else if (strcmp(action, "Shutdown") == 0) {
        fprintf(log_file, "[%s] - Successfully shut off decryption process with PID %d.\n", 
                timestamp, pid);
    } 
    else if (strcmp(action, "Download") == 0) {
        fprintf(log_file, "[%s] - Successfully downloaded and extracted starter kit.\n",
                timestamp);
    }
    
    fclose(log_file);
}

char *base64_decode(const char *input) {
    BIO *bio, *b64;
    char *buffer = (char *)malloc(strlen(input) + 1);
    memset(buffer, 0, strlen(input) + 1);
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf((void *)input, -1);
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_read(bio, buffer, strlen(input));
    BIO_free_all(bio);
    
    return buffer;
}

void decrypt_filename(const char *filename) {
    char *dot = strrchr(filename, '.');
    if (dot == NULL) return;
    
    char *encoded = strndup(filename, dot - filename);
    char *decoded = base64_decode(encoded);
    
    char new_name[MAX_PATH];
    snprintf(new_name, sizeof(new_name), "%s%s", decoded, dot);
    
    if (rename(filename, new_name) == 0) {
        printf("Decrypted: %s -> %s\n", filename, new_name);
    } else {
        perror("Failed to rename decrypted file");
    }
    
    free(encoded);
    free(decoded);
}

void decrypt_daemon() {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    
    if (pid > 0) {
        decrypt_pid = pid;
        write_log("Decrypt", "", pid);
        printf("Decryption daemon started with PID: %d\n", pid);
        
        // Save PID to file
        FILE *pid_file = fopen(PID_FILE, "w");
        if (pid_file) {
            fprintf(pid_file, "%d", pid);
            fclose(pid_file);
        }
        return;
    }
    
    // Daemon process
    umask(0);
    setsid();
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    while (1) {
        DIR *dir = opendir(QUARANTINE_DIR);
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_REG) {
                    char full_path[MAX_PATH];
                    snprintf(full_path, sizeof(full_path), "%s/%s", QUARANTINE_DIR, entry->d_name);
                    decrypt_filename(full_path);
                }
            }
            closedir(dir);
        }
        sleep(5);
    }
}

void move_files(const char *src_dir, const char *dest_dir, const char *action) {
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir(src_dir);
    if (dir == NULL) {
        perror("Failed to open directory");
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char src_path[MAX_PATH];
            char dest_path[MAX_PATH];
            
            snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);
            snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, entry->d_name);
            
            if (rename(src_path, dest_path) == 0) {
                write_log(action, entry->d_name, 0);
                printf("Moved %s to %s\n", entry->d_name, dest_dir);
            } else {
                perror("Failed to move file");
            }
        }
    }
    
    closedir(dir);
}

void eradicate_files() {
    DIR *dir = opendir(QUARANTINE_DIR);
    if (!dir) {
        perror("Failed to open quarantine directory");
        return;
    }
    
    struct dirent *entry;
    int files_deleted = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char file_path[MAX_PATH];
            snprintf(file_path, sizeof(file_path), "%s/%s", QUARANTINE_DIR, entry->d_name);
            
            if (remove(file_path) == 0) {
                write_log("Eradicate", entry->d_name, 0);
                printf("Deleted %s\n", entry->d_name);
                files_deleted++;
            } else {
                perror("Failed to delete file");
            }
        }
    }
    closedir(dir);
    
    if (files_deleted == 0) {
        printf("No files found to eradicate in quarantine directory\n");
    }
}

void shutdown_daemon() {
    // Read PID from file if not set
    if (decrypt_pid == -1) {
        FILE *pid_file = fopen(PID_FILE, "r");
        if (pid_file) {
            fscanf(pid_file, "%d", &decrypt_pid);
            fclose(pid_file);
        }
    }
    
    if (decrypt_pid == -1) {
        printf("No decryption process running\n");
        return;
    }
    
    if (kill(decrypt_pid, SIGTERM) == 0) {
        write_log("Shutdown", "", decrypt_pid);
        printf("Successfully shut down decryption process with PID %d\n", decrypt_pid);
        remove(PID_FILE);
        decrypt_pid = -1;
    } else {
        perror("Failed to shut down decryption process");
    }
}

void download_starter_kit() {
    struct stat st = {0};
    if (stat(STARTER_KIT_DIR, &st) == -1) {
        printf("Downloading starter kit...\n");
        int result = system("wget --no-check-certificate 'https://drive.google.com/uc?export=download&id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS' -O starter_kit.zip");
        if (result != 0) {
            printf("Failed to download starter kit\n");
            return;
        }
        
        printf("Extracting starter kit...\n");
        result = system("unzip -q starter_kit.zip -d starter_kit_tmp");
        if (result != 0) {
            printf("Failed to extract starter kit\n");
            return;
        }
        
        remove("starter_kit.zip");
        
        if (rename("starter_kit_tmp", STARTER_KIT_DIR) != 0) {
            perror("Failed to rename directory");
            return;
        }
        
        write_log("Download", "", 0);
        printf("Starter kit downloaded and extracted successfully\n");
    } else {
        printf("Starter kit already exists\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s [--download|--decrypt|--quarantine|--return|--eradicate|--shutdown]\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    mkdir(QUARANTINE_DIR, 0755);
    
    if (strcmp(argv[1], "--download") == 0) {
        download_starter_kit();
    } 
    else if (strcmp(argv[1], "--decrypt") == 0) {
        decrypt_daemon();
    } 
    else if (strcmp(argv[1], "--quarantine") == 0) {
        move_files(STARTER_KIT_DIR, QUARANTINE_DIR, "Quarantine");
    } 
    else if (strcmp(argv[1], "--return") == 0) {
        move_files(QUARANTINE_DIR, STARTER_KIT_DIR, "Return");
    } 
    else if (strcmp(argv[1], "--eradicate") == 0) {
        eradicate_files();
    } 
    else if (strcmp(argv[1], "--shutdown") == 0) {
        shutdown_daemon();
    } 
    else {
        printf("Invalid argument\n");
        printf("Usage: %s [--download|--decrypt|--quarantine|--return|--eradicate|--shutdown]\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}