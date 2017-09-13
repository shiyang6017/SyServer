#ifndef _SY_SERVER_H_
#define _SY_SERVER_H_

#include "base/buffer.h"
#include "base/map.h"
#include "base/mempool.h"
#include "base/queue.h"
#include "base/string.h"
#include "base/vector.h"
#include "server.h"
#include "util.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <sys/sendfile.h>
#include <sys/un.h>
#include <sys/epoll.h>
#define MAX_EVENT_NUM   (65536)

#define EVENTS_IN   (EPOLLIN)
#define EVENTS_OUT  (EPOLLOUT)




typedef enum {
    PROT_HTTP,
    PROT_UWSGI, // TODO
    PROT_FCGI,  // TODO
} sy_protocol_t;

typedef struct {
    bool pass;
    int fd;
    sy_string_t path;
    sy_string_t root;
    sy_string_t host;
    uint16_t port;
    sy_protocol_t protocol;
} sy_location_t;

typedef struct {
    uint16_t port;
    int root_fd;
    bool debug;
    bool daemon;
    int timeout;
    sy_vector_t workers;
    sy_vector_t locations;
    char* text;
} sy_config_t;

extern sy_config_t server_cfg;

int sy_config_load(sy_config_t* cfg);
void sy_config_destroy(sy_config_t* cfg);

/*
 * Connection
 */
extern int epoll_fd;
extern struct epoll_event events[MAX_EVENT_NUM];
extern sy_pool_t connection_pool;
extern sy_pool_t request_pool;
extern sy_pool_t accept_pool;


#define COMMON_HEADERS              \
    /* General headers */           \
    sy_string_t cache_control;         \
    sy_string_t c;            \
    sy_string_t date;                  \
    sy_string_t pragma;                \
    sy_string_t trailer;               \
    sy_string_t transfer_encoding;     \
    sy_string_t upgrade;               \
    sy_string_t via;                   \
    sy_string_t warning;               \
    /* Entity headers */            \
    sy_string_t allow;                 \
    sy_string_t content_encoding;      \
    sy_string_t content_language;      \
    sy_string_t content_length;        \
    sy_string_t content_location;      \
    sy_string_t content_md5;           \
    sy_string_t content_range;         \
    sy_string_t content_type;          \
    sy_string_t expires;               \
    sy_string_t last_modified;

typedef struct {
    COMMON_HEADERS
    sy_string_t accept;
    sy_string_t accept_charset;
    sy_string_t accept_encoding;
    sy_string_t authorization;
    sy_string_t cookie;
    sy_string_t expect;
    sy_string_t from;
    sy_string_t host;
    sy_string_t if_match;
    sy_string_t if_modified_since;
    sy_string_t if_none_match;
    sy_string_t if_range;
    sy_string_t if_unmodified_since;
    sy_string_t max_forwards;
    sy_string_t proxy_authorization;
    sy_string_t range;
    sy_string_t referer;
    sy_string_t te;
    sy_string_t user_agent;
} sy_request_headers_t;

typedef struct {
    COMMON_HEADERS
    sy_string_t accept_ranges;
    sy_string_t age;
    sy_string_t etag;
    sy_string_t location;
    sy_string_t proxy_authenticate;
    sy_string_t retry_after;
    sy_string_t server;
    sy_string_t vary;
    sy_string_t www_authenticate;
} sy_response_headers_t;

/*
 * Request
 */
typedef enum {
    M_CONNECT,
    M_DELETE, 
    M_GET,  // support
    M_HEAD, // support
    M_OPTIONS,
    M_POST, 
    M_PUT,
    M_TRACE,
} sy_method_t;

typedef enum {
    RS_REQUEST_LINE,
    RS_HEADERS,
    RS_BODY,
    RS_PASS_HEADERS,
    RS_PASS_BODY,
} sy_request_stage_t;

// Tranfer coding
typedef enum {
    TE_IDENTITY, // support only
    TE_CHUNKED,
    TE_GZIP,
    TE_COMPRESS,
    TE_DEFLATE,
} sy_transfer_encoding_t;

typedef struct {
    sy_string_t type;
    sy_string_t subtype;
    float q;
} sy_accept_type_t;

typedef struct {
    uint16_t major;
    uint16_t minor;
} sy_version_t;

typedef struct {
    sy_string_t scheme;
    sy_string_t host;
    sy_string_t port;
    sy_string_t abs_path;
    sy_string_t extension;
    sy_string_t query;
    int nddots;
    int nentries;
    int state;
} sy_uri_t;




typedef struct sy_request{
    sy_method_t method;
    sy_version_t version;
    sy_request_headers_t headers;
    sy_list_t accepts;

    // For state machine
    int state;
    sy_string_t request_line;
    sy_string_t header_name;
    sy_string_t header_value;
    sy_uri_t uri;
    sy_string_t host;
    uint16_t port;

    sy_request_stage_t stage;
    
    uint8_t discard_body: 1;
    uint8_t body_done: 1;
    uint8_t done: 1;
    uint8_t response_done: 1;
    uint8_t keep_alive: 1;

    sy_transfer_encoding_t t_encoding;
    int content_length;
    int body_received;
    
    sy_buffer_t rb;
    sy_buffer_t sb;
    struct sy_connection* c;
    struct sy_connection* uc;

    int (*in_handler)(struct sy_request* r);
    int (*out_handler)(struct sy_request* r);
    int status;
    int resource_fd;
    int resource_len;
} sy_request_t;

/*
 * Connection
 */
enum {
    C_SIDE_FRONT,
    C_SIDE_BACK, // TODO
};

typedef struct {
    int fd; 
    int side; 
    struct epoll_event event; 
    sy_request_t* r;
    time_t active_time;
    int heap_idx;
} sy_connection_t;

#define HTTP_1_1    (sy_version_t){1, 1}
#define HTTP_1_0    (sy_version_t){1, 0}

sy_connection_t* sy_open_connection(int fd);
sy_connection_t* sy_uwsgi_open_connection(sy_request_t* r, sy_location_t* loc);
void sy_close_connection(sy_connection_t* c);
int sy_add_listener(int* listen_fd);
int sy_set_nonblocking(int fd);
void sy_connection_activate(sy_connection_t* c);
void sy_connection_expire(sy_connection_t* c);
bool sy_connection_is_expired(sy_connection_t* c);
int sy_connection_register(sy_connection_t* c);
void sy_connection_unregister(sy_connection_t* c);
void sy_connection_sweep(void);

static inline int sy_connection_disable_in(sy_connection_t* c) {
    if (c->event.events & EVENTS_IN) {
        c->event.events &= ~EVENTS_IN;
        return epoll_ctl(epoll_fd, EPOLL_CTL_MOD,
                         c->fd, &c->event);
    }
    return 0;
}

static inline int sy_connection_enable_in(sy_connection_t* c) {
    if (!(c->event.events & EVENTS_IN)) {
        c->event.events |= EVENTS_IN;
        return epoll_ctl(epoll_fd, EPOLL_CTL_MOD,
                         c->fd, &c->event);
    }
    return 0;
}

static inline int sy_connection_disable_out(sy_connection_t* c) {
    if (c->event.events & EVENTS_OUT) {
        c->event.events &= ~EVENTS_OUT;
        return epoll_ctl(epoll_fd, EPOLL_CTL_MOD,
                         c->fd, &c->event);
    }
    return 0;
}

static inline int sy_connection_enable_out(sy_connection_t* c) {
    if (!(c->event.events & EVENTS_OUT)) {
        c->event.events |= EVENTS_OUT;
        return epoll_ctl(epoll_fd, EPOLL_CTL_MOD,
                         c->fd, &c->event);
    }
    return 0;
} 



typedef int (*sy_header_processor_t)(sy_request_t* request, int offset);
void sy_header_map_init(void);
void sy_request_init(sy_request_t* r, sy_connection_t* c);
void sy_request_clear(sy_request_t* request);
void sy_request_release(sy_request_t* request);
int sy_handle_request(sy_connection_t* c);
int sy_handle_response(sy_connection_t* c);
int sy_handle_pass(sy_connection_t* uc);
int sy_handle_upstream(sy_connection_t* uc);
int sy_send_response_buffer(sy_request_t* r);
int sy_send_response_file(sy_request_t* r);

void sy_mime_map_init(void);

int sy_response_build(sy_request_t* r);
int sy_response_build_err(sy_request_t* request, int err);

// state : used for parser
enum {
    // request line states
    RL_S_BEGIN = 0,
    RL_S_METHOD,
    RL_S_SP_BEFORE_URI,
    RL_S_URI, 
    RL_S_SP_BEFROE_VERSION, 
    RL_S_HTTP_H,
    RL_S_HTTP_HT,
    RL_S_HTTP_HTT,
    RL_S_HTTP_HTTP,
    RL_S_HTTP_VERSION_SLASH,
    RL_S_HTTP_VERSION_MAJOR,
    RL_S_HTTP_VERSION_DOT,
    RL_S_HTTP_VERSION_MINOR,
    RL_S_SP_AFTER_VERSION,
    RL_S_ALMOST_DONE, // CR
    RL_S_DONE,        // LF

    // header line states
    HL_S_BEGIN,
    HL_S_IGNORE,
    HL_S_NAME,
    HL_S_COLON, // : 
    HL_S_SP_BEFORE_VALUE,
    HL_S_VALUE,
    HL_S_SP_AFTER_VALUE,
    HL_S_ALMOST_DONE, 
    HL_S_DONE,       

    // URI states
    URI_S_BEGIN,
    URI_S_SCHEME,
    URI_S_SCHEME_COLON,
    URI_S_SCHEME_SLASH,
    URI_S_SCHEME_SLASH_SLASH,
    URI_S_HOST,
    URI_S_PORT,
    URI_S_ABS_PATH_DOT,
    URI_S_ABS_PATH_DDOT,
    URI_S_ABS_PATH_SLASH,
    URI_S_ABS_PATH_ENTRY,
    URI_S_EXTENSION,
    URI_S_QUERY,
};


void sy_parse_init(void);
int sy_parse_request_line(sy_request_t* request);
int sy_parse_header_line(sy_request_t* request);
int sy_parse_request_body_chunked(sy_request_t* request);
int sy_parse_request_body_identity(sy_request_t* request);
int sy_parse_header_accept(sy_request_t* request);
void sy_parse_header_host(sy_request_t* request);

#endif
