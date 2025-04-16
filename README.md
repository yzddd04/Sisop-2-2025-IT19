# Sisop-2-2025-IT19
### Anggota Kelompok:
- Ahmad Yazid Arifuddin (*5027241040*)
- Muhammad Ziddan Habibi (*5027241122*)
- Andi Naufal Zaki (*5027241059*)







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
- Terus memantau proses milik '<user>'
- Menyimpan hasil pemantauan ke dalam 'file log', bukan hanya tampil di layar

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
  - Menyimpan dan membaca 'PID' dari 'file .pid' (opsional, agar proses bisa dihentikan dengan tepat)
  - Pengecekan apakah proses masih berjalan
- Pengelolaan File dan Log
  - Jika script menyimpan PID ke dalam file (misalnya 'debugmon_<user>.pid'), maka 'stop' dapat membaca file tersebut dan 'kill' berdasarkan isinya



### d)Menggagalkan proses user yang sedang berjalan

![image](https://github.com/user-attachments/assets/2135303d-455d-434b-a7b1-6c1db43c00fc)



### e)Mengizinkan user untuk kembali menjalankan proses

![image](https://github.com/user-attachments/assets/61da9848-1805-4fe9-aa2f-b9bfd73c08c5)

### f)Mencatat ke dalam file log

![image](https://github.com/user-attachments/assets/1e896d4e-fe31-4767-9da8-1b9226f7dd3b)



