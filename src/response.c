#include "server.h"


#define SERVER_NAME     "syServer/0.1"

static char sy_err_page_tail[] =
    "<hr><center><span style='font-style: italic;'>"
     SERVER_NAME "</span></center>" CRLF
    "</body>" CRLF
    "</html>" CRLF;

static char sy_err_301_page[] =
    "<html>" CRLF
    "<head><title>301 Moved Permanently</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>301 Moved Permanently</h1></center>" CRLF;

static char sy_err_302_page[] =
    "<html>" CRLF
    "<head><title>302 Found</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>302 Found</h1></center>" CRLF;

static char sy_err_303_page[] =
    "<html>" CRLF
    "<head><title>303 See Other</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>303 See Other</h1></center>" CRLF;

static char sy_err_307_page[] =
    "<html>" CRLF
    "<head><title>307 Temporary Redirect</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>307 Temporary Redirect</h1></center>" CRLF;

static char sy_err_400_page[] =
    "<html>" CRLF
    "<head><title>400 Bad Request</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>400 Bad Request</h1></center>" CRLF;

static char sy_err_401_page[] =
    "<html>" CRLF
    "<head><title>401 Authorization Required</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>401 Authorization Required</h1></center>" CRLF;

static char sy_err_402_page[] =
    "<html>" CRLF
    "<head><title>402 Payment Required</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>402 Payment Required</h1></center>" CRLF;

static char sy_err_403_page[] =
    "<html>" CRLF
    "<head><title>403 Forbidden</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>403 Forbidden</h1></center>" CRLF;

static char sy_err_404_page[] =
    "<html>" CRLF
    "<head><title>404 Not Found</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>404 Not Found</h1></center>" CRLF;

static char sy_err_405_page[] =
    "<html>" CRLF
    "<head><title>405 Not Allowed</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>405 Not Allowed</h1></center>" CRLF;

static char sy_err_406_page[] =
    "<html>" CRLF
    "<head><title>406 Not Acceptable</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>406 Not Acceptable</h1></center>" CRLF;

static char sy_err_407_page[] =
    "<html>" CRLF
    "<head><title>407 Proxy Authentication Required</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>407 Proxy Authentication Required</h1></center>" CRLF;

static char sy_err_408_page[] =
    "<html>" CRLF
    "<head><title>408 Request Time-out</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>408 Request Time-out</h1></center>" CRLF;

static char sy_err_409_page[] =
    "<html>" CRLF
    "<head><title>409 Conflict</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>409 Conflict</h1></center>" CRLF;

static char sy_err_410_page[] =
    "<html>" CRLF
    "<head><title>410 Gone</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>410 Gone</h1></center>" CRLF;

static char sy_err_411_page[] =
    "<html>" CRLF
    "<head><title>411 Length Required</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>411 Length Required</h1></center>" CRLF;

static char sy_err_412_page[] =
    "<html>" CRLF
    "<head><title>412 Precondition Failed</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>412 Precondition Failed</h1></center>" CRLF;

static char sy_err_413_page[] =
    "<html>" CRLF
    "<head><title>413 Request Entity Too Large</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>413 Request Entity Too Large</h1></center>" CRLF;

static char sy_err_414_page[] =
    "<html>" CRLF
    "<head><title>414 Request-URI Too Large</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>414 Request-URI Too Large</h1></center>" CRLF;

static char sy_err_415_page[] =
    "<html>" CRLF
    "<head><title>415 Unsupported Media Type</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>415 Unsupported Media Type</h1></center>" CRLF;

static char sy_err_416_page[] =
    "<html>" CRLF
    "<head><title>416 Requested Range Not Satisfiable</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>416 Requested Range Not Satisfiable</h1></center>" CRLF;

static char sy_err_417_page[] =
    "<html>" CRLF
    "<head><title>417 Expectation Failed</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>417 Expectation Failed</h1></center>" CRLF;

static char sy_err_500_page[] =
    "<html>" CRLF
    "<head><title>500 Internal Server Error</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>500 Internal Server Error</h1></center>" CRLF;

static char sy_err_501_page[] =
    "<html>" CRLF
    "<head><title>501 Not Implemented</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>501 Not Implemented</h1></center>" CRLF;

static char sy_err_502_page[] =
    "<html>" CRLF
    "<head><title>502 Bad Gateway</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>502 Bad Gateway</h1></center>" CRLF;

static char sy_err_503_page[] =
    "<html>" CRLF
    "<head><title>503 Service Temporarily Unavailable</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>503 Service Temporarily Unavailable</h1></center>" CRLF;

static char sy_err_504_page[] =
    "<html>" CRLF
    "<head><title>504 Gateway Time-out</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>504 Gateway Time-out</h1></center>" CRLF;

static char sy_err_505_page[] =
    "<html>" CRLF
    "<head><title>505 HTTP Version Not Supported</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>505 HTTP Version Not Supported</h1></center>" CRLF;

static char sy_err_507_page[] =
    "<html>" CRLF
    "<head><title>507 Insufficient Storage</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>507 Insufficient Storage</h1></center>" CRLF;


#define MIME_MAP_SIZE       (131)
static sy_map_slot_t mime_map_data[2 * MIME_MAP_SIZE];

static sy_map_t mime_map = {
    .size = MIME_MAP_SIZE,
    .max_size = 2 * MIME_MAP_SIZE,
    .data = mime_map_data,
    .cur = mime_map_data + MIME_MAP_SIZE
};

static sy_string_t mime_tb [][2] = {
    {SY_STRING_INIT("htm"),    SY_STRING_INIT("text/html")},
    {SY_STRING_INIT("html"),   SY_STRING_INIT("text/html")},
    {SY_STRING_INIT("gif"),    SY_STRING_INIT("image/gif")},
    {SY_STRING_INIT("ico"),    SY_STRING_INIT("image/x-icon")},
    {SY_STRING_INIT("jpeg"),   SY_STRING_INIT("image/jpeg")},
    {SY_STRING_INIT("jpg"),    SY_STRING_INIT("image/jpeg")},
    {SY_STRING_INIT("png"),    SY_STRING_INIT("image/png")},
    {SY_STRING_INIT("svg"),    SY_STRING_INIT("image/svg+xml")},
    {SY_STRING_INIT("txt"),    SY_STRING_INIT("text/plain")},
    {SY_STRING_INIT("zip"),    SY_STRING_INIT("application/zip")},
    {SY_STRING_INIT("css"),    SY_STRING_INIT("text/css")},
};


static char* sy_err_page(int status, int* len);
static const sy_string_t sy_status_repr(int status);
static void sy_response_put_status_line(sy_request_t* request);
static void sy_response_put_date(sy_request_t* r);

void sy_mime_map_init(void) {
    int n = sizeof(mime_tb) / sizeof(mime_tb[0]);
    for (int i = 0; i < n; ++i) {
        sy_map_val_t val;
        val.mime = mime_tb[i][1];
        sy_map_put(&mime_map, &mime_tb[i][0], &val);
    }
}


int sy_response_build(sy_request_t* r) {
    sy_buffer_t* b = &r->sb;
    
    sy_response_put_status_line(r);
    sy_response_put_date(r);
    sy_buffer_append_cstring(b, "Server: " SERVER_NAME CRLF);
    
    // TODO: Cache-control or Exprires
    switch (r->status) {
    case 304:
        // 304 has no body
        if (r->resource_fd != -1) {
            close(r->resource_fd);
            r->resource_fd = -1;
        }
    
        sy_buffer_append_cstring(b, CRLF);
        return OK;
    case 100:
        // 100 has no body
        if (r->resource_fd != -1) {
            close(r->resource_fd);
            r->resource_fd = -1;
        }
        sy_buffer_append_cstring(b, CRLF);
        return OK;
    default:
        break;
    }
    
    sy_string_t content_type = SY_STRING("text/html");
    sy_map_slot_t* slot = sy_map_get(&mime_map, &r->uri.extension);
    if (slot) {
        content_type = slot->val.mime;
    }
    sy_buffer_append_cstring(b, "Content-Type: ");
    sy_buffer_append_string(b, &content_type);
    sy_buffer_append_cstring(b, CRLF);
    
    sy_buffer_print(b, "Content-Length: %d" CRLF,
                 r->resource_len);
    
    sy_buffer_append_cstring(b, CRLF);
    return OK;
}

static void sy_response_put_status_line(sy_request_t* r) {
    sy_buffer_t* b = &r->sb;
    sy_string_t version;
    if (r->version.minor == 1) {
        version = SY_STRING("HTTP/1.1 ");
    } else {
        version = SY_STRING("HTTP/1.0 ");
    }
    
    sy_buffer_append_string(b, &version);
    sy_string_t status = sy_status_repr(r->status);
    sy_buffer_append_string(b, &status);
    sy_buffer_append_cstring(b, CRLF);
}

static void sy_response_put_date(sy_request_t* r) {
    
    sy_buffer_t* b = &r->sb;   
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    b->end += strftime(b->end, sy_buffer_margin(b),
                       "Date: %a, %d %b %Y %H:%M:%S GMT" CRLF, tm);
}

int sy_response_build_err(sy_request_t* r, int err) {
    sy_buffer_t* b = &r->sb;
    
    r->status = err;

    sy_response_put_status_line(r);
    sy_response_put_date(r);
    sy_buffer_append_cstring(b, "Server: " SERVER_NAME CRLF);
   
    r->keep_alive = false;
    sy_buffer_append_cstring(b, "Connection: close" CRLF);
    sy_buffer_append_cstring(b, "Content-Type: text/html" CRLF);
    
    int page_len;
    int page_tail_len = sizeof(sy_err_page_tail) - 1;
    char* page = sy_err_page(r->status, &page_len);
    if (page != NULL) {
        sy_buffer_print(b, "Content-Length: %d" CRLF,
                page_len + page_tail_len);
    }
    
    sy_buffer_append_cstring(b, CRLF);
    if (page != NULL) {
        sy_buffer_append_string(b, &(sy_string_t){page, page_len});
        sy_buffer_append_string(b, &(sy_string_t){sy_err_page_tail, page_tail_len});
    }
    sy_connection_disable_in(r->c);
    sy_connection_enable_out(r->c);
    r->body_done = true;
    return OK;
}

static char* sy_err_page(int status, int* len) {
#   define SY_ERR_CASE(err)                        \
    case err:                                   \
        *len = sizeof(sy_err_##err##_page) - 1;    \
        return sy_err_##err##_page;    

    switch(status) {
    case 100:
    case 101:
    case 200:
    case 201:
    case 202:
    case 203:
    case 204:
    case 205:
    case 206:
    case 300:
        return NULL;
   
    SY_ERR_CASE(301)
    SY_ERR_CASE(302)
    SY_ERR_CASE(303)
    case 304: 
    case 305:
        assert(false);
        return NULL;
    SY_ERR_CASE(307)
    SY_ERR_CASE(400)
    SY_ERR_CASE(401)
    SY_ERR_CASE(402)
    SY_ERR_CASE(403)
    SY_ERR_CASE(404)
    SY_ERR_CASE(405)
    SY_ERR_CASE(406)
    SY_ERR_CASE(407)
    SY_ERR_CASE(408)
    SY_ERR_CASE(409)
    SY_ERR_CASE(410)
    SY_ERR_CASE(411)
    SY_ERR_CASE(412)
    SY_ERR_CASE(413)
    SY_ERR_CASE(414)
    SY_ERR_CASE(415)
    SY_ERR_CASE(416)
    SY_ERR_CASE(417)
    SY_ERR_CASE(500)
    SY_ERR_CASE(501)
    SY_ERR_CASE(502)
    SY_ERR_CASE(503)
    SY_ERR_CASE(504)
    SY_ERR_CASE(505)
    SY_ERR_CASE(507)

#   undef SY_ERR_CASE 
    
    default:
        assert(false);
        *len = 0; 
        return NULL;
    }
    
    return NULL;   
}

static const sy_string_t sy_status_repr(int status) {
    switch (status) {
    case 100: return SY_STRING("100 Continue");
    case 101: return SY_STRING("101 Switching Protocols");
    case 200: return SY_STRING("200 OK");
    case 201: return SY_STRING("201 Created");
    case 202: return SY_STRING("202 Accepted");
    case 203: return SY_STRING("203 Non-Authoritative Information");
    case 204: return SY_STRING("204 No Content");
    case 205: return SY_STRING("205 Reset Content");
    case 206: return SY_STRING("206 Partial Content");
    case 300: return SY_STRING("300 Multiple Choices");
    case 301: return SY_STRING("301 Moved Permanently");
    case 302: return SY_STRING("302 Found");
    case 303: return SY_STRING("303 See Other");
    case 304: return SY_STRING("304 Not Modified");
    case 305: return SY_STRING("305 Use Proxy");
    case 307: return SY_STRING("307 Temporary Redirect");
    case 400: return SY_STRING("400 Bad Request");
    case 401: return SY_STRING("401 Unauthorized");
    case 402: return SY_STRING("402 Payment Required");
    case 403: return SY_STRING("403 Forbidden");
    case 404: return SY_STRING("404 Not Found");
    case 405: return SY_STRING("405 Method Not Allowed");
    case 406: return SY_STRING("406 Not Acceptable");
    case 407: return SY_STRING("407 Proxy Authentication Required");
    case 408: return SY_STRING("408 Request Time-out");
    case 409: return SY_STRING("409 Conflict");
    case 410: return SY_STRING("410 Gone");
    case 411: return SY_STRING("411 Length Required");
    case 412: return SY_STRING("412 Precondition Failed");
    case 413: return SY_STRING("413 Request Entity Too Large");
    case 414: return SY_STRING("414 Request-URI Too Large");
    case 415: return SY_STRING("415 Unsupported Media Type");
    case 416: return SY_STRING("416 Requested range not satisfiable");
    case 417: return SY_STRING("417 Expectation Failed");
    case 500: return SY_STRING("500 Internal Server Error");
    case 501: return SY_STRING("501 Not Implemented");
    case 502: return SY_STRING("502 Bad Gateway");
    case 503: return SY_STRING("503 Service Unavailable");
    case 504: return SY_STRING("504 Gateway Time-out");
    case 505: return SY_STRING("505 HTTP Version not supported");
    default:
        assert(false);  
        return SY_STRING_NULL;
    }
    
    return SY_STRING_NULL;    
}
