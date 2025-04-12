#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <pwd.h>

#define LOG_FILE "debugmon.log"
#define PID_FILE "debugmon.pid"
#define MAX_CMD 256
#define MAX_PATH 512

//mengecek apakah proses dengan PID tertentu dimiliki oleh user
int is_process_owned_by_user(const char *pid, const char *username) {
    char path[MAX_PATH], owner[64];
    snprintf(path, sizeof(path), "/proc/%s/status", pid);
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    while (fgets(path, sizeof(path), f)) {
     if (strncmp(path, "Uid:", 4) == 0) {
         uid_t uid;
         sscanf(path, "Uid:\t%u", &uid);
         struct passwd *pw = getpwuid(uid);
         if (pw && strcmp(pw->pw_name, username) == 0) {
              fclose(f);
              return 1;
          }
      }
  }
    fclose(f);
    return 0;
}

//mencatat proses dan status Nge RUN atau FAILED nya kedalam file .log
void log_process(const char *proc_name, const char *status) {
  FILE *log = fopen(LOG_FILE, "a");
  if (!log) return;
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  fprintf(log, "[%02d:%02d:%04d]-[%02d:%02d:%02d]_%s_%s\n",
         t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
         t->tm_hour, t->tm_min, t->tm_sec,
         proc_name, status);
  fclose(log);
}

//menampilkan daftar proses yang terjadi yang dimiliki oleh user
void list_processes(const char *username) {
  DIR *dir = opendir("/proc"); //Membuka direktori /proc, yaitu direktori virtual di Linux yang berisi informasi tentang semua proses yang sedang berjalan.
  struct dirent *entry;
  printf("PID\tCMD\tCPU\tMEM\n"); //Menampilkan header (judul kolom) di terminal:
  while ((entry = readdir(dir)) != NULL) {  //Melakukan loop untuk membaca setiap entri dalam direktori /proc.Akan berhenti jika sudah tidak ada entri lagi (readdir() mengembalikan NULL).
      if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
        if (is_process_owned_by_user(entry->d_name, username)) {
            char cmd[MAX_PATH];
            snprintf(cmd, sizeof(cmd), "ps -p %s -o pid=,comm=,%%cpu=,%%mem=", entry->d_name);
            system(cmd);
        }
    }
 }
 closedir(dir); //menutup direktori /proc
}

//menjalankan proses daemon yang memantau proses user dibalik layar
void daemonize(const char *username) { //menerima parameter username untuk user siapa yang akan dipantau
 pid_t pid = fork(); //membuat child process dengan fork
 if (pid < 0) exit(1);
 if (pid > 0) {
   FILE *pid_file = fopen(PID_FILE, "w");
   if (pid_file) {
        fprintf(pid_file, "%d", pid);
        fclose(pid_file);
    }
   exit(0);
}
setsid(); //membuat proses baru
while (1) {
    DIR *dir = opendir("/proc");
    struct dirent *entry;       //Buka direktori /proc dan siapkan entry untuk membaca isi direktori tersebut
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            if (is_process_owned_by_user(entry->d_name, username)) {
                log_process(entry->d_name, "RUNNING");
            }
        }
    }
    closedir(dir);
    sleep(5);
}
}

//untuk menghentikan daemon
void stop_daemon(const char *username) {
    FILE *pid_file = fopen(PID_FILE, "r");
    if (!pid_file) {
        perror("Gagal membuka file PID. Apakah daemon sedang berjalan?");
        return;
    }

    int pid;
    fscanf(pid_file, "%d", &pid);
    fclose(pid_file);

    if (kill(pid, SIGTERM) == 0) {
        printf("Proses daemon (PID %d) dihentikan.\n", pid);
        remove(PID_FILE); // hapus file PID
    } else {
        perror("Gagal menghentikan daemon");
    }
}

//menghentikan atau kill semua proses user
void fail_processes(const char *username) {
    DIR *dir = opendir("/proc");
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            if (is_process_owned_by_user(entry->d_name, username)) {
                char path[256], process_name[256];
                snprintf(path, sizeof(path), "/proc/%.20s/comm", entry->d_name);
                FILE *fp = fopen(path, "r");
                if (fp) {
                    if (fgets(process_name, sizeof(process_name), fp) != NULL) {
                        strtok(process_name, "\n");
                        log_process(process_name, "FAILED");
                    }
                    fclose(fp);
                }
                kill(atoi(entry->d_name), SIGKILL);
            }
        }
    }
    closedir(dir);
}

//mengembalikan kembali proses untuk berjalan
void revert_processes(const char *username) {
    log_process("revert", "RUNNING");
    printf("User %s sekarang bisa menjalankan proses lagi\n", username);
}


void tulis_log(const char *nama_proses, const char *status) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) {
        perror("Gagal membuka file log");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(log, "[%02d:%02d:%04d]-[%02d:%02d:%02d]_%s_%s\n",
        t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
        t->tm_hour, t->tm_min, t->tm_sec,
        nama_proses, status);
    fclose(log);
}

int main(int argc, char *argv[]) { //program dijalankan dengan <command> <user>
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <command> <user>\n", argv[0]);
        
        return 1;
    }  //kode ini akan menentukan perintah apa yang diminta oleh pengguna saat menjalankan program lewat terminal.
    if (strcmp(argv[1], "list") == 0) {
        list_processes(argv[2]);
    } else if (strcmp(argv[1], "daemon") == 0) {
        daemonize(argv[2]);
    } else if (strcmp(argv[1], "stop") == 0) {
        stop_daemon(argv[2]);
    } else if (strcmp(argv[1], "fail") == 0) {
        fail_processes(argv[2]);
    } else if (strcmp(argv[1], "revert") == 0) {
        revert_processes(argv[2]);
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        return 1;
    }
    return 0;
}
