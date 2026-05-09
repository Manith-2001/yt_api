# yt-api

A lightweight C HTTP/HTTPS server built on [Mongoose v7.21](https://mongoose.ws/) with `yt-dlp` integration.

## Overview

**yt-api** is a C-based HTTP/HTTPS server designed as a REST API wrapper around [`yt-dlp`](https://github.com/yt-dlp/yt-dlp). It verifies that `yt-dlp` is available on the host system (both at build time and at runtime) and exposes API endpoints for clients to interact with it over HTTP.

## Tech Stack

| Layer | Technology |
|---|---|
| Language | C (C99/C11) |
| HTTP Library | [Mongoose v7.21](https://mongoose.ws/) (single-header, embedded) |
| Build System | GNU Make |
| TLS | Mongoose built-in TLS (no OpenSSL required) |
| External Dependency | `yt-dlp` (runtime) |
| Platforms | macOS (ARM64), Linux, Windows (MinGW) |

## Directory Structure

```
.
├── Makefile                   # Build system
├── main.c                     # Entry point & event loop
├── router.c                   # URI dispatch table
├── router.h                   # Route types & dispatch declaration
├── handlers/
│   ├── download_handler.c     # POST /api/download handler
│   └── download_handler.h     # Handler declaration
├── utils/
│   ├── ANSI_colors.h          # ANSI terminal color/style macros
│   ├── utils.h                # check_yt_dlp() declaration
│   └── utils.c                # yt-dlp availability check
└── mongoose/
    ├── mongoose.h             # Mongoose header (v7.21)
    └── mongoose.c             # Mongoose implementation
```

## Prerequisites

- C compiler (gcc, clang, or mingw on Windows)
- GNU Make
- [`yt-dlp`](https://github.com/yt-dlp/yt-dlp#installation) installed and available in `PATH`

## Build

```bash
make          # Builds yt-api and runs it
```

To build without running:

```bash
make yt-api
```

### Windows (MinGW)

```batch
make CC=gcc
```

### Clean

```bash
make clean
```

## Usage

```bash
./yt-api
```

The server starts listening on:

| Protocol | Address |
|---|---|
| HTTP | `http://0.0.0.0:8000` |
| HTTPS | `https://0.0.0.0:8443` |

## API Endpoints

| Route | Method | Description |
|---|---|---|
| `/api/download` | POST | Accepts JSON `{"link": "<url>"}`, returns `{"received": "<url>"}` |

### Example Requests

```bash
# Download request
curl -X POST http://localhost:8000/api/download \
  -H "Content-Type: application/json" \
  -d '{"link":"https://www.youtube.com/watch?v=..."}'

# Missing link field (returns 400)
curl -X POST http://localhost:8000/api/download \
  -H "Content-Type: application/json" \
  -d '{"foo":"bar"}'

# Wrong method (returns 405)
curl http://localhost:8000/api/download

# HTTPS (self-signed cert — use -k)
curl -k -X POST https://localhost:8443/api/download \
  -H "Content-Type: application/json" \
  -d '{"link":"https://youtu.be/..."}'
```

## Configuration

| Variable | Purpose | Default |
|---|---|---|
| `PROG` | Binary name | `yt-api` (`yt-api.exe` on Windows) |
| `CC` | C compiler | `cc` |
| `CFLAGS_EXTRA` | Additional Mongoose build flags | `-DMG_TLS=MG_TLS_BUILTIN` |
| `OUT` | Output file argument | `-o $(PROG)` |
| `ARGS` | Arguments passed when running via `make` | *(empty)* |

Listen addresses and TLS certificates are hardcoded in `main.c`. The server uses a self-signed EC (P-256) certificate for HTTPS.

## Architecture

- **Event-driven:** Mongoose uses a single-threaded callback-based event loop — no threads or blocking I/O.
- **Router dispatch:** HTTP requests are dispatched to handlers via a route table in `router.c`, using `mg_match` for URI and method matching. Unmatched routes return 404; method mismatches return 405.
- **Belt-and-suspenders check:** `yt-dlp` availability is verified both at build time (Makefile) and at runtime on startup.
- **Portable:** Mongoose ships as a single `.h` + `.c` pair with no external dependencies beyond libc and OS socket APIs.

## License

- Project source: See repository
- Mongoose: GPLv2 or commercial (Cesanta Software)
