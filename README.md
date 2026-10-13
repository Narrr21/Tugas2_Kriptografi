# Tugas 2 Kriptografi - Cipher Blok 128-bit

Implementasi algoritma kriptografi cipher blok simetris 128-bit berbasis Jaringan Feistel (Feistel Network) dengan pengujian analisis keamanan dan antarmuka web interaktif.

> **API Documentation:** [https://narrr21.github.io/Tugas2_Kriptografi/](https://narrr21.github.io/Tugas2_Kriptografi/)

---

## 1. Deskripsi Program 
  Program ini mengimplementasikan algoritma enkripsi dan dekripsi simetris menggunakan struktur Jaringan Feistel 16 round dengan ukuran blok 128-bit dan panjang kunci 128-bit. Fitur utama algoritma mencakup:
  - **Key-Dependent Dynamic S-Box:** Pembentukan tabel substitusi dinamis 8-bit berdasarkan kunci utama menggunakan algoritma Fisher-Yates permutation.
  - **Key Schedule / Key Expansion:** Derivasi 16 subkunci putaran (round keys) 64-bit menggunakan operasi rotasi bit dan konstanta matematika (Golden Ratio & Digit Pi).
  - **Round Function:** Kombinasi operasi penambahan modular modulo $2^{64}$, transposisi bit, substitusi S-Box dinamis, dan difusi non-linear.
  - **Mode Operasi:** Mendukung 5 mode operasi standar, yaitu:
    - Electronic Codebook (ECB)
    - Cipher Block Chaining (CBC)
    - Cipher Feedback (CFB)
    - Output Feedback (OFB)
    - Counter (CTR)
  - **Padding:** Menggunakan skema padding PKCS#7 untuk mode berbasis blok (ECB & CBC).
  - **Analisis Kriptografi:** Dilengkapi pengujian empiris meliputi *Avalanche Effect*, analisis diferensial, analisis linier, uji entropi, uji histogram, dan validasi mode operasi.
  - **Antarmuka Pengguna (GUI):** Antarmuka web berbasis Streamlit untuk enkripsi/dekripsi teks dan file secara interaktif.

---

## 2. Tech Stack

- **C++ (Standar C++20):** Implementasi inti algoritma cipher blok, fungsi padding, ekspansi kunci, shared library C API, dan suite pengujian performa/keamanan.
- **Python 3:** Wrapper API menggunakan modul `ctypes` untuk menjembatani shared library C++ dengan antarmuka pengguna.
- **Streamlit:** Framework antarmuka pengguna (GUI) berbasis web untuk demonstrasi enkripsi dan dekripsi interaktif.
- **GCC / G++:** Kompiler C++ untuk memproses source code C++20 ke biner executable dan shared library.
- **Shell / PowerShell Scripting:** Skrip otomasi kompilasi dan eksekusi lintas platform (Linux dan Windows).

---

## 3. Dependensi

### Kebutuhan Sistem & Kompiler
- **C++ Compiler:** Kompiler dengan dukungan standar C++20 (misal: GCC/G++ $\ge$ 10, Clang $\ge$ 12, atau MSVC 2019/2022).
- **Python:** Python versi 3.8 atau lebih baru (disarankan $\ge$ 3.10).

### Dependensi Pustaka Python
Dependensi pustaka tercantum pada file `src/gui/requirements.txt`:
- `streamlit>=1.50`

---

## 4. Tata Cara Menjalankan Program

### A. Menjalankan Suite Pengujian (CLI)

Suite pengujian mencakup verifikasi avalanche effect, differential cryptanalysis, linear cryptanalysis, entropi Shannon, analisis histogram, dan uji seluruh mode operasi.

#### Pada Linux / macOS:
1. Pastikan script memiliki izin eksekusi:
   ```bash
   chmod +x run.sh
   ```
2. Jalankan skrip pengujian:
   ```bash
   ./run.sh
   ```

#### Pada Windows:
- Menggunakan **PowerShell**:
  ```powershell
  .\run.ps1
  ```
- Menggunakan **Command Prompt (CMD)**:
  ```cmd
  run.bat
  ```

Hasil kompilasi pengujian akan ditempatkan pada folder `bin/` dan dieksekusi secara berurutan.

---

### B. Menjalankan Antarmuka Grafis (GUI Streamlit)

Aplikasi GUI memungkinkan enkripsi/dekripsi teks maupun file secara langsung dengan memilih mode operasi, kunci hex, dan IV.

#### 1. Persiapan Environment Python
Disarankan menggunakan virtual environment:
```bash
# Membuat virtual environment (opsional)
python3 -m venv .venv

# Aktivasi virtual environment:
# Linux / macOS:
source .venv/bin/activate
# Windows PowerShell:
.\.venv\Scripts\Activate.ps1
# Windows CMD:
.\.venv\Scripts\activate.bat

# Install dependensi
pip install -r src/gui/requirements.txt
```

#### 2. Menjalankan GUI

- **Linux / macOS:**
  Jalankan skrip otomasi yang akan mengompilasi shared library `cipher.dll` / `.so` jika belum ada, kemudian meluncurkan Streamlit:
  ```bash
  chmod +x src/gui/run.sh src/gui/build.sh
  ./src/gui/run.sh
  ```
  *Atau secara manual:*
  ```bash
  ./src/gui/build.sh
  streamlit run src/gui/app.py
  ```

- **Windows:**
  - Melalui **PowerShell**:
    ```powershell
    .\src\gui\run.ps1
    ```
  - Melalui **Command Prompt (CMD)**:
    ```cmd
    src\gui\run.bat
    ```
Aplikasi web otomatis terbuka pada browser default (umumnya di `http://localhost:8501`).

---

## 5. Dokumentasi API

Dokumentasi API lengkap dapat diakses secara daring pada:  
[https://narrr21.github.io/Tugas2_Kriptografi/](https://narrr21.github.io/Tugas2_Kriptografi/)