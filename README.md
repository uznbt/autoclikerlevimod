# LeviLauncher Native Autoclicker Mod

[![Language](https://img.shields.io/badge/Language-C%2B%2B17-00599C?style=for-the-badge&logo=cplusplus)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Android-3DDC84?style=for-the-badge&logo=android)](https://developer.android.android.com/)
[![Target](https://img.shields.io/badge/Target-LeviLauncher_1.5.25+-FF6F00?style=for-the-badge)](https://github.com/LiteLDev/LeviLauncher)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](LICENSE)
[![Release](https://img.shields.io/badge/Release-.levipack_Ready-brightgreen?style=for-the-badge)](https://github.com/uznbt/autoclikerlevimod/releases)

Autoclicker Native C++ eksklusif untuk **[LeviLauncher](https://levilauncher.levimc.org/)** / LeviLaunchroid (Minecraft Bedrock Android). Didesain khusus agar berjalan lancar, ringan, 0-lag, tanpa memerlukan aplikasi tambahan, overlay luar, ataupun akses root.

 Tinggal unduh file `.levipack` dari menu **[Releases](https://github.com/uznbt/autoclikerlevimod/releases)** di GitHub ini dan pasang ke LeviLauncher!

---

## Fitur Utama Mod

- **0-Lag Touch Injection**: Mengirim input klik langsung di dalam memori game (JNI Native). Tidak membuat game patah-patah atau merusak FPS.
- **Dynamic HUD Position**: Tombol melayang "**AC**" di layar dapat digeser (*draggable*). Klik otomatis akan tepat mengenai lokasi di mana tombol "AC" tersebut ditaruh.
- **Mode Auto-Click (PvP)**: Pukulan cepat (*spam click*) otomatis untuk PvP / menyerang mob dengan interval milidetik yang dapat disesuaikan.
- **Mode Auto-Hold (Mining / Building)**: Menahan layar secara terus-menerus tanpa lepas. Cocok untuk *mining* blok panjang, menghancurkan *obsidian*, atau *fast building*.
- **Smart Anti-Freeze (Smart Pause)**: Begitu jari asli Anda menyentuh analog jalan atau layar, autoclicker akan otomatis *pause* sejenak sehingga game tidak *freeze* atau susah dimatikan.
- **Emergency Stop**: Bisa mematikan/menyalakan mod secara instan menggunakan tombol fisik **Volume Up/Down** HP Anda.

---

## Cara Install & Menggunakan (.levipack)

1. **Download File Mod**:
   - Buka halaman [GitHub Releases](https://github.com/uznbt/autoclikerlevimod/releases).
   - Unduh file berkestensi `.levipack` (contoh: `autoclickerlevimod-0.1.0-arm64-v8a.levipack`).

2. **Pasang di LeviLauncher**:
   - Buka aplikasi **LeviLauncher / LeviLaunchroid** di Android Anda.
   - Masuk ke **Mod Manager** / Pengelola Mod.
   - Impor atau salin file `.levipack` ke folder mod LeviLauncher Anda, lalu **Aktifkan Mod**.

3. **Pengaturan Mode & Kecepatan**:
   - Buka **Mod Settings** di LeviLauncher.
   - **Mode**: Pilih antara `Auto-Click` (PvP / Spam) atau `Auto-Hold` (Mining / Tahan Tangan).
   - **Interval (ms)**: Atur kecepatan klik dalam milidetik (Rekomendasi: `30ms` ~ `100ms`).

4. **Penggunaan di Dalam Game**:
   - Masuk ke dunia Minecraft Bedrock.
   - Tombol melayang **`AC`** akan muncul. Geser tombol ke posisi yang nyaman.
   - Tekan tombol **`AC`** untuk mengaktifkan (**ON**). Tekan lagi untuk mematikan (**OFF**).
   - Anda juga dapat menekan **Tombol Volume HP** untuk menyalakan/mematikan mod kapan saja saat darurat.

---

## Hal-Hal yang Harus Dihindari (Peringatan & Tips)

1. **Jangan Mengatur Interval Terlalu Kecil (< 20ms)**:
   - Mengatur interval klik terlalu ekstrim (seperti `1ms` - `10ms`) dapat menumpuk antrean *input touch* pada sistem Android. Hal ini menyebabkan UI game terasa berat/lag dan tombol susah dipencet balik. **Gunakan interval ideal antara 30ms - 100ms** (sekitar 10 - 30 CPS).
2. **Resiko Kena Auto-Ban / Anti-Cheat Server**:
   - Jika bermain di server multiplayer (seperti Hive, Cubecraft, dll.), CPS yang terlalu tinggi dan terlalu stabil dapat terdeteksi oleh sistem Anti-Cheat server (misal: Grim, Vulcan). Gunakan interval yang wajar demi keamanan akun Anda.
3. **Hindari Menekan Tombol AC Berulang-ulang Sangat Cepat Secara Manual**:
   - Saat mod sedang menyala (**ON**), tidak perlu menekan tombol `AC` secara beruntun dengan jari asli. Gunakan **Tombol Volume HP** jika ingin mematikan mod secara mendadak saat darurat.
4. **Matikan Mod Sebelum Mengubah Mode di Setting**:
   - Disarankan untuk mematikan status autoclicker (**OFF**) terlebih dahulu sebelum berganti antara Mode *Auto-Click* dan *Auto-Hold* di Mod Settings LeviLauncher.

---

## Kontribusi

Kontribusi selalu terbuka! Jika Anda menemukan bug, ingin menambahkan fitur baru, atau meningkatkan performa mod:

1. **Fork** repositori ini.
2. Buat branch baru untuk fitur Anda (`git checkout -b feature/FiturBaru`).
3. Commit perubahan Anda (`git commit -m 'feat: menambahkan fitur baru'`).
4. Push ke branch tersebut (`git push origin feature/FiturBaru`).
5. Buat **Pull Request**.

Jika menemukan bug atau memiliki saran, silakan buka **[Issues](https://github.com/uznbt/autoclikerlevimod/issues)**.

---

## Referensi & Komunitas LeviLauncher

- **Situs Resmi LeviLauncher**: [https://levilauncher.levimc.org/](https://levilauncher.levimc.org/)
- **GitHub Resmi LeviLauncher**: [https://github.com/LiteLDev/LeviLauncher](https://github.com/LiteLDev/LeviLauncher)
- **Framework Preloader Android**: [https://github.com/LiteLDev/preloader-android](https://github.com/LiteLDev/preloader-android)

---

## Lisensi

Proyek ini dilisensikan di bawah [Lisensi MIT](LICENSE). Bebas digunakan, dimodifikasi, dan didistribusikan secara terbuka.

---

## Developer / Compiling (Opsional)

Jika Anda ingin mengkompilasi sendiri dari source code:

1. Pastikan terinstall Android NDK r28b & PowerShell (`pwsh`).
2. Jalankan skrip build:
   ```bash
   ./build.sh
   ```
3. File `.levipack` baru akan tercipta di folder `build-arm64-v8a/`.
