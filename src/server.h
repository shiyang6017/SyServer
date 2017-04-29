#ifndef _SY_SERVER_H_
#define _SY_SERVER_H_

#include "base/buffer.h"
#include "base/map.h"
#include "base/pool.h"
#include "base/queue.h"
#include "base/string.h"
#include "base/vector.h"
#include "event.h"
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

/*
 * Config
 */
typedef enum {
    PROT_HTTP,
    PROT_UWSGI, // support only uwsgi
    PROT_FCGI,
} protocol_t;

typedef struct {
    bool pass;
    int fd;
    string_t path;
    string_t root;
    string_t host;
    uint16_t port;
    protocol_t protocol;
} location_t;

typedef struct {
    uint16_t port;
    int root_fd;
    bool debug;
    bool daemon;
    int timeout;
    vector_t workers;
    vector_t locations;
    char* text;
} config_t;

extern config_t server_cfg;

int sy_config_load(config_t* cfg);
void sy_config_destroy(config_t* cfg);

/*
 * Connection
 */
extern int epoll_fd;
extern struct epoll_event events[MAX_EVENT_NUM];
extern pool_t connection_pool;
extern pool_t request_pool;
extern pool_t accept_pool;


#define COMMON_HEADERS              \
    /* General headers */           \
    string_t cache_control;         \
    string_t c;            \
    string_t date;                  \
    string_t pragma;                \
    string_t trailer;               \
    string_t transfer_encoding;     \
    string_t upgrade;               \
    string_t via;                   \
    string_t warning;               \
    /* Entity headers */            \
    string_t allow;                 \
    string_t content_encoding;      \
    string_t content_language;      \
    string_t content_length;        \
    string_t content_location;      \
    string_t content_md5;           \
    string_t content_range;         \
    string_t content_type;          \
    string_t expires;               \
    string_t last_modified;

typedef struct {
    COMMON_HEADERS
    string_t accept;
    string_t accept_charset;
    string_t accept_encoding;
    string_t authorization;
    string_t cookie;
    string_t expect;
    string_t from;
    string_t host;
    string_t if_match;
    string_t if_modified_since;
    string_t if_none_match;
    string_t if_range;
    string_t if_unmodified_since;
    string_t max_forwards;
    string_t proxy_authorization;
    string_t range;
    string_t referer;
    string_t te;
    string_t user_agent;
} request_headers_t;

typedef struct {
    COMMON_HEADERS
    string_t accept_ranges;
    string_t age;
    string_t etag;
    string_t location;
    string_t proxy_authenticate;
    string_t retry_after;
    string_t server;
    string_t vary;
    string_t www_authenticate;
} response_headers_t;

/*
 * Request
 */
typedef enum {
    M_CONNECT,
    M_DELETE,
    M_GET,
    M_HEAD,
    M_OPTIONS,
    M_POST,
    M_PUT,
    M_TRACE,
} method_t;

typedef enum {
    RS_REQUEST_LINE,
    RS_HEADERS,
    RS_BODY,
    RS_PASS_HEADERS,
    RS_PASS_BODY,
} request_stage_t;

// Tranfer coding
typedef enum {
    TE_IDENTITY,
    TE_CHUNKED,
    TE_GZIP,
    TE_COMPRESS,
    TE_DEFLATE,
} transfer_encoding_t;

typedef struct {
    string_t type;
    string_t subtype;
    float q;
} accept_type_t;

typedef struct {
    uint16_t major;
    uint16_t minor;
} version_t;

typedef struct {
    string_t scheme;
    string_t host;
    string_t port;
    string_t abs_path;
    string_t extension;
    string_t query;
    int nddots;
    int nentries;
    int state;
} uri_t;


typedef struct request {
    method_t method;
    version_t version;
    request_headers_t headers;
    list_t accepts;

    // For state machine
    int state;
    string_t request_line;
    string_t header_name;
    string_t header_value;
    uri_t uri;
    string_t host;
    uint16_t port;

    request_stage_t stage;
    
    uint8_t discard_body: 1;
    uint8_t body_done: 1;
    uint8_t done: 1;
    uint8_t response_done: 1;
    uint8_t keep_alive: 1;

    transfer_encoding_t t_encoding;
    int content_length;
    int body_received;
    
    buffer_t rb;
    buffer_t sb;
    struct connection* c;
    struct connection* uc;

    int (*in_handler)(struct request* r);
    int (*out_handler)(struct request* r);
    int status;
    int resource_fd;
    int resource_len;
} request_t;

/*
 * Connection
 */
enum {
    C_SIDE_FRONT,
    C_SIDE_BACK,
};

typedef struct connection {
    int fd; 
    int side; 
    julia_epoll_event_t event; 
    request_t* r;
    time_t active_time;
    int heap_idx;
} connection_t;

#define HTTP_1_1    (version_t){1, 1}
#define HTTP_1_0    (version_t){1, 0}

connection_t* sy_open_connection(int fd);
connection_t* sy_uwsgi_open_connection(request_t* r, location_t* loc);
void sy_close_connection(connection_t* c);
int sy_add_listener(int* listen_fd);
int sy_set_nonblocking(int fd);
void sy_connection_activate(connection_t* c);
void sy_connection_expire(connection_t* c);
bool sy_connection_is_expired(connection_t* c);
int sy_connection_register(connection_t* c);
void sy_connection_unregister(connection_t* c);
void sy_connection_sweep(void);

static inline int sy_connection_disable_in(connection_t* c) {
    if (c->event.events & EVENTS_IN) {
        c->event.events &= ~EVENTS_IN;
        return epoll_ctl(epoll_fd, EPOLL_CTL_MOD,
                         c->fd, &c->event);
    }
    return 0;
}

static inline int sy_connection_enable_in(connection_t* c) {
    if (!(c->event.events & EVENTS_IN)) {
        c->event.events |= EVENTS_IN;
        return epoll_ctl(epoll_fd, EPOLL_CTL_MOD,
                         c->fd, &c->event);
    }
    return 0;
}

static inline int sy_connection_disable_out(connection_t* c) {
    if (c->event.events & EVENTS_OUT) {
        c->event.events &= ~EVENTS_OUT;
        return epoll_ctl(epoll_fd, EPOLL_CTL_MOD,
                         c->fd, &c->event);
    }
    return 0;
}

static inline int sy_connection_enable_out(connection_t* c) {
    if (!(c->event.events & EVENTS_OUT)) {
        c->event.events |= EVENTS_OUT;
        return epoll_ctl(epoll_fd, EPOLL_CTL_MOD,
                         c->fd, &c->event);
    }
    return 0;
} 

/*
 * Request
 */
typedef int (*header_processor_t)(request_t* request, int offset);
void sy_header_map_init(void);
void sy_request_init(request_t* r, connection_t* c);
void sy_request_clear(request_t* request);
void sy_request_release(request_t* request);
int sy_handle_request(connection_t* c);
int sy_handle_response(connection_t* c);
int sy_handle_pass(connection_t* uc);
int sy_handle_upstream(connection_t* uc);
int sy_send_response_buffer(request_t* r);
int sy_send_response_file(request_t* r);
/*
 * Response
 */
void sy_mime_map_init(void);

int sy_response_build(request_t* r);
int sy_response_build_err(request_t* request, int err);


/*
 * Parse
 */
// State machine: request line states
enum {
    // Request line states
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
    RL_S_ALMOST_DONE,
    RL_S_DONE,

    // Header line states
    HL_S_BEGIN,
    HL_S_IGNORE,
    HL_S_NAME,
    HL_S_COLON,
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
int sy_parse_request_line(request_t* request);
int sy_parse_header_line(request_t* request);
int sy_parse_request_body_chunked(request_t* request);
int sy_parse_request_body_identity(request_t* request);
int sy_parse_header_accept(request_t* request);
void sy_parse_header_host(request_t* request);

#endif
