FROM ubuntu:22.04
COPY maxint.deb /tmp/
RUN apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates && \
    dpkg -i /tmp/maxint.deb || apt-get install -y -f && \
    rm -rf /var/lib/apt/lists/* /tmp/maxint.deb
ENTRYPOINT ["/usr/bin/maxint"]
