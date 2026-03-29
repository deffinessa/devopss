# Stage 1: Build with musl for a fully static binary
FROM ubuntu:22.04 AS builder
RUN apt-get update && \
    apt-get install -y --no-install-recommends musl-tools make && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /build
COPY Makefile .
COPY src/ src/
RUN CC=musl-gcc CFLAGS="-Wall -Wextra -Werror -O2 -static" make all

# Stage 2: Runtime — scratch base (~0 MB, binary only ~117 kB)
FROM scratch
COPY --from=builder /build/maxint /maxint
EXPOSE 8080
ENTRYPOINT ["/maxint"]
