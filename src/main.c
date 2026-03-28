#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define PORT       8080
#define ARRAY_SIZE 10
#define VALUE_MIN  (-100)
#define VALUE_MAX  100
#define BACKLOG    128
#define BUF_SIZE   8192

static long long g_requests_total     = 0;
static long long g_computations_total = 0;
static int       g_last_max_value     = 0;

static int max_in_array(const int *arr, size_t len)
{
    size_t i;
    int    m;
    if (!arr || len == 0)
        return INT_MIN;
    m = arr[0];
    for (i = 1; i < len; ++i)
        if (arr[i] > m)
            m = arr[i];
    return m;
}

static void do_compute(char *out, size_t sz)
{
    int    arr[ARRAY_SIZE];
    int    mx;
    size_t i;
    size_t pos   = 0;
    int    range = VALUE_MAX - VALUE_MIN + 1;

    srand((unsigned int)time(NULL));

    for (i = 0; i < ARRAY_SIZE; ++i)
        arr[i] = VALUE_MIN + (rand() % range);

    pos += (size_t)snprintf(out + pos, sz - pos, "Array:");
    for (i = 0; i < ARRAY_SIZE && pos < sz - 1; ++i)
        pos += (size_t)snprintf(out + pos, sz - pos, " %d", arr[i]);
    if (pos < sz - 1)
        pos += (size_t)snprintf(out + pos, sz - pos, "\n");

    mx                    = max_in_array(arr, ARRAY_SIZE);
    g_last_max_value      = mx;
    g_computations_total += 1;

    if (pos < sz - 1)
        snprintf(out + pos, sz - pos, "Max: %d\n", mx);
}

static void send_response(int fd, int code, const char *ctype,
                          const char *body, size_t blen)
{
    char    hdr[512];
    int     hlen;
    ssize_t w;

    hlen = snprintf(hdr, sizeof(hdr),
                    "HTTP/1.1 %d OK\r\n"
                    "Content-Type: %s\r\n"
                    "Content-Length: %zu\r\n"
                    "Connection: close\r\n\r\n",
                    code, ctype, blen);
    w = write(fd, hdr, (size_t)hlen);
    if (w >= 0) {
        w = write(fd, body, blen);
    }
    (void)w;
}

static void handle_conn(int fd)
{
    char    req[BUF_SIZE];
    ssize_t n;
    char    method[8];
    char    path[256];
    char    body[4096];
    size_t  blen;

    n = read(fd, req, sizeof(req) - 1);
    if (n <= 0)
        return;
    req[n] = '\0';
    g_requests_total += 1;

    method[0] = '\0';
    path[0]   = '\0';
    sscanf(req, "%7s %255s", method, path);

    if (strcmp(path, "/metrics") == 0) {
        blen = (size_t)snprintf(body, sizeof(body),
            "# HELP maxint_requests_total Total HTTP requests received\n"
            "# TYPE maxint_requests_total counter\n"
            "maxint_requests_total %lld\n"
            "# HELP maxint_computations_total Total max computations performed\n"
            "# TYPE maxint_computations_total counter\n"
            "maxint_computations_total %lld\n"
            "# HELP maxint_last_max_value Last computed maximum value\n"
            "# TYPE maxint_last_max_value gauge\n"
            "maxint_last_max_value %d\n"
            "# HELP maxint_up Service health (1 = up)\n"
            "# TYPE maxint_up gauge\n"
            "maxint_up 1\n",
            g_requests_total, g_computations_total, g_last_max_value);
        send_response(fd, 200,
                      "text/plain; version=0.0.4; charset=utf-8",
                      body, blen);
    } else if (strcmp(path, "/healthz") == 0) {
        send_response(fd, 200, "text/plain", "OK\n", 3);
    } else if (strcmp(path, "/") == 0 || strcmp(path, "/compute") == 0) {
        do_compute(body, sizeof(body));
        blen = strlen(body);
        send_response(fd, 200, "text/plain", body, blen);
    } else {
        send_response(fd, 404, "text/plain", "404 Not Found\n", 14);
    }
}

int main(void)
{
    int                srv;
    int                opt;
    int                client;
    struct sockaddr_in addr;

    signal(SIGPIPE, SIG_IGN);

    srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) {
        perror("socket");
        return 1;
    }

    opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);

    if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    if (listen(srv, BACKLOG) < 0) {
        perror("listen");
        return 1;
    }

    printf("maxint server listening on :%d\n", PORT);
    printf("  GET /         - compute max of random array\n");
    printf("  GET /compute  - compute max of random array\n");
    printf("  GET /metrics  - Prometheus metrics\n");
    printf("  GET /healthz  - health check\n");
    fflush(stdout);

    for (;;) {
        client = accept(srv, NULL, NULL);
        if (client < 0) {
            if (errno == EINTR)
                continue;
            perror("accept");
            break;
        }
        handle_conn(client);
        close(client);
    }

    close(srv);
    return 0;
}
