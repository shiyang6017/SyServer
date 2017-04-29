#include "server.h"

static int sy_startup(uint16_t port);
static int sy_server_init(void);
static void sy_usage(void);

static void sy_usage(void) {
    printf("Usage:\n"
           "    syServer [-s signal]    signal could only be stop\n");
    exit(OK);
}

static void sy_save_pid(int pid) {
    FILE* fp = fopen(INSTALL_DIR "syServer.pid", "w");
    if (fp == NULL) {
        ju_error("open pid file: " INSTALL_DIR "syServer.pid failed");
        exit(ERROR);
    }
    fprintf(fp, "%d", pid);
    fclose(fp);
}

static int sy_get_pid(void) {
    FILE* fp = fopen(INSTALL_DIR "syServer.pid", "a+");
    if (fp == NULL) {
        ju_error("open pid file: " INSTALL_DIR "syServer.pid failed");
        exit(ERROR);
    }
    int pid = 0;
    fscanf(fp, "%d", &pid);
    return pid;
}

static void sy_send_signal(const char* signal) {
    if (strncasecmp(signal, "stop", 4) == 0) {
        kill(-get_pid(), SIGINT);
    } else {
        usage();
    }
    exit(OK);
}

static void sy_sig_int(int signo) {
    ju_log("syServer exited...");
    save_pid(0);
    kill(-getpid(), SIGINT);
    raise(SIGKILL);
}

static int sy_startup(uint16_t port) {
    // If the client closed the c, then it will cause SIGPIPE
    // Here simplely ignore this SIG
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, sig_int);

    // Open socket, bind and listen
    int listen_fd = 0;
    struct sockaddr_in server_addr = {0};
    int addr_len = sizeof(server_addr);
    listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd == ERROR) {
        return ERROR;
    }

    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (server_cfg.workers.size > 1) {
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    }

    memset((void*)&server_addr, 0, addr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listen_fd, (struct sockaddr*)&server_addr, addr_len) < 0) {
        return ERROR;
    }

    if (listen(listen_fd, 1024) < 0) {
        return ERROR;
    }
    return listen_fd;
}

static int sy_server_init(void) {
    // Set limits
    struct rlimit nofile_limit = {65535, 65535};
    setrlimit(RLIMIT_NOFILE, &nofile_limit);

    sy_parse_init();
    sy_header_map_init();
    sy_mime_map_init();

    sy_pool_init(&connection_pool, sizeof(connection_t), 8, 0);
    sy_pool_init(&request_pool, sizeof(request_t), 8, 0);

    epoll_fd = epoll_create1(0);
    SY_ABORT_ON(epoll_fd == ERROR, "epoll_create1");
    return OK;
}

static void sy_accept_connection(int listen_fd) {
    while (true) {
        int c_fd = accept(listen_fd, NULL, NULL);
        if (c_fd == ERROR) {
            SY_ERR_ON((errno != EWOULDBLOCK), "accept");
            break;
        }
        sy_open_connection(c_fd);
    }
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        if (argv[1][0] != '-') {
            sy_usage();
        }
        switch (argv[1][1]) {
        case 'h': sy_usage(); break;
        case 's': send_signal(argv[2]); break;
        default: sy_usage(); break;
        }
    }

    get_pid();

    if (sy_config_load(&server_cfg) != OK) {
        raise(SIGINT);
    }

    if (server_cfg.debug) {
        goto work;
    }

    if (server_cfg.daemon) {
        sy_daemon(1, 0);
    }

    if (get_pid() != 0) {
        sy_error("syServer has already been running...");
        exit(ERROR);
    }

    sy_save_pid(getpid());

    int nworker = 0;
    while (true) {
        if (nworker >= server_cfg.workers.size) {
            int stat;
            wait(&stat);
            if (WIFEXITED(stat))
                raise(SIGINT);
            // Worker unexpectly exited, restart it
            sy_log("syServer failed, restarting...");
        }
        int pid = fork();
        SY_ABORT_ON(pid < 0, "fork");
        if (pid == 0) {
            break;
        }
        int* worker = sy_vector_at(&server_cfg.workers, nworker++);
        *worker = pid;
    }

work:;
    int listen_fd;
    if (sy_server_init() != OK ||
        (listen_fd = sy_startup(server_cfg.port)) < 0) {
        sy_error("startup server failed");
        exit(ERROR);
    }

    sy_log("syServer started...");
    sy_log("listening at port: %u", server_cfg.port);
    assert(sy_add_listener(&listen_fd) != ERROR);

wait:;
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENT_NUM, 30);
    if (nfds == ERROR) {
        SY_ABORT_ON(errno != EINTR, "epoll_wait");
    }

    for (int i = 0; i < nfds; ++i) {
        int fd = *((int*)(events[i].data.ptr));
        if (fd == listen_fd) {
            // We could accept more than one c per request
            sy_accept_connection(listen_fd);
            continue;
        }

        int err;
        connection_t* c = events[i].data.ptr;
        if (!sy_connection_is_expired(c) && events[i].events & EPOLLIN) {
            err = (c->side == C_SIDE_BACK) ?
                  sy_handle_upstream(c): sy_handle_request(c);
            err == ERROR ? sy_connection_expire(c): sy_connection_activate(c);
        }
        if (!sy_connection_is_expired(c) && events[i].events & EPOLLOUT) {
            err = (c->side == C_SIDE_BACK) ?
                  sy_handle_pass(c): sy_handle_response(c);
            err == ERROR ? sy_connection_expire(c): sy_connection_activate(c);
        }
    }
    sy_connection_sweep();
    goto wait;

    close(epoll_fd);
    close(listen_fd);
    return OK;
}
