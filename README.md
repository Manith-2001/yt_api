# yt-api

A lightweight C HTTP/HTTPS server built on [Mongoose v7.21](https://mongoose.ws/) with `yt-dlp` integration for streaming YouTube videos directly to clients.

## Overview

**yt-api** is a C-based HTTP/HTTPS server that wraps [`yt-dlp`](https://github.com/yt-dlp/yt-dlp) as a streaming video service. When a client requests a download, the server queues the job, downloads the video via `yt-dlp` to a local temp directory, and streams the binary file back to the client using HTTP chunked transfer encoding — all without buffering the full file in memory.

## Tech Stack

| Layer | Technology |
|---|---|
| Language | C (C99/C11) |
| HTTP Library | [Mongoose v7.21](https://mongoose.ws/) (single-header, embedded) |
| Build System | GNU Make |
| TLS | MG_TLS_BUILTIN (configured, HTTPS not yet bound) |
| Threading | POSIX Threads (detached per-request + background worker) |
| IPC | SysV Message Queues |
| External Dependency | `yt-dlp` (runtime) |
| Platforms | macOS (ARM64), Linux, Windows (MinGW) |

## Directory Structure

```
.
├── Makefile                      # Build system
├── main.c                        # Entry point & Mongoose event loop
├── router.c / router.h           # URI dispatch table
├── Dockerfile                    # Multi-stage Alpine Docker build
├── docker-compose.yml            # Docker Compose service definition
├── .dockerignore                 # Files excluded from Docker context
├── handlers/
│   ├── download_handler.c        # POST /api/download handler (fd steal, pthread spawn)
│   └── download_handler.h
├── threads/
│   ├── api_thread.c / .h         # Per-request detached pthread (streams response)
│   ├── downloader_thread.c / .h   # Background download worker pthread
│   └── job_queue.c / .h          # SysV message queue wrappers
├── utils/
│   ├── yt_downloader.c / .h      # yt-dlp wrapper (yt_download)
│   ├── ANSI_colors.h             # Terminal color/style macros
│   └── utils.c / .h              # check_yt_dlp() availability check
└── mongoose/
    ├── mongoose.h                 # Mongoose header (v7.21)
    └── mongoose.c                 # Mongoose implementation
```

## Prerequisites

- C compiler (gcc, clang, or mingw on Windows)
- GNU Make
- [`yt-dlp`](https://github.com/yt-dlp/yt-dlp#installation) installed and available in `PATH`
- `curl` (for testing)

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

### Docker

```bash
docker compose up --build
```

Builds a multi-stage Alpine Docker image (builder stage compiles statically, runtime stage provides Python + `yt-dlp`). Exposes ports 8000 and 8443.

## Usage

```bash
./yt-api
```

The server starts listening on:

| Protocol | Address |
|---|---|
| HTTP | `http://0.0.0.0:8000` |
| HTTPS | `https://0.0.0.0:8443` (configured, not yet bound) |

## API Endpoints

| Route | Method | Description |
|---|---|---|
| `/api/download` | POST | Accepts JSON `{"link": "<url>"}`, downloads video via yt-dlp, streams binary data back using chunked transfer encoding |

### Example Requests

```bash
# Stream a video (download + stream in real-time)
curl -N -X POST http://localhost:8000/api/download \
  -H "Content-Type: application/json" \
  -d '{"link":"https://www.youtube.com/watch?v=dQw4w9WgXcQ"}' \
  -o video.webm

# Missing link field (returns 400)
curl -X POST http://localhost:8000/api/download \
  -H "Content-Type: application/json" \
  -d '{"foo":"bar"}'

# Wrong method (returns 405)
curl http://localhost:8000/api/download
```

> **Note:** Use `-N` with `curl` to disable buffering and see streaming progress. The response is raw binary data (video/webm by default), not JSON.

## Configuration

| Variable | Purpose | Default |
|---|---|---|
| `PROG` | Binary name | `yt-api` (`yt-api.exe` on Windows) |
| `CC` | C compiler | `cc` |
| `CFLAGS_EXTRA` | Additional Mongoose build flags | *(empty)* |
| `OUT` | Output file argument | `-o $(PROG)` |
| `ARGS` | Arguments passed when running via `make` | *(empty)* |

Listen address is hardcoded in `main.c`. The server initialises Mongoose with `MG_LL_DEBUG` log level for verbose event logging (`main.c:40`).

## Architecture

### Threading Model

The server uses two categories of pthreads:

1. **Per-request detached pthreads** (`api_function`): Spawned for each incoming download request. These are responsible for taking over the client connection and streaming the response. Each request gets its own thread that detaches immediately after creation — the main event loop never blocks on it.

2. **Background download worker** (`download_function`): A single persistent pthread that dequeues download jobs from the `downloadqueue` SysV message queue, invokes `yt-dlp`, and enqueues the result to the `completedqueue`.

```
Client Request
      │
      ▼
data_handler() ──steals fd──► api_function() pthread (detached)
      │                                │
      │                         enque_download(link, tid)
      │                                │
      ▼                                ▼
 mg_http_reply()              downloadqueue (SysV msg queue)
 (never called)                        │
                                        ▼
                               download_function() [worker pthread]
                                        │
                                        │ yt_download(link)
                                        ▼
                               completedqueue (SysV msg queue)
                                        │
                                        ▼
                               api_function() dequeues result
                                        │
                                        ▼
                               fopen(filepath) + stream via raw send()
```

### File Descriptor Steal Pattern

When `data_handler()` receives a request, it **steals the socket file descriptor** from the Mongoose connection struct before Mongoose can close it:

```c
void *fd = c->fd;              // Save the raw fd
c->fd = (void *)MG_INVALID_SOCKET; // Mark invalid in mongoose
c->is_closing = 1;             // Tell mongoose to close this connection struct
// mongoose will NOT close our fd — it thinks the connection is already closing
```

The detached pthread receives only the raw `int fd` and uses raw POSIX `send()` directly — it **never touches the mongoose struct** again. This avoids use-after-free since `mg_close_conn` will free the `c` pointer in the next `mg_mgr_poll` iteration.

### SysV Job Queues

Two SysV message queues coordinate between threads:

| Queue | Writer | Reader | Purpose |
|---|---|---|---|
| `downloadqueue` | `api_function` | `download_function` | Job requests (link + TID) |
| `completedqueue` | `download_function` | `api_function` | Completion results (filepath + TID) |

The `completedqueue` is a shared queue — multiple `api_function` threads may be waiting on it simultaneously. Each thread dequeues messages and re-enqueues any that don't match its own TID (busy-wait with 1-second sleep). This is a deliberate simplicity tradeoff: no per-thread condition variables, just shared-queue filtering.

### Chunked Transfer Encoding

The streaming response uses HTTP chunked transfer encoding (no Content-Length needed):

```
HTTP/1.1 200 OK
Transfer-Encoding: chunked
Content-Type: video/webm
Connection: close

<hex-size>\r\n
<binary-chunk-data>\r\n
<hex-size>\r\n
<binary-chunk-data>\r\n
...
0\r\n\r\n
```

Each chunk is sent as: hex length + `\r\n` + data + `\r\n`. The final `0\r\n\r\n` terminates the stream.

### yt-dlp Integration

`yt_download()` in `utils/yt_downloader.c` performs a two-step download:

1. **Capture filename**: `yt-dlp -P ./tmp/ -o "%(id)s.%(ext)s" --print filename` to get the output path without downloading
2. **Actual download**: `yt-dlp -S "height:720" -P ./tmp/ -o "%(id)s.%(ext)s"` to download the best 720p stream

The `-S "height:720"` flag selects the best format with a vertical resolution of at most 720 pixels. Files are saved to `./tmp/` with the naming scheme `%(id)s.%(ext)s`.

After the download completes, the worker thread sleeps for 1 second to allow the OS to flush the file, enqueues the result to `completedqueue`, then sleeps another 15 seconds (a simple cooldown between downloads). This provides a simple (if blunt) mitigation for the file-flush race condition where the OS may not have fully persisted the file before the completion message is sent.

### Router Dispatch

HTTP requests are dispatched to handlers via a route table in `router.c`, using `mg_match` for URI and method matching. Unmatched routes return 404; method mismatches return 405.

### Portable

Mongoose ships as a single `.h` + `.c` pair with no external dependencies beyond libc and OS socket APIs.

## Roadmap / TODO

- [ ] **Extension selection**: Allow clients to specify desired file extension (e.g., `webm`, `mp4`) in the JSON payload. `yt-dlp` supports `-f` and post-processing flags to control output format. Pass through as an optional `format` or `ext` field in the request body.
- [ ] **Rate throttling**: Implement per-client or global request rate limiting. Options: token bucket, sliding window, or simple per-IP cooldown via SysV shared memory or a lightweight in-memory counter map.
- [ ] **Public deployment**: Deploy on a VPS with a domain for public access. Consider Docker containerization for easy deployment.

## License

- Project source: See repository
- Mongoose: GPLv2
