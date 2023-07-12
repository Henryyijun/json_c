#include "json.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if ACTIVE_PLAN == 1
//---------------------------------------------------------------------------
//  方案1
//---------------------------------------------------------------------------
/**
 * @brief 创建一个JSON对象(readme.md文件中的范例)
 */
static JSON *create_json(void)
{
    JSON *json = json_new(JSON_OBJ);
    if (!json)
        goto failed_;
    JSON *basic = json_new(JSON_OBJ);
    if (!basic)
        goto failed_;

    json_add_member(json, "basic", basic);
    json_add_member(basic, "enable", json_new_bool(TRUE));
    json_add_member(basic, "port", json_new_num(389));
    json_add_member(basic, "timeout", json_new_num(10));
    json_add_member(basic, "basedn", json_new_str("aa"));
    json_add_member(basic, "fd", json_new_num(-1));
    json_add_member(basic, "maxcnt", json_new_num(133333333333));
    
    JSON* array = json_new(JSON_ARR);
    json_add_element(array, json_new_str("200.200.0.1"));
    json_add_element(array, json_new_str("200.0.0.254"));
    json_add_member(basic, "dns", array);
    JSON* advance = json_new(JSON_OBJ);
    json_add_member(basic, "advance", advance);
    JSON* dns_array = json_new(JSON_ARR);
    JSON* dns_array_obj_1 = json_new(JSON_OBJ);
    json_add_member(dns_array_obj_1, "name", json_new_str("huannan"));
    json_add_member(dns_array_obj_1, "ip", json_new_str("200.200.0.0.1"));

    JSON* dns_array_obj_2 = json_new(JSON_OBJ);
    json_add_member(dns_array_obj_2, "name", json_new_str("huanbei"));
    json_add_member(dns_array_obj_2, "ip", json_new_str("200.0.0.254"));
    json_add_element(dns_array, dns_array_obj_1);
    json_add_element(dns_array, dns_array_obj_2);

    JSON* portpool_array = json_new(JSON_ARR);
    json_add_element(portpool_array, json_new_num(130));
    json_add_element(portpool_array, json_new_num(131));
    json_add_element(portpool_array, json_new_num(132));
    json_add_member(advance, "portpool", portpool_array);
    json_add_member(advance, "dns", dns_array);
    json_add_member(advance, "url", json_new_str("http://200.200.0.4/main"));
    json_add_member(advance, "path", json_new_str("/etc/sinfors"));
    json_add_member(advance, "value", json_new_num(3.14));

    //TODO: 补充完善代码，构建出完整的JSON对象(即readme.md中展示的范例)
    if (!json_add_member(basic, "ip", json_new_str("200.200.3.61"))) {
        printf("failed_");
        goto failed_;
    }   
    return json;
failed_:
    json_free(json);
    return NULL;
}
/**
 * @brief 后台服务程序使用的配置项
 */
typedef struct dns_st {
    char* name;
    char* ip;
} dns_st;

typedef struct advance_st {
    int* portpool;
    char* url;
    char* path;
    double val;
    dns_st* ds;
} advance_st;

typedef struct config_st {
    char *ip;
    int port;
    BOOL enable;
    char *basedn;
    char** dns;
    int timeout;
    int fd;
    long long maxcnt;
    advance_st ade;
    //TODO: ...
} config_st;

/**
 * @brief 初始化为默认配置
 */
static void config_init(config_st *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
}
/**
 * @brief 释放加载的配置数据
 */
static void config_clear(config_st *cfg)
{
    free(cfg->ip);
    free(cfg->basedn);
    memset(cfg, 0, sizeof(*cfg));
}
/**
 * @brief 从JSON数据中加载配置
 */
static int config_load(config_st *cfg, const JSON *json)
{
    const JSON *basic = json_get_member(json, "basic");
    if (!basic)
        return -1;
    const char *ip = json_obj_get_str(basic, "ip", NULL);
    if (!ip)
        return -1;
    cfg->ip = strdup(ip);
    cfg->port = json_num(json_get_member(basic, "port"), 80);
    // TODO: ...
    return 0;
}

int main(int argc, char *argv[])
{
    JSON *json = create_json();
    if (!json)
        return 1;
    int ret = json_save(json, "./test.yml");

//TODO: ...
    config_st cfg;
    config_init(&cfg);
    ret = config_load(&cfg, json);
    json_free(json);
    config_clear(&cfg);
    return ret < 0 ? 1 : 0;
}

#elif ACTIVE_PLAN == 2
//---------------------------------------------------------------------------
//  方案2
//---------------------------------------------------------------------------

int main()
{
    int ret = 0;
    JSON *json = json_new(JSON_OBJ);

    json_set(json, "basic", json_new(JSON_OBJ));
    json_set(json, "basic.ip", json_new_str("200.200.3.61"));
    json_set(json, "basic.enable", json_new_bol(true));
    json_set(json, "basic.port", json_new_int(389));
    json_set(json, "basic.timeout", json_new_int(10));
    json_set(json, "basic.dns[0]", json_new_str("200.200.0.1"));
    ret = json_set(json, "basic.dns[1]", json_new_str("200.0.0.254"));
    if (ret < 0)
        goto failed_;

    const JSON *val = json_get(json, "basic.dns");

    int port = json_int(json_get(json, "basic.port"), 80);
    bool enable = json_bool(json_get(json, "basic.enable"));
    const char *ip = json_str(json_get(json, "basic.ip"), "127.0.0.1");
    const char *dns0 = json_str(json_get(val, "[0]"), "192.168.1.1");
//...
    ret = json_save(json, "./test.yml");
    json_free(json);
    return 0;
failed_:
    json_free(json);
    return 1;
}
#else
//---------------------------------------------------------------------------
//  方案3
//---------------------------------------------------------------------------

int main()
{
    JSON *json = json_new(JSON_OBJ);
    json_set_value(json, "basic", "{}");
    json_set_value(json, "basic.ip", "\"200.200.0.1\"");
    json_set_value(json, "basic.dns", "[]");
    json_set_value(json, "basic.dns[0]", "\"200.200.0.2\"");
    json_set_value(json, "basic.enable", "true");
    JSON *advance = json_set_value(json, "advance", "{}");
    int ret = json_set_value(advance, "enable", "false");
    if (ret < 0) {
        perror("json_set_value");
        return 1;
    }
    //TODO:
    json_save(json, "./dns.json");
    json_free(json);
    return 0;
}

int main()
{
    JSON *json = json_load("./dns.json");
    if (!json) {
        perror("json_load");
        return 1;
    }
    if (json_get_bool(json, "basic.enable") == TRUE)
        sys_enable();
    int port = json_get_int(json, "basic.port", 80);
    unsigned int ip = inet_addr(json_get_str(json, "basic.ip", "127.0.0.1"));
    sys_listen(port);
    //TODO:
    json_free(json);
    return 0;
}

#endif
