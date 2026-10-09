<div align="center">

# 🗃️ new_ls

**A modular UNIX file-listing utility in C**

![Language](https://img.shields.io/badge/C-C11-00599C?logo=c&logoColor=white) ![Target](https://img.shields.io/badge/Target-NetBSD-EAB92D) ![Build](https://img.shields.io/badge/Build-BSD%20make-blue) ![Options](https://img.shields.io/badge/Flags-19-brightgreen)

</div>

## Overview

`new_ls` is a from-scratch, educational implementation of the subset of the NetBSD `ls(1)` manual distributed for the UNIX systems programming midterm. It never launches the system `ls` to produce its output. Source code is separated into six implementation modules and a shared interface.

## Requirements

- NetBSD with development tools (`cc`, `make`), or another compatible POSIX environment.
- No third-party runtime libraries are required.
- Optional Git to clone the repository.

## Build and test

```sh
make
make test
./new_ls -la
```

## Optional installation (run without `./`)

Run `make install` with sufficient permissions; the default location is `/usr/local/bin/new_ls`.

```sh
make
su
make install
exit
new_ls -la
```

If `new_ls: not found`, verify `/usr/local/bin` is on `PATH`. For Bash, run `export PATH="/usr/local/bin:$PATH"` in the current shell. To install without root into a personal prefix, run `make PREFIX="$HOME/.local" install` and put `$HOME/.local/bin` on `PATH`.

To remove the installed binary, use `make uninstall` with the same permissions and prefix. **The system `/bin/ls` is never overwritten.**

## Usage

```text
new_ls [-AacdFfhiklnqRrSstuw] [file ...]
```

| Option | Behavior |
|---|---|
| `-a`, `-A` | Show dotfiles; `-A` excludes `.` and `..` |
| `-c`, `-u` | Select status-change or access time |
| `-d`, `-R` | List directories themselves or recurse |
| `-F` | Append type indicators |
| `-f`, `-r`, `-S`, `-t` | Unsorted, reverse, size or time order |
| `-h`, `-k`, `-s` | Human sizes, 1-KiB block units, blocks |
| `-i` | Show inode number |
| `-l`, `-n` | Long listing, numeric users/groups |
| `-q`, `-w` | Replace nonprintable characters or print names raw |

Examples:

```sh
./new_ls -A ~
./new_ls -la /etc
./new_ls -R .
./new_ls -St /tmp
./new_ls -- -filename
```

## Design

| Module | Responsibility |
|---|---|
| `cli_parser.c` | `getopt` parsing and option precedence |
| `file_utils.c` | Path joins and memory-safe dynamic catalog |
| `scanner.c` | Directory reads, operands, recursive traversal |
| `ordering.c` | Sorting by filename, size, selected timestamp |
| `formatter.c` | Long format, permissions, blocks, name escaping |
| `main.c` | Top-level control and exit status |
| `include/new_ls.h` | Shared models and interfaces |

Important system calls/APIs: `opendir`, `readdir`, `closedir`, `lstat`, `stat`, `readlink`, `getpwuid`, `getgrgid`, `localtime_r`, `strftime`.

## Validation and limitations

Run `make test`, then compare with the **NetBSD** implementation using identical paths. The program prints one entry per line, in keeping with the provided abbreviated manual; interactive column layout is intentionally not implemented. Exact spacing, block-size environment conventions, locale/multibyte rules, and whiteout handling can differ from NetBSD's production `ls`. The smoke-test suite is not a substitute for comprehensive cross-platform verification.

When publishing this as coursework, the student should run the tests in their own NetBSD VM, understand the implementation, and document their own observations and contributions.
