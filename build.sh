#!/bin/bash
set -e

# Warna untuk output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Menyiapkan Lingkungan Build untuk LeviLaunchroid ===${NC}"

# 1. Mengecek dan Menginstall dependensi (cmake, ninja, wget, unzip)
echo -e "${GREEN}[1/5] Memeriksa dependensi (cmake, ninja, pwsh, unzip)...${NC}"
if ! command -v cmake &> /dev/null || ! command -v ninja &> /dev/null || ! command -v wget &> /dev/null || ! command -v unzip &> /dev/null; then
    echo "Beberapa paket dasar (cmake, ninja, wget, unzip) belum terinstall. Menginstall..."
    sudo pacman -S --needed cmake ninja wget unzip
fi

if ! command -v pwsh &> /dev/null; then
    echo "PowerShell (pwsh) dibutuhkan oleh skrip packaging resmi Levi."
    echo "Menginstall powershell-bin dari AUR (menggunakan yay)..."
    if command -v yay &> /dev/null; then
        yay -S --needed powershell-bin
    elif command -v paru &> /dev/null; then
        paru -S --needed powershell-bin
    else
        echo "AUR helper (yay/paru) tidak ditemukan! Tolong install 'powershell-bin' secara manual melalui AUR."
        exit 1
    fi
fi

# 2. NDK 28.2.13676358 (WAJIB - versi yang dibutuhkan template LeviLauncher)
# NDK r26d TIDAK KOMPATIBEL karena ABI libc++ berbeda, akan menyebabkan crash!
NDK_FULL_VERSION="28.2.13676358"
NDK_SHORT="r28b"
NDK_DIR="$HOME/android-ndk-${NDK_SHORT}"
NDK_ZIP_URL="https://dl.google.com/android/repository/android-ndk-${NDK_SHORT}-linux.zip"

if [ ! -d "$NDK_DIR" ]; then
    echo -e "${GREEN}[2/5] Mengunduh Android NDK ${NDK_SHORT} (sekitar 700MB, mohon tunggu)...${NC}"
    echo -e "${YELLOW}PENTING: NDK versi ini WAJIB untuk kompatibilitas dengan LeviLauncher.${NC}"
    wget -c "$NDK_ZIP_URL" -O /tmp/ndk.zip
    echo -e "${GREEN}Mengekstrak NDK ke $HOME...${NC}"
    unzip -q /tmp/ndk.zip -d "$HOME"
    rm /tmp/ndk.zip
    echo -e "${GREEN}NDK ${NDK_SHORT} berhasil diinstall di $NDK_DIR${NC}"
else
    echo -e "${GREEN}[2/5] Android NDK ${NDK_SHORT} sudah ditemukan di $NDK_DIR.${NC}"
fi

# 3. Verifikasi versi NDK yang ada
echo -e "${GREEN}[3/5] Verifikasi NDK...${NC}"
if [ ! -f "$NDK_DIR/source.properties" ]; then
    echo "ERROR: NDK tidak valid di $NDK_DIR"
    exit 1
fi
ACTUAL_VERSION=$(grep "Pkg.Revision" "$NDK_DIR/source.properties" | cut -d= -f2 | tr -d ' ')
echo "    NDK Version: $ACTUAL_VERSION"

# 4. Mengatur Environment Variable
export ANDROID_NDK_HOME="$NDK_DIR"
export ANDROID_HOME="$NDK_DIR"

# 5. Menjalankan Skrip Build Bawaan (Membuat file Mod)
echo -e "${GREEN}[4/5] Menjalankan proses kompilasi kode C++ menjadi .so dan .levipack...${NC}"
pwsh ./scripts/package.ps1 -Abi arm64-v8a

echo -e "${GREEN}[5/5] Selesai! File mod kamu (berakhiran .levipack) akan ada di folder build-arm64-v8a/${NC}"
