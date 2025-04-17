# Sisop-2-2025-IT10

## Member

| No  | Nama                   | NRP        |
| --- | ---------------------- | ---------- |
| 1   | Ahmad Yazid Arifuddin  | 5027241040 |
| 2   | Muhammad Ziddan Habibi | 5027241122 |
| 3   | Andi Naufal Zaki       | 5027241059 |


### Soal 1

#### Penjelasan

- A. Downloading the Clues
- B. Filtering the Files
- C. Combine the File Content
- D. Decode the file
- E. Password Check

##### Help Command

```c
./action
./action Filter
./action Combine
./action Decode
```


A. Pertama, cek apakah file sudah ada atau belum, jika sudah ada maka skip download folder, jika belum ada maka mendownload drive nya dahulu, maka donwload menggunakan wget untuk mengunduh Clues.zip dari Google Drive dengan command `system("wget --no-check-certificate 'https://drive.google.com/uc?export=download&id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK' -O Clues.zip");`, kemudian setelah download, file langsung diekstrak menggunakan unzip `system ("unzip Clues.zip;` dan menge print output text `printf("Unzipping Clues.zip...\n");`, setelah itu menghapus secara otomatis file zip nya dengan command `remove("Clues.zip");` dan mengeprint `printf("Removing Clues.zip...\n");`
```c
void downloadClues() {
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
}
```

B. Setelah berhasil download, membuat file direktori Filtered, kemudian memfilter folder petunjuk file file ClueA, ClueB, ClueC, ClueD. Setelah itu di setiap folder, akan mencari file `.txt` dengan nama 1 karakter alfanumerik seperti a.txt, 1.txt, dll. Kemudian memindahkan file nya ke folder Filtered, kemudian menghapus semua file tersisa di dalam folder ClueA-D yang tidak dipindahkan.
```
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

```

C. Setelah itu, membuat file Combined.txt, kemudian membuka folder Filtered dengan command `if ((dir = opendir("Filtered")) == NULL) {
        perror("Failed to open Filtered directory");
        fclose(combined);
        return;
    }`, kemudian membaca dan menggabungkan isi file berama 1.txt sampai 9.txt, a-z.txt ke dalam combined.txt, dan untuk setiap angka i, juga gabungkan isi file a.txt hingga i-th letter.txt (a.txt, b.txt, ..., i-th letter.txt) ke dalam Combined.txt. Kemdudian menghapus setiap file setelah digabungkan.

```
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
```

D. Kemudian mendecode the file, pertama tama membuka file Combined.txt untuk membaca isi nya, kemudian membuka file Decoded.txt dan melakkan decode ROT13 pada setiap karakter, kemudian menulis hasil nya ke Decoded.txt.
```
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
```

E. Kemudian masuk ke link website untuk mengecek password dari hasil decode tersebut.
![image](https://github.com/user-attachments/assets/cfe1fd95-2458-4eb3-9d3f-17cfdaaf3be5)

    


## Nomer 4 (Soal_4)

### a)mengetahui semua aktivitas user

![image](https://github.com/user-attachments/assets/50abde7d-f2dc-4766-96d3-ffb252caf513)

Soal pada gambar tersebut menjelaskan sebuah skenario di mana Doraemon ingin mengetahui semua proses atau aktivitas yang sedang dijalankan oleh user tertentu di komputer. Program debugmon ini akan menampilkan daftar proses yang dijalankan oleh user tersebut, termasuk informasi berikut:
- PID (Process ID): ID unik dari proses.
- Command: Perintah atau nama program yang dijalankan.
- CPU usage: Seberapa banyak CPU yang digunakan oleh proses itu.
- Memory usage: Seberapa banyak memori RAM yang digunakan oleh proses itu.

Materi yang digunakan untuk mengerjakan ini:
Untuk memahami dan mengerjakan soal ini, kamu perlu menguasai materi berikut:
- Shell Scripting / Bash
- Perintah Linux untuk memantau proses
- Penggunaan Pipe (|) dan Redirect (>, <)

### b)Memasang mata-mata dalam mode daemon

![image](https://github.com/user-attachments/assets/e5e612d1-a9fd-4fc8-ab41-7642ae772ffb)

Penjelasan Maksud Soal:
Daemon adalah program yang berjalan di background (latar belakang) secara terus-menerus, seperti layanan sistem.

Doraemon ingin debugmon:
- Tetap berjalan tanpa interaksi manual
- Terus memantau proses milik `<user>`
- Menyimpan hasil pemantauan ke dalam `file.log`, bukan hanya tampil di layar

Jadi, saat Doraemon mengetik perintah di atas:
- debugmon akan berjalan sebagai proses daemon
- Secara berkala akan mengambil data proses use
- Mencatat hasilnya ke dalam file log (misal 'log_<user>.txt')

Materi yang Digunakan:
- Konsep Daemon di Linux
- Shell Scripting Lanjutan
- Pemrosesan Proses di Linux
- Penanganan File
- (Opsional) Signal Handling

### c)Menghentikan pengawasan

![image](https://github.com/user-attachments/assets/b5e8f0b0-03ac-4fa3-9c05-fa778a4d64ea)

Penjelasan Maksud Soal:
- debugmon sebelumnya dijalankan sebagai 'daemon' yang memantau proses user dan mencatatnya ke log.
- Perintah 'stop <user>' digunakan untuk menghentikan daemon yang memantau user tersebut.
- Artinya, debugmon harus:
  - Mencari proses daemon yang sedang berjalan untuk <user>
  - Menghentikan proses tersebut (misalnya dengan perintah kill)

 Materi yang Digunakan untuk Mengerjakan:
- Manajemen Proses di Linux
  - Mengetahui bagaimana proses berjalan di background
  - Menggunakan perintah 'ps', 'pgrep', atau 'pidof' untuk mencari proses berdasarkan nama atau argumen
  - Menghentikan proses dengan 'kill', 'killall', atau 'pkill'
- Shell Scripting
  - Menyimpan dan membaca `PID` dari `file .pid` (opsional, agar proses bisa dihentikan dengan tepat)
  - Pengecekan apakah proses masih berjalan
- Pengelolaan File dan Log
  - Jika script menyimpan PID ke dalam file (misalnya `debugmon_<user>.pid`), maka 'stop' dapat membaca file tersebut dan 'kill' berdasarkan isinya



### d)Menggagalkan proses user yang sedang berjalan

![image](https://github.com/user-attachments/assets/2135303d-455d-434b-a7b1-6c1db43c00fc)

Penjelasan Maksud Soal:
`debugmon fail <user>` adalah perintah untuk:
- Menghentikan (`kill`) semua proses milik user tersebut yang sedang aktif.
- Menulis informasi proses tersebut ke dalam log file dengan status FAILED.
- Mencegah user menjalankan proses baru selama mode ini aktif (semacam "blokir sementara").

Materi yang Digunakan untuk Mengerjakan Ini:
1. Manajemen Proses di Linux
2. Shell Scripting (lanjutan)
Looping proses untuk log dan kill
Redirect log ke file, misalnya `log_<user>.txt`, dengan tambahan status FAILED
3. Blokir User dari Menjalankan Proses (Opsional, Lanjutan)

### e)Mengizinkan user untuk kembali menjalankan proses

![image](https://github.com/user-attachments/assets/61da9848-1805-4fe9-aa2f-b9bfd73c08c5)

Penjelasan Maksud Soal:
`debugmon revert <user>` dipakai untuk mengembalikan user ke kondisi normal setelah sebelumnya berada dalam mode FAIL.
Dengan kata lain:
User yang tadinya tidak bisa menjalankan proses apa pun → sekarang sudah bisa lagi.
Status pengawasan "fail" dihapus.

Materi yang Digunakan untuk Mengerjakan Ini:
1. Shell Scripting (lanjutan)
Menghapus flag atau file penanda bahwa user sedang dalam mode FAIL

2. Logika Pemulihan Sistem
Jika sebelumnya mode FAIL mencegah eksekusi proses via script daemon atau pemantauan background, maka revert akan:
Menghapus indikator mode FAIL
Memberi pesan bahwa mode normal telah dipulihkan



### f)Mencatat ke dalam file log

![image](https://github.com/user-attachments/assets/1e896d4e-fe31-4767-9da8-1b9226f7dd3b)



