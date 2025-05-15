#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <iconv.h>

#define BUF 8192

void error(const char *msg) { perror(msg); exit(1); }

char *get_charset(const char *hdrs) {
    char *p = strcasestr(hdrs, "charset=");
    if (!p) return "UTF-8";
    p += 8;
    static char cs[32];
    sscanf(p, "%31[^;\r\n]", cs);
    return cs;
}

void convert(const char *from, const char *in, size_t inlen) {
    iconv_t cd = iconv_open("UTF-8", from);
    if (cd == (iconv_t)-1) { fwrite(in, 1, inlen, stdout); return; }
    char out[BUF*2], *outp = out;
    size_t outlen = sizeof(out);
    char *inp = (char *)in;
    iconv(cd, &inp, &inlen, &outp, &outlen);
    fwrite(out, 1, sizeof(out)-outlen, stdout);
    iconv_close(cd);
}

int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "Usage: %s <url>\n", argv[0]); return 1; }

    char host[256], path[1024];
    sscanf(argv[1], "http://%255[^/]%1023s", host, path);
    if (path[0] == '\0') strcpy(path, "/");

    struct hostent *he = gethostbyname(host);
    if (!he) error("host");
    int s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sa = { .sin_family=AF_INET, .sin_port=htons(80) };
    memcpy(&sa.sin_addr, he->h_addr, he->h_length);
    connect(s, (struct sockaddr*)&sa, sizeof sa);

    dprintf(s, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, host);

    char buf[BUF], *body;
    int n = recv(s, buf, BUF-1, 0); buf[n] = 0;

    char *hdr_end = strstr(buf, "\r\n\r\n");
    if (!hdr_end) error("no headers");
    *hdr_end = 0;
    char *charset = get_charset(buf);

    body = hdr_end + 4;
    convert(charset, body, n - (body - buf));

    while ((n = read(s, buf, BUF)) > 0)
        convert(charset, buf, n);

    close(s);
    return 0;
}
