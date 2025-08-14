# Kryptor

An in-progress C++ encryption library.  
Today: hand-rolled **RSA** built on **GMP** for big-integer math.  
Next up: **AES** (with authenticated modes) and hybrid RSA+AES.

> ⚠️ **Security status: experimental**  
> Current RSA usage is **raw / textbook** (no OAEP/PSS), and file encryption converts entire files directly to big integers. Do **not** use in production or to protect real secrets yet. See [Security Notes](#security-notes).

---

## Features (WIP)

- ✅ RSA key generation (p, q, n, e, d, CRT params) with GMP
- ✅ RSA encrypt/decrypt (textbook RSA, `m^e mod n`, CRT optimize for decrypt)
- ✅ Simple file I/O helpers (read/write strings and big integers)
- 🛠️ AES module (in progress)
- 🧪 Minimal logging and debug prints to inspect internals
- 📦 Planned: OAEP, PSS, hybrid RSA+AES (RSA-KEM), key serialization, CLI

---

## Getting Started

### Prerequisites
- C++17 or newer
- [GMP / GMPXX](https://gmplib.org/) installed and discoverable by your compiler
