#include "buffer.h"
#include "string.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>


int sy_buffer_recv(buffer_t* buffer, int fd) {
    while (!buffer_full(buffer)) {
        int margin = buffer->limit - buffer->end;
        int len = recv(fd, buffer->end, margin, 0);
        if (len == 0) // EOF
            return OK;
        if (len == -1) {
            if (errno == EAGAIN)
                return AGAIN;
            perror("recv err");
            return ERROR;
        }
        //read_n += len;
        buffer->end += len;
    };  // We may have not read all data
    return AGAIN;
}

int sy_buffer_send(buffer_t* buffer, int fd) {
    //int sent = 0;
    while (buffer_size(buffer) > 0) {
        int len = send(fd, buffer->begin, buffer_size(buffer), 0);
        if (len == -1) {
            if (errno == EAGAIN)
                return AGAIN;
            else if (errno == EPIPE) {
                // TODO: the connection is broken
            }
            perror("send");
            return ERROR;
        }
        //sent += len;
        buffer->begin += len;
    };
    buffer_clear(buffer);
    return OK;
}

int sy_buffer_append_string(buffer_t* buffer, const string_t* str) {
    int margin = buffer->limit - buffer->end;
    assert(margin > 0);
    int appended = min(margin, str->len);
    memcpy(buffer->end, str->data, appended);
    buffer->end += appended;
    return appended;
}

int sy_buffer_print(buffer_t* buffer, const char* format, ...) {
    va_list args;
    va_start (args, format);
    int margin = buffer->limit - buffer->end;
    assert(margin > 0);
    int len = vsnprintf(buffer->end, margin, format, args);
    buffer->end += len;
    va_end (args);
    return len;
}

void sy_print_buffer(buffer_t* buffer) {
    for(char* p = buffer->begin; p != buffer->end; ++p) {
        printf("%c", *p);
        fflush(stdout);
    }
    printf("\n");
}
