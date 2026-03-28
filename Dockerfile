# Stage 1: Build
FROM ubuntu:22.04 AS builder
RUN apt-get update && \
    apt-get install -y --no-install-recommends gcc make && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /build
COPY Makefile .
COPY src/ src/
RUN make all

# Stage 2: Runtime
FROM ubuntu:22.04
RUN apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates && \
    rm -rf /var/lib/apt/lists/*
COPY --from=builder /build/maxint /usr/bin/maxint
EXPOSE 8080
ENTRYPOINT ["/usr/bin/maxint"]
