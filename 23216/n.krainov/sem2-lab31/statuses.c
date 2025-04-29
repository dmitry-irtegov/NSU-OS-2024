#include "statuses.h"
#include "proxy.h"
#include <string.h>

int status502(Buffer* buf) {
    buf->buffer = strdup("HTTP/1.0 502 Bad Gateway\r\nContent-Type: text/html;\r\ncharset=UTF-8\r\n\r\n\
<html>\n\
    <head>\n\
        <title>Bad Gateway<\\title>\n\
    </head>\n\
    <body>\n\
        <p>The server is unreachable at this time.</p>\n\
    </body>\n\
</html>\n");

    if (buf->buffer == NULL) {
        return 1;
    }

    buf->len = strlen("HTTP/1.0 502 Bad Gateway\r\nContent-Type: text/html;\r\ncharset=UTF-8\r\n\r\n\
<html>\n\
    <head>\n\
        <title>Bad Gateway<\\title>\n\
    </head>\n\
    <body>\n\
        <p>The server is unreachable at this time.</p>\n \
    </body>\n\
</html>\n");
    buf->count = buf->len;

    return 0;
}

int status400(Buffer* buf) {
    buf->buffer = strdup("HTTP/1.0 400 Bad Request\r\n\
Content-Type: text/html; charset=UTF-8\r\n\r\n\
<html>\n\
    <head>\n\
        <title>Bad Request<\\title>\n\
    </head>\n\
    <body>\n\
        <p>Error in request</p>\n \
    </body>\n\
</html>\n");

    buf->len = strlen("HTTP/1.0 400 Bad Request\r\n\
Content-Type: text/html; charset=UTF-8\r\n\r\n\
<html>\n\
    <head>\n\
        <title>Bad Request<\\title>\n\
    </head>\n\
    <body>\n\
        <p>Error in request</p>\n \
    </body>\n\
</html>\n");


    buf->count = buf->len;

    return 0;
}