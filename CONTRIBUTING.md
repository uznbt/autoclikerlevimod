# Panduan Kontribusi (Contributing Guidelines)

Terima kasih telah tertarik untuk berkontribusi pada **LeviLauncher Native Autoclicker Mod**! Kontribusi dari komunitas sangat kami hargai.

---

## Cara Berkontribusi

### 1. Melaporkan Bug (Bug Reports)
Jika Anda menemukan bug, error, atau masalah saat menggunakan mod:
1. Buka [GitHub Issues](https://github.com/uznbt/autoclikerlevimod/issues).
2. Periksa apakah isu serupa sudah pernah dilaporkan sebelumnya.
3. Jika belum, buat issue baru dengan menyertakan:
   - Versi LeviLauncher / LeviLaunchroid yang digunakan.
   - Versi Android & Model HP.
   - Langkah-langkah untuk mereproduksi bug.
   - Log error (jika ada, misal tombstone/logcat).

### 2. Mengajukan Fitur Baru (Feature Requests)
Punya ide menarik untuk meningkatkan mod ini?
- Silakan buka **Issue** baru dengan label **Feature Request**.
- Jelaskan secara detail fitur yang Anda inginkan dan mengapa fitur tersebut bermanfaat.

### 3. Mengirimkan Kode (Pull Requests / PR)
Jika Anda ingin memperbaiki bug atau membuat fitur sendiri:

1. **Fork** repositori [uznbt/autoclikerlevimod](https://github.com/uznbt/autoclikerlevimod).
2. Clone hasil fork Anda ke komputer lokal:
   ```bash
   git clone https://github.com/USERNAME/autoclikerlevimod.git
   cd autoclikerlevimod
   ```
3. Buat branch baru untuk pengerjaan Anda:
   ```bash
   git checkout -b feature/nama-fitur-anda
   # atau untuk bugfix:
   git checkout -b fix/deskripsi-bug
   ```
4. Lakukan perubahan kode dan uji coba kompilasi lokal:
   ```bash
   ./build.sh
   ```
5. Commit perubahan dengan pesan commit yang jelas:
   ```bash
   git commit -m "feat: menambahkan dukungan mode X"
   ```
6. Push branch ke GitHub Anda:
   ```bash
   git push origin feature/nama-fitur-anda
   ```
7. Buka halaman **Pull Request** di repositori utama kami dan jelaskan perubahan yang Anda buat.

---

## Standar Kode & Persyaratan Build

- **Bahasa**: C++17 / C++20
- **Compiler / Toolchain**: Android NDK **r28b** (`28.2.13676358`)
- **SDK**: Preloader Android (`preloader-android`)
- **Gaya Kode**: Ikuti format `.clang-format` yang sudah ada di repositori.
- **JNI Safety**: Pastikan semua Local Reference JNI dihapus (`DeleteLocalRef`) dan penanganan exception `ExceptionCheck()` dipanggil dengan benar untuk mencegah memory leak atau crash pada thread Android.

---

## Lisensi

Dengan berkontribusi pada repositori ini, Anda menyetujui bahwa kontribusi Anda akan dilisensikan di bawah [Lisensi MIT](LICENSE).
