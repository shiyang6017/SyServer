#include "server.h"

static int sy_request_handle_uri(request_t* r);
static int sy_request_handle_request_line(request_t* r);
static int sy_request_handle_headers(request_t* r);
static int sy_request_handle_body(request_t* r);
static int sy_header_handle_generic(request_t* r, int offset);
static int sy_header_handle_connection(request_t* r, int offset);
static int sy_header_handle_t_encoding(request_t* r, int offset);
static int sy_header_handle_content_length(request_t* r, int offset);
static int sy_header_handle_host(request_t* r, int offset);
static int sy_header_handle_accept(request_t* r, int offset);
static int sy_header_handle_if_modified_since(request_t* r, int offset);


typedef struct {
    string_t name;
    header_val_t val;
} header_nv_t;

#define HEADER_PAIR(name, processor)    \
    {STRING_INIT(#name), {offsetof(request_headers_t, name), processor}}

static header_nv_t header_tb[] = {
    HEADER_PAIR(cache_control, header_handle_generic),
    HEADER_PAIR(c, header_handle_connection),
    HEADER_PAIR(date, header_handle_generic),
    HEADER_PAIR(pragma, header_handle_generic),
    HEADER_PAIR(trailer, header_handle_generic),
    HEADER_PAIR(transfer_encoding, header_handle_t_encoding),
    HEADER_PAIR(upgrade, header_handle_generic),
    HEADER_PAIR(via, header_handle_generic),
    HEADER_PAIR(warning, header_handle_generic),
    HEADER_PAIR(allow, header_handle_generic),
    HEADER_PAIR(content_encoding, header_handle_generic),
    HEADER_PAIR(content_language, header_handle_generic),
    HEADER_PAIR(content_length, header_handle_content_length),
    HEADER_PAIR(content_location, header_handle_generic),
    HEADER_PAIR(content_md5, header_handle_generic),
    HEADER_PAIR(content_range, header_handle_generic),
    HEADER_PAIR(content_type, header_handle_generic),
    HEADER_PAIR(expires, header_handle_generic),
    HEADER_PAIR(last_modified, header_handle_generic),

    HEADER_PAIR(accept, header_handle_accept),
    HEADER_PAIR(accept_charset, header_handle_generic),
    HEADER_PAIR(accept_encoding, header_handle_generic),
    HEADER_PAIR(authorization, header_handle_generic),
    HEADER_PAIR(cookie, header_handle_generic),
    HEADER_PAIR(expect, header_handle_generic),
    HEADER_PAIR(from, header_handle_generic),
    HEADER_PAIR(host, header_handle_host),
    HEADER_PAIR(if_match, header_handle_generic),
    HEADER_PAIR(if_modified_since, header_handle_if_modified_since),
    HEADER_PAIR(if_none_match, header_handle_generic),
    HEADER_PAIR(if_range, header_handle_generic),
    HEADER_PAIR(if_unmodified_since, header_handle_generic),
    HEADER_PAIR(max_forwards, header_handle_generic),
    HEADER_PAIR(proxy_authorization, header_handle_generic),
    HEADER_PAIR(range, header_handle_generic),
    HEADER_PAIR(referer, header_handle_generic),
    HEADER_PAIR(te, header_handle_generic),
    HEADER_PAIR(user_agent, header_handle_generic),
};

#undef HEADER_PAIR

#define HEADER_MAP_SIZE     (131)
static map_slot_t header_map_data[2 * HEADER_MAP_SIZE];
static map_t header_map = {
    .size = HEADER_MAP_SIZE,
    .max_size = 2 * HEADER_MAP_SIZE,
    .data = header_map_data,
    .cur = header_map_data + HEADER_MAP_SIZE
};

void sy_header_map_init(void) {
    int n = sizeof(header_tb) / sizeof(header_tb[0]);
    for (int i = 0; i < n; ++i) {
        map_val_t val;
        val.header = header_tb[i].val;
        sy_map_put(&header_map, &header_tb[i].name, &val);
    }
}

static inline void sy_uri_init(uri_t* uri) {
    memset(uri, 0, sizeof(uri_t));
    uri->state = URI_S_BEGIN;
}

/*
 * Request
 */
void sy_request_init(request_t* r, connection_t* c) {
    r->method = M_GET;    
    r->version.major = 0;
    r->version.minor = 0;

    memset(&r->headers, 0, sizeof(r->headers));
    sy_list_init(&r->accepts, &accept_pool);

    r->state = RL_S_BEGIN;
    sy_string_init(&r->request_line);
    sy_string_init(&r->header_name);
    sy_string_init(&r->header_value);
    sy_uri_init(&r->uri);
    sy_string_init(&r->host);
    r->port = 80;

    r->stage = RS_REQUEST_LINE;

    r->discard_body = false;
    r->body_done = false;
    r->done = false;
    r->response_done = false;
    r->keep_alive = false;

    r->t_encoding = TE_IDENTITY;
    r->content_length = -1;
    r->body_received = 0;

    sy_buffer_init(&r->rb);
    sy_buffer_init(&r->sb);
    r->c = c;
    r->uc = NULL;
    r->in_handler = request_handle_request_line;
    r->out_handler = send_response_buffer;

    r->status = 200;
    r->resource_fd = -1;
    r->resource_len = 0;
}

void sy_request_clear(request_t* r) {
    connection_t* c = r->c;
    connection_t* uc = r->uc;
    request_init(r, c);
    r->c = c;
    r->uc = uc;
}

/*
 * Send recived data to backend
 * Return:
 *  OK: send all data in buffer
 *  AGAIN: send partial data
 *  ERROR: error occurred, the upstream connection must be closed
 */

int sy_handle_pass(connection_t* uc) {
    request_t* r = uc->r;
    buffer_t* b = &r->rb;
    int err = buffer_send(b, uc->fd);
    if (err == OK) {
        // Remove the EPOLLOUT event of upstream side
        // Done send all data
        buffer_clear(b);
        connection_enable_in(r->c);
        connection_disable_out(uc);
    } else if (err == ERROR) {
        // The connection has been closed by peer
        response_build_err(r, 503);
    }
    return err;
}

/*
 * The upstream connection is closed every response finished
 * Return:
 *  OK, ERROR: the upstream connection must be closed
 *  AGAIN:
 */
int sy_handle_upstream(connection_t* uc) {
    request_t* r = uc->r;
    buffer_t* b = &r->sb;
    int err = buffer_recv(b, uc->fd);
    if (err == OK) {
        // The connection has been closed by peer
        return ERROR;
    } else if (err == ERROR) {
        // Error in backend
        response_build_err(r, 503);
        return ERROR;
    } else if (buffer_full(b)) {
        //connection_disable_in(uc);
    }
    connection_enable_out(r->c);
    return err;
}

int sy_send_response_buffer(request_t* r) {
    connection_t* c= r->c;
    buffer_t* b = &r->sb;
    int err = buffer_send(b, c->fd);
    if (err == OK) {
        buffer_clear(b);
        if (r->uc) {
            return AGAIN;
        } else if (r->resource_fd != -1) {
            r->out_handler = send_response_file;
            return OK;
        }
        connection_disable_out(c);
        r->response_done = true;
        return OK;
    } else if (err == ERROR) {
        return ERROR;
    }
    return AGAIN;
}

int sy_send_response_file(request_t* r) {
    connection_t* c = r->c;
    assert(!r->uc);
    int fd = c->fd;
    while (1) {
        int len = sendfile(fd, r->resource_fd, NULL, r->resource_len);
        if (len == 0) {
            // Have send the whole file
            r->response_done = true;
            close(r->resource_fd);
            r->resource_fd = -1;
            return OK;
        } else if (len < 0) {
            if (errno == EAGAIN)
                return AGAIN;
            SY_ERR_ON(1, "sendfile");
            return ERROR;
        }
    }
    return OK;  
}


int handle_response(connection_t* c) {
    request_t* r = c->r;
    int err;
    do {
        err = r->out_handler(r);
    } while (err == OK && !r->response_done);
    if (r->response_done) {
        if (r->keep_alive) {
            connection_disable_out(c);
            connection_enable_in(c);
            request_clear(r);
        } else {
            return ERROR;
        }
    }
    return err;
}

int sy_handle_request(connection_t* c) {
    request_t* r = c->r;
    buffer_t* b = &r->rb;
    int err = buffer_recv(b, c->fd);
    if (err != AGAIN) {
        return ERROR;
    }

    do {
        err = r->in_handler(r);
    } while (err == OK && !r->body_done);
    return err;
}

static location_t* sy_match_location(string_t* path) {
    vector_t* locs = &server_cfg.locations;
    for (int i = 0; i < locs->size; ++i) {
        location_t* loc = vector_at(locs, i);
        // The first matched location is returned
        if (strncasecmp(loc->path.data, path->data, loc->path.len) == 0)
            return loc;
    }
    return NULL;
}

static int sy_request_handle_uri(request_t* r) {
    uri_t* uri = &r->uri;
    if (uri->host.data) {
        r->host = uri->host;
        if (uri->port.data) {
            r->port = atoi(uri->port.data);
        }
    }

    location_t* loc = match_location(&uri->abs_path);
    if (loc == NULL) {
        return sy_response_build_err(r, 404);
    }
    if (loc->pass) {
        if (!(r->uc = sy_uwsgi_open_connection(r, loc))) {
            return sy_response_build_err(r, 503);
        }
        return OK;
    }

    const char* rel_path;
    // It is safe to do this, as we are not proxy now
    uri->abs_path.data[uri->abs_path.len] = 0;
    if (uri->abs_path.len == 1) {
        rel_path = "./";
    } else {
        rel_path = uri->abs_path.data + 1;
    }

    int fd = openat(server_cfg.root_fd, rel_path, O_RDONLY);

    // Open the requested resource failed
    if (fd == -1) {
        return sy_response_build_err(r, 404);
    }
    struct stat st;
    fstat(fd, &st);
    if (S_ISDIR(st.st_mode)) {
        int tmp_fd = fd;
        fd = openat(fd, "index.html", O_RDONLY);
        close(tmp_fd); 
        if (fd == -1) {
            // Accessing to a directory is forbidden
            return sy_response_build_err(r, 403);
        }
        fstat(fd, &st);
        uri->extension = STRING("html");
    }
    r->resource_fd = fd;
    r->resource_len = st.st_size;
    return OK;
}

static int sy_request_handle_request_line(request_t* r) {
    int err = sy_parse_request_line(r);
    if (err == AGAIN) {
        return err;
    } else if (err != OK) {
        // Reuqest line parsing error, can't recovery
        return sy_response_build_err(r, 400);
    }
    // Supports only HTTP/1.1
    if (r->version.major != 1 || r->version.minor > 2) {
        return sy_response_build_err(r, 505);
    }

    // HTTP/1.1: persistent c default
    r->keep_alive = r->version.minor == 1;

    // TODO: check method
    // We still need to receive the left part of this r
    // Thus, the c will hold
    r->in_handler = request_handle_headers;
    return sy_request_handle_uri(r);
}

static int sy_request_handle_headers(request_t* r) {
    int err;
    while (true) {
        err = sy_parse_header_line(r);
        switch (err) {
        case AGAIN:
            return AGAIN;
        case EMPTY_LINE:
            goto done;
        case OK: {
            map_slot_t* slot = sy_map_get(&header_map, &r->header_name);
            if (slot == NULL)
                break;
            header_val_t header = slot->val.header;
            if (header.offset != -1) {
                header_processor_t processor = header.processor;
                int err = processor(r, header.offset);
                if (err != 0)
                    return err;
            }
        } break;
        default:
            assert(false);
        }
    }

done:
    r->in_handler = request_handle_body;
    return OK;
}

static int header_handle_connection(request_t* r, int offset) {
    header_handle_generic(r, offset);
    request_headers_t* headers = &r->headers;
    if(strncasecmp("close", headers->c.data, 5) == 0)
        r->keep_alive = false;
    return OK;
}

static int header_handle_t_encoding(request_t* r, int offset) {
    header_handle_generic(r, offset);

    string_t* transfer_encoding = &r->headers.transfer_encoding;
    if (strncasecmp("chunked", transfer_encoding->data, 7) == 0) {
        r->t_encoding = TE_CHUNKED;
    } else if (strncasecmp("gzip", transfer_encoding->data, 4) == 0
            || strncasecmp("x-gzip", transfer_encoding->data, 6) == 0) {
        r->t_encoding = TE_GZIP;
    } else if (strncasecmp("compress", transfer_encoding->data, 8) == 0
            || strncasecmp("x-compress", transfer_encoding->data, 10) == 0) {
        r->t_encoding = TE_COMPRESS;
    } else if (strncasecmp("deflate", transfer_encoding->data, 7) == 0) {
        r->t_encoding = TE_DEFLATE;
    } else if (strncasecmp("identity", transfer_encoding->data, 8) == 0) {
        r->t_encoding = TE_IDENTITY;
    } else {
        // Must close the c as we can't understand the body
        return response_build_err(r, 415);
    }
    return OK;
}

static int header_handle_content_length(request_t* r, int offset) {
    header_handle_generic(r, offset);
    //sring_t* name = &r->header_name;
    string_t* val = &r->header_value;

    int len = atoi(val->data);
    if (len < 0) {
        return response_build_err(r, 400);
    }
    r->content_length = len;
    return OK;
}

// If both the uri in the r contains host[:port]
// and has this host header, the host header is active.
static int header_handle_host(request_t* r, int offset) {
    header_handle_generic(r, offset);
    //memcpy(r->headers.host.data, "localhost:3030", 14);
    return OK;
}

static int header_handle_if_modified_since(request_t* r, int offset) {
    header_handle_generic(r, offset);
    return OK;
}

static int request_handle_body(request_t* r) {
    int err = OK;
    switch (r->t_encoding) {
    case TE_IDENTITY:
        err = parse_request_body_identity(r);
        break;
    case TE_CHUNKED:
        assert(false);
        err = parse_request_body_chunked(r);
        break;
    default:
        // TODO(wgtdkp): cannot understanding
        // May discard the body
        assert(false);
    }

    buffer_t* b = &r->rb;
    switch (err) {
    case AGAIN:
        connection_disable_in(r->c);
        b->begin = b->data;
        connection_enable_out(r->uc);
        return AGAIN;
    case OK:
        // Do not allow pipelining
        connection_disable_in(r->c);
        if (!r->uc) {
            response_build(r);
            buffer_clear(b);
            connection_enable_out(r->c);
        } else {
            b->begin = b->data;
            connection_enable_out(r->uc);
        }
        r->body_done = true;
        r->in_handler = request_handle_request_line;
        return OK;
    default:
        return response_build_err(r, 400);
    }
    return OK;
}

static int header_handle_accept(request_t* r, int offset) {
    header_handle_generic(r, offset);
    //parse_header_accept(r);
    return OK;
}

static int header_handle_generic(request_t* r, int offset) {
    string_t* member = (string_t*)((char*)&r->headers + offset);
    *member = r->header_value;
    return OK;
}
