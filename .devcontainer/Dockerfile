
FROM debian:13-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    gdb \
    strace \
    lsof \
    procps \
    iproute2 \
    netcat-openbsd \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*
