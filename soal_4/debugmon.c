#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <signal.h>
#include <time.h>
#include <dirent.h>

#define MAX_PATH 1024
#define MAX_COMMAND 256
#define LOG_FILE "debugmon.log"

// Fungsi untuk menulis log
void write_log(const char *user, const char *process, const char *status) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Gagal membuka file log");
        return;
    }
    
    time_t now;
    struct tm *t;
    time(&now);
    t = localtime(&now);
    
    fprintf(log_file, "[%02d:%02d:%04d]-[%02d:%02d:%02d]_%s_%s\n", 
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec,
            process, status);
    
    fclose(log_file);
}

// Mendaftar proses user
void list_processes(const char *username) {
    struct passwd *pw = getpwnam(username);
    if (pw == NULL) {
        printf("User tidak ditemukan\n");
        return;
    }
    
    char command[MAX_COMMAND];
    snprintf(command, sizeof(command), 
        "ps -u %s -o pid,comm,%%cpu,%%mem", username);
    
    system(command);
}

// Mode daemon
void daemon_mode(const char *username) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork gagal");
        exit(EXIT_FAILURE);
    }
    
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    umask(0);
    
    if (setsid() < 0) {
        perror("Gagal membuat sesi baru");
        exit(EXIT_FAILURE);
    }
    
    write_log(username, "debugmon", "RUNNING");
    
    while(1) {
        sleep(60);
    }
}

// Hentikan pemantauan
void stop_monitoring(const char *username) {
    write_log(username, "debugmon", "STOPPED");
}

// Gagalkan proses user
void fail_all_processes(const char *username) {
    struct passwd *pw = getpwnam(username);
    if (pw == NULL) {
        printf("User tidak ditemukan\n");
        return;
    }
    
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Gagal membuka file log");
        return;
    }
    
    time_t now;
    struct tm *t;
    time(&now);
    t = localtime(&now);
    
    fprintf(log_file, "[%02d:%02d:%04d]-[%02d:%02d:%02d] FAIL PROCESS DETAILS:\n", 
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec);
    
    char command[MAX_COMMAND];
    FILE *fp;
    char path[1035];
    
    snprintf(command, sizeof(command), "ps -u %s -o pid=", username);
    
    fp = popen(command, "r");
    if (fp == NULL) {
        printf("Gagal menjalankan perintah\n");
        fclose(log_file);
        return;
    }
    
    while (fgets(path, sizeof(path), fp) != NULL) {
        pid_t pid = atoi(path);
        if (pid > 0 && pid != getpid()) {
            kill(pid, SIGTERM);
            fprintf(log_file, "PID %d - TERMINATED\n", pid);
        }
    }
    
    pclose(fp);
    fclose(log_file);
    
    write_log(username, "all_processes", "FAILED");
}

// Kembalikan proses
void revert_processes(const char *username) {
    write_log(username, "debugmon", "RUNNING");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Penggunaan: ./debugmon <mode> <user>\n");
        printf("Mode: list, daemon, stop, fail, revert\n");
        return 1;
    }
    
    char *mode = argv[1];
    char *username = argv[2];
    
    if (strcmp(mode, "list") == 0) {
        list_processes(username);
    }
    else if (strcmp(mode, "daemon") == 0) {
        daemon_mode(username);
    }
    else if (strcmp(mode, "stop") == 0) {
        stop_monitoring(username);
    }
    else if (strcmp(mode, "fail") == 0) {
        fail_all_processes(username);
    }
    else if (strcmp(mode, "revert") == 0) {
        revert_processes(username);
    }
    else {
        printf("Mode tidak valid\n");
        return 1;
    }
    
    return 0;
}