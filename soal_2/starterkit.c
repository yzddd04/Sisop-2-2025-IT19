#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <pthread.h>

#define STARTER_KIT "starter_kit"
#define QUARANTINE "quarantine"
#define LOG_FILE "activity.log"
#define ZIP_FILE "starter_kit.zip"
#define PID_FILE "decrypt.pid"

// Bikin folder kalau belum ada
void buat_folder(const char *nama_folder) {
    struct stat s = {0};
    if (stat(nama_folder, &s) == -1) {
        mkdir(nama_folder, 0777);
    }
}

// Bikin catatan log
void catat_log(const char *pesan) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char waktu[64];
    strftime(waktu, sizeof(waktu), "[%d-%m-%Y][%H:%M:%S]", tm);
    fprintf(log, "%s - %s\n", waktu, pesan);
    fclose(log);
}

// Fungsi decode base64 sederhana (buat decrypt nama file)
int base64_decode(const char *input, char *output) {
    static const char tabel[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int val = 0, valb = -8, len = 0;
    for (int i = 0; input[i]; i++) {
        char *p = strchr(tabel, input[i]);
        if (!p) break;
        val = (val << 6) + (p - tabel);
        valb += 6;
        if (valb >= 0) {
            output[len++] = (char)((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    output[len] = '\0';
    return len;
}

// Thread yang jalanin decrypt daemon
void *decrypt_thread(void *arg) {
    DIR *dir = opendir(STARTER_KIT);
    if (!dir) return NULL;

    buat_folder(QUARANTINE);
    struct dirent *file;
    while ((file = readdir(dir)) != NULL) {
        if (file->d_type == DT_REG) {
            char nama_asli[256];
            base64_decode(file->d_name, nama_asli);

            char asal[512], tujuan[512];
            snprintf(asal, sizeof(asal), "%s/%s", STARTER_KIT, file->d_name);
            snprintf(tujuan, sizeof(tujuan), "%s/%s", QUARANTINE, nama_asli);

            rename(asal, tujuan);

            char log_msg[512];
            snprintf(log_msg, sizeof(log_msg), "Successfully moved and decrypted: %s", nama_asli);
            catat_log(log_msg);
        }
    }
    closedir(dir);

    char msg[128];
    snprintf(msg, sizeof(msg), "Successfully started decryption process with PID %d.", getpid());
    catat_log(msg);

    while (1) pause(); // biar daemon tetap hidup
    return NULL;
}

// Mulai proses decrypt sebagai daemon (background)
void mulai_decrypt() {
    pid_t pid = fork();
    if (pid == 0) {
        pthread_t t;
        pthread_create(&t, NULL, decrypt_thread, NULL);

        FILE *pidfile = fopen(PID_FILE, "w");
        if (pidfile) {
            fprintf(pidfile, "%d", getpid());
            fclose(pidfile);
        }

        pthread_join(t, NULL);
        exit(0);
    } else {
        printf("Decrypt daemon running with PID %d\n", pid);
    }
}

// Pindahkan file dari starter_kit ke quarantine
void quarantine_file() {
    DIR *dir = opendir(STARTER_KIT);
    if (!dir) return;

    buat_folder(QUARANTINE);
    struct dirent *file;
    while ((file = readdir(dir)) != NULL) {
        if (file->d_type == DT_REG) {
            char asal[512], tujuan[512];
            snprintf(asal, sizeof(asal), "%s/%s", STARTER_KIT, file->d_name);
            snprintf(tujuan, sizeof(tujuan), "%s/%s", QUARANTINE, file->d_name);

            rename(asal, tujuan);

            char log_msg[512];
            snprintf(log_msg, sizeof(log_msg), "%s - Successfully moved to quarantine directory.", file->d_name);
            catat_log(log_msg);
        }
    }
    closedir(dir);
}

// Balikin file dari quarantine ke starter_kit
void balikin_file() {
    DIR *dir = opendir(QUARANTINE);
    if (!dir) return;

    struct dirent *file;
    while ((file = readdir(dir)) != NULL) {
        if (file->d_type == DT_REG) {
            char asal[512], tujuan[512];
            snprintf(asal, sizeof(asal), "%s/%s", QUARANTINE, file->d_name);
            snprintf(tujuan, sizeof(tujuan), "%s/%s", STARTER_KIT, file->d_name);

            rename(asal, tujuan);

            char log_msg[512];
            snprintf(log_msg, sizeof(log_msg), "%s - Successfully returned to starter kit directory.", file->d_name);
            catat_log(log_msg);
        }
    }
    closedir(dir);
}

// Hapus semua file di quarantine
void hapus_karantina() {
    DIR *dir = opendir(QUARANTINE);
    if (!dir) return;

    struct dirent *file;
    while ((file = readdir(dir)) != NULL) {
        if (file->d_type == DT_REG) {
            char path[512];
            snprintf(path, sizeof(path), "%s/%s", QUARANTINE, file->d_name);
            unlink(path);

            char log_msg[512];
            snprintf(log_msg, sizeof(log_msg), "%s - Successfully deleted.", file->d_name);
            catat_log(log_msg);
        }
    }
    closedir(dir);
}

// Matikan proses decrypt daemon
void matikan_decrypt() {
    FILE *f = fopen(PID_FILE, "r");
    if (!f) {
        printf("PID file not found.\n");
        return;
    }

    int pid;
    fscanf(f, "%d", &pid);
    fclose(f);

    if (kill(pid, SIGTERM) == 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Successfully shut off decryption process with PID %d.", pid);
        catat_log(msg);
        printf("Decrypt process with PID %d stopped.\n", pid);
    } else {
        perror("Gagal mematikan proses decrypt");
    }
}

// Tampilkan petunjuk penggunaan
void help() {
    printf("Cara pakai:\n");
    printf("  ./starterkit               → download & extract zip\n");
    printf("  ./starterkit --decrypt     → mulai proses decrypt (daemon)\n");
    printf("  ./starterkit --quarantine  → pindah file ke quarantine\n");
    printf("  ./starterkit --return      → kembalikan file ke starter_kit\n");
    printf("  ./starterkit --eradicate   → hapus semua file di quarantine\n");
    printf("  ./starterkit --shutdown    → matikan decrypt daemon\n");
}

int main(int argc, char *argv[]) {
    buat_folder(STARTER_KIT);
    buat_folder(QUARANTINE);

    if (argc == 1) {
        // download dan unzip starter kit
        pid_t pid = fork();
        if (pid == 0) {
            char *args[] = {
                "wget", "-O", ZIP_FILE,
                "https://drive.google.com/uc?id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS&export=download",
                NULL
            };
            execvp("wget", args);
            perror("wget gagal");
            exit(1);
        } else {
            wait(NULL);
            pid = fork();
            if (pid == 0) {
                char *args[] = {"unzip", ZIP_FILE, "-d", STARTER_KIT, NULL};
                execvp("unzip", args);
                perror("unzip gagal");
                exit(1);
            } else {
                wait(NULL);
                unlink(ZIP_FILE);
                printf("Berhasil di-download dan di-extract ke starter_kit/\n");
            }
        }
    } else if (argc == 2) {
        if (strcmp(argv[1], "--decrypt") == 0) mulai_decrypt();
        else if (strcmp(argv[1], "--quarantine") == 0) quarantine_file();
        else if (strcmp(argv[1], "--return") == 0) balikin_file();
        else if (strcmp(argv[1], "--eradicate") == 0) hapus_karantina();
        else if (strcmp(argv[1], "--shutdown") == 0) matikan_decrypt();
        else help();
    } else {
        help();
    }

    return 0;
}