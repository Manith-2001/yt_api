# ============================================================
# Stage 1: Builder — compile yt-api
# ============================================================
FROM alpine:3.21 AS builder

RUN apk add --no-cache gcc musl-dev make curl python3 py3-pip && pip install --break-system-packages yt-dlp

WORKDIR /build

COPY . .

RUN make yt-api CFLAGS="-static"

# ============================================================
# Stage 2: Runtime — Python + yt-dlp + compiled binary
# ============================================================
FROM python:3.12-alpine

RUN pip install --no-cache-dir yt-dlp

WORKDIR /app

COPY --from=builder /build/yt-api .

EXPOSE 8000 8443

ENTRYPOINT ["./yt-api"]
