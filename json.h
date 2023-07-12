#ifndef JSON_H_
#define JSON_H_

/**
 *  想想：
 *  1. 你的JSON接口是为什么场景设计的？
 *  2. 这些场景有什么特点，会怎么影响的API设计风格？
 *  3. 你的用户怎么用这些API构建程序？
 *  4. 怎么设计API，会让用户用起来更爽？
 *  5. 怎么设计API，会让这些API更容易调试、测试？
 */
typedef enum json_e {
    JSON_NONE,
    JSON_BOL,           //BOOL类型
    JSON_NUM,           //数值类型
    JSON_STR,           //字符串类型
    JSON_ARR,           //数组类型
    JSON_OBJ,           //对象类型
} json_e;

typedef unsigned int BOOL;
typedef unsigned int U32;
typedef struct value JSON;

#define TRUE 1
#define FALSE 0

// 启用第几套方案
#define ACTIVE_PLAN 1

JSON *json_new(json_e type);
json_e json_type(const JSON *json);
void json_free(JSON *json);

int json_save(const JSON *json, const char *fname);

double json_num(const JSON *json, double def);
BOOL json_bool(const JSON *json);
const char *json_str(const JSON *json, const char *def);

JSON *json_new_num(double val);
JSON *json_new_bool(BOOL val);
JSON *json_new_str(const char *str);

const JSON *json_get_member(const JSON *json, const char *key);
const JSON *json_get_element(const JSON *json, U32 idx);

JSON *json_add_member(JSON *json, const char *key, JSON *val);
JSON *json_add_element(JSON *json, JSON *val);
/*
在完成API的设计初稿的时候，要写个demo，验证API设计OK，并找到API实现当中需要注意的问题。
比如下述代码，如果要这样写，对json_new，json_add_member有什么要求？怎么保证内存不会泄漏？不出错？
JSON *json = json_new(JSON_OBJ);
JSON *basic = json_new(JSON_OBJ);
JSON *dns = json_new(JSON_ARR);
if (!json || !basic || !dns) {
    json_free(json);
    json_free(basic);
    json_free(dns);
    return -1;
}
json_add_member(json, "basic", basic);
json_add_member(json, "dns", dns);
json_add_element(dns, json_new_str("200.200.2.254"))；
json_add_element(dns, json_new_str("192.168.1.1"));
json_add_member(basic, "enable", json_new_bool(true));
json_free(json);
*/
//-----------------------------------------------------------------------------
//  以下三种方案可任选一种，补充完善
//  也可自行设计其他方案实现
//-----------------------------------------------------------------------------
#if ACTIVE_PLAN == 1
//-----------------------------------------------------------------------------
//  方案1
//-----------------------------------------------------------------------------
double json_obj_get_num(const JSON *json, const char *key, double def);
BOOL json_obj_get_bool(const JSON *json, const char *key);
const char *json_obj_get_str(const JSON *json, const char *key, const char *def);

int json_arr_count(const JSON *json); 
double json_arr_get_num(const JSON *json, int idx, double def);
BOOL json_arr_get_bool(const JSON *json, int idx);
const char *json_arr_get_str(const JSON *json, int idx, const char *def); 

int json_obj_set_num(JSON *json, const char *key, double val);
int json_obj_set_bool(JSON *json, const char *key, BOOL val);
int json_obj_set_str(JSON *json, const char *key, const char *val);

int json_arr_add_num(JSON *json, double val);
int json_arr_add_bool(JSON *json, BOOL val);
int json_arr_add_str(JSON *json, const char *val);

/*
JSON *json = json_new(JSON_OBJ);
JSON *basic = json_new(JSON_OBJ);
JSON *dns = json_new(JSON_ARR);
if (!json || !basic || !dns) {
    json_free(json);
    json_free(basic);
    json_free(dns);
    return -1;
}
json_add_member(json, "basic", basic);
json_add_member(json, "advance", json_new(JSON_OBJ));
json_obj_set_bool(basic, "enable", true);
json_obj_set_str(basic, "ip", "200.200.3.2");
json_add_member(basic, "dns", dns);

json_arr_add_str(dns, "200.200.3.254");
json_arr_add_str(dns, "200.200.1.1");

 */
//#elif ACTIVE_PLAN == 2
//-----------------------------------------------------------------------------
//  方案2
//-----------------------------------------------------------------------------
int json_set(JSON *json, const char *path, JSON *val);
const JSON *json_get(const JSON *json, const char *path);
/*
JSON *json = json_new(JSON_OBJ);

json_set(json, "basic", json_new(JSON_OBJ));
json_set(json, "basic.enable", json_new_bool(true));
json_set(json, "basic.dns", json_new(JSON_ARR));
json_set(json, "basic.dns[0]", json_new_str("192.168.1.1"));

if (json_bool(json_get(json, "basic.enable")) == TRUE)
    sys_enable();
unsigned int ip = inet_addr(json_str(json_get(json, "basic.ip"), "127.0.0.1"));
...
*/
//-----------------------------------------------------------------------------
//  方案3
//-----------------------------------------------------------------------------
JSON *json_set_value(JSON *json, const char *path, const char *value);
const JSON *json_get_value(const JSON *json, const char *path);
int json_get_int(const JSON *json, const char *path, int def);
BOOL json_get_bool(const JSON *json, const char *path);
const char *json_get_str(const JSON *json, const char *path, const char *def);

/*
JSON *json = json_new(JSON_OBJ);
json_set_value(json, "basic", "{}");
json_set_value(json, "basic.ip", "\"200.200.0.1\"");
json_set_value(json, "basic.dns", "[]");
json_set_value(json, "basic.dns[0]", "\"200.200.0.2\"");
json_set_value(json, "basic.enable", "true");
JSON *advance = json_set_value(json, "advance", "{}");
json_set_value(advance, "enable", "false");

if (json_get_bool(json, "basic.enable") == TRUE)
    sys_enable();
int port = json_get_int(json, "basic.port", 80);
unsigned int ip = inet_addr(json_get_str(json, "basic.ip", "127.0.0.1"));
sys_listen(port);
...
json_free(json);
*/

#endif //ACTIVE_PLAN

//-----------------------------------------------------------------------------
//  TODO: 增加你认为还应该增加的接口
//-----------------------------------------------------------------------------


#endif

