# C-Allocator — Custom Dynamic Memory Manager

![C11](https://img.shields.io/badge/C-11-blue.svg?style=flat-square)
![Tests](https://img.shields.io/badge/Tests-86%2F86%20Passed-brightgreen.svg?style=flat-square)
![Leaks](https://img.shields.io/badge/Memory%20Leaks-0-brightgreen.svg?style=flat-square)
![AI-Free Code](https://img.shields.io/badge/Logic-100%25%20Handwritten%20(No%20AI)-orange.svg?style=flat-square)
![License](https://img.shields.io/badge/License-MIT-lightgrey.svg?style=flat-square)

A custom, low-level heap memory allocator implemented in C11. This project replicates core dynamic memory allocation routines (`malloc`, `calloc`, `realloc`, `free`) over a simulated heap managed via `sbrk()`, featuring chunk metadata management, canary-based fence integrity checks, and runtime pointer validation.

---

## 📌 Origin & Philosophy

- **🎓 Revisited Post-Graduation:** Originally tackled as a challenging academic assignment during computer science studies, this project was revisited after graduating to refine pointer arithmetic, eliminate edge cases, and deepen low-level systems programming fundamentals.
- **🧠 Zero-AI Implementation:** The goal was to build the entire allocator logic **100% by hand without AI assistance** (the only exception being the generation of this README). All memory structures, pointer manipulation, chunk traversal, and validation algorithms are entirely human-crafted.

---

## 🧱 Memory Architecture & Block Layout

The allocator tracks memory using an explicit doubly-linked list of active chunks inside a contiguous heap space requested from the operating system via `custom_sbrk()`.

Each allocated block is structured with control metadata and protective canaries:

```text
+-------------------------------------------------------------------------------------------+
|    Control Block      |  Front Fence (Canary)  |     User Payload     |  Back Fence (Canary)  |
| struct memory_chunk_t |        8 bytes         |       N bytes        |        8 bytes        |
| (prev, next, sz, sum) |       "########"       | (Returned to caller) |       "########"      |
+-------------------------------------------------------------------------------------------+
^                       ^                        ^
|                       |                        |
chunk_start             fence_start              chunk_to_address(chunk)
```

### Key Defensive Mechanisms:
1. **Canary Fences (`#`)**: An 8-byte guard pattern (`########`) precedes and follows the user payload. Writing out of bounds (buffer overflow or underflow) alters these bytes and is detected during validation.
2. **Control Block Integrity Checksum (`validation_value`)**: Each chunk maintains a validation sum computed from its internal pointers and size (`prev + next + size`). If an overwrite corrupts chunk headers, `heap_validate()` flags heap corruption.
3. **Pointer Inspection**: The allocator can classify arbitrary pointers into valid user addresses, control blocks, guard fences, internal payload offsets, or unallocated space.

---

## 🚀 API Overview

```c
#include "heap.h"

// Lifecycle
int    heap_setup(void);
void   heap_clean(void);

// Standard allocation routines
void*  heap_malloc(size_t size);
void*  heap_calloc(size_t number, size_t size);
void*  heap_realloc(void *memblock, size_t size);
void   heap_free(void *memblock);

// Diagnostics & Integrity
int                  heap_validate(void);
size_t               heap_get_largest_used_block_size(void);
enum pointer_type_t get_pointer_type(const void *pointer);
```

### Pointer Classification (`get_pointer_type`):
- `pointer_valid` — points to the exact beginning of user payload.
- `pointer_inside_data_block` — points inside active payload memory.
- `pointer_inside_fences` — points into guard canary bytes.
- `pointer_control_block` — points into chunk header metadata.
- `pointer_unallocated` — points to unallocated / free heap space.
- `pointer_null` — null pointer passed.

---

## 🧪 Testing & Verification

The implementation is verified with an automated test suite containing **86 thorough unit tests** covering:
- Consecutive block allocations & heap expansions.
- In-place `realloc` growth vs reallocation and relocation.
- Hole filling, chunk unlinking, and boundary reuse.
- Canary corruption detection on single-byte overflows and underflows.
- Control block checksum tampering detection.
- Zero-leak validation with `RLDebug`.

### Test Results
```text
========================================================
Testy dostępne:   86 (AVAIL)
      Wykonane:   86 (DONE)
Testy poprawne:   86 (PASSED)
Testy niepoprawne:    0 (FAILED)
  Wycieki zasobów:    0 (LEAKS)
========================================================
Wszystkie bloki pamięci zostały pomyślnie zwolnione - brak wycieków.
Nie wykryto uszkodzenia sterty.
```

---

## 📁 Project Structure

```text
.
├── heap.c              # Core allocator implementation
├── heap.h              # Public allocator API
├── main.c              # Main entry point & demo
├── makefile            # Build automation & test runner
├── CMakeLists.txt      # CMake build configuration
├── README.md           # Documentation
└── tests/              # Test suite & unit test harness
    ├── unit_test_v2.c  # 86 automated unit tests
    ├── memmanager.c    # Heap boundary & sbrk emulation
    ├── rdebug.c / .h   # Memory leak & fence verification
    └── ...
```

---

## 🛠️ Building & Running

### Requirements
- GCC / Clang with C11 support
- Make or CMake (>= 3.17)
- Linux / WSL

### Using Make

```bash
# Build the project
make build

# Run full unit test suite (86 tests)
make run_unit_tests

# Run main program
make run_main

# Clean build artifacts
make clean
```

### Using CMake

```bash
cmake -B build
cmake --build build
./build/project1 1    # Run unit tests
```

---

## 📄 License

This project is licensed under the MIT License.
