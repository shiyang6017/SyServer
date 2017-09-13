#include "server.h"

#include "base/cJSON.h"

sy_config_t server_cfg;

static void sy_config_init(sy_config_t* cfg) {
    memset(cfg, 0, sizeof(sy_config_t));
}

int sy_config_load(config_t* cfg) {

    sy_config_init(&server_cfg);

    int fd = open("config.json", O_RDONLY);
    if (fd < 0) {
        sy_error("open fail");
        return ERROR;
    }

    struct stat cfg_stat;
    if (fstat(fd, &cfg_stat) < 0) {
        sy_error("fstat fail.");
        return ERROR;
    }

    int file_size = cfg_stat.st_size;
    char* cjson_str = (char*)malloc(file_size + 1);
    
    if (cjson_str == NULL) {
        sy_error("malloc fail.");
        return ERROR;
    }

    if (read(fd, cjson_str, file_size) < 0) {
        sy_error("read fail.");
        goto FAIL;
    }
    cjson_str[file_size] = '\0';

    cJSON * root = cJSON_Parse(cjson_str);
    if (root == NULL) {
        sy_error("cJSON_Parse fail.");
        goto FAIL;
    }

    cJSON *debug = cJSON_GetObjectItemCaseSensitive(root, "debug");
    if (debug == NULL) {
        sy_error(" option of \"debug\" fail.");
        goto FAIL;
    }
    if (cJSON_IsFalse(debug)) {
        server_cfg.debug = false;
        server
    }

    free(cjson_str);
    return OK;

FAIL:
    free(cjson_str);
    return ERROR;
}