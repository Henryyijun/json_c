#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "json.h"

typedef struct array array;
typedef struct object object;
typedef struct value value;
typedef struct keyvalue keyvalue;

/**
 *  想想：这些结构体定义在.c是为什么？
 */
/**
 *  想想：如果要提升内存分配效率，这个结构体该作什么变化？
 */
struct array {
    value **elems;      /* 想想: 这里如果定义为'value *elems'会怎样？ */
    U32 count;          //elems中有多少个value*
};

/**
 * @brief 对象的键值对
 */
struct keyvalue {
    char *key;          //键名
    value *val;         //值
};

/**
 *  想想：如果要提升内存分配效率，这个结构体该作什么变化？
 */
struct object {
    keyvalue *kvs;      //这是一个keyvalue的数组，可以通过realloc的方式扩充的动态数组
    U32 count;          //数组kvs中有几个键值对
};

/**
 * @brief JSON值
 */
struct value {
    json_e type;        //JSON值的具体类型
    union {
        double num;     //数值，当type==JSON_NUM时有效
        BOOL bol;       //布尔值，当type==JSON_BOL时有效
        char *str;      //字符串值，堆中分配的一个字符串，当type==JSON_STR时有效
        array arr;      //值数组，当type==JSON_ARR时有效
        object obj;     //对象，当type==JSON_OBJ时有效
    };
};

/**
 *  @brief 新建一个type类型的JSON值，采用缺省值初始化
 *  
 *  @param [in] type JSON值的类型，见json_e的定义
 *  @return 堆分配的JSON值
 *  
 *  @details 
 *  1. 对于数值，初始化为0
 *  2. 对于BOOL，初始化为FALSE
 *  3. 对于字符串，初始化为NULL
 *  4. 对于OBJ，初始化为空对象
 *  5. 对于ARR，初始化为空数组
 */
JSON *json_new(json_e type)
{
    JSON *json = (JSON *)calloc(1, sizeof(JSON));
    if (!json) {
        //想想：为什么输出到stderr，不用printf输出到stdout？
        fprintf(stderr, "json_new: calloc(%lu) failed\n", sizeof(JSON));
        return NULL;
    }
    json->type = type;
    return json;
}
/**
 * 释放一个JSON值
 * @param json json值
 * @details
 * 该JSON值可能含子成员，也要一起释放
 */
void json_free(JSON *json)
{
    //TODO:
    //Written by Henry
    if (json->type == JSON_BOL || json->type == JSON_NUM) {
        free(json);
    } else if (json->type == JSON_STR) {
        free(json->str);
        free(json);
    } else if (json->type == JSON_ARR) {
        U32 count = json->arr.count;
        for (int i = 0; i < count; i++) {
            json_free(json->arr.elems[i]);
        } 
        free(json->arr.elems);
        free(json);
    } else if (json->type == JSON_OBJ) {
        object* obj = &(json->obj);
        U32 count = obj->count;
        keyvalue *kvs = obj->kvs;

        for (int i = 0; i < count; i++) {
            free(kvs[i].key);
            json_free(kvs[i].val);
        }
        free(kvs);
        free(json);
    } else {
        fprintf(stderr, "json_free: free(%lu) failed\n", sizeof(JSON));
    }
    return;
}
/**
 * 获取JSON值json的类型
 * @param json json值
 * @return json的实际类型
 */
json_e json_type(const JSON *json)
{
    assert(json);
    return json ? json->type : JSON_NONE;
}
/**
 * 新建一个BOOL类型的JSON值
 * @param val 新建JSON的初值
 * @return JSON* JSON值，失败返回NULL
 */
JSON *json_new_bool(BOOL val)
{
    //TODO:
    //Written by Henry
    JSON* json = json_new(JSON_BOL);
    if (!json) {
        return NULL;
    }
    json->bol = val;
    return json;
}
/**
 * 新建一个数字类型的JSON值
 * @param val 新建JSON的初值
 * @return JSON* JSON值，失败返回NULL
 */
JSON *json_new_num(double val)
{
    JSON *json = json_new(JSON_NUM);
    if (!json)
        return NULL;
    json->num = val;
    return json;
}
/**
 * 新建一个字符串类型的JSON值
 * @param str 新建JSON的初值
 * @return JSON* JSON值，失败返回NULL
 */
JSON *json_new_str(const char *str)
{
    JSON *json;
    assert(str);
    
    json = json_new(JSON_STR);
    if (!json)
        return json;
    json->str = strdup(str);
    if (!json->str) {
        fprintf(stderr, "json_new_str: strdup(%s) failed", str);
        json_free(json);
        return NULL;
    }
    return json;
}
//想想：json_num和json_str为什么带一个def参数？
/**
 * @brief 获取JSON_NUM类型JSON值的数值
 * 
 * @param json 数值类型的JSON值
 * @param def   类型不匹配时返回的缺省值
 * @return double 如果json是合法的JSON_NUM类型，返回其数值，否则返回缺省值def
 */
double json_num(const JSON *json, double def)
{
    //想想：为什么这里不assert(json)?
    return json && json->type == JSON_NUM ? json->num : def;
}
/**
 * @brief 获取JSON_BOOL类型JSON值的布尔值
 * 
 * @param json 布尔值类型的JSON值
 * @return BOOL 如果json是合法的JSON_BOL类型，返回其数值，否则返回FALSE
 */
BOOL json_bool(const JSON *json)
{
    //想想：为什么这里不assert(json)?
    return json && json->type == JSON_BOL ? json->bol : FALSE;
}
/**
 * @brief 获取JSON_STR类型JSON值的字符串值
 * 
 * @param json 字符串类型的JSON值
 * @param def   类型不匹配时返回的缺省值
 * @return const char* 如果json是合法的JSON_STR类型，返回其字符串值，否则返回def
 */
const char *json_str(const JSON *json, const char *def)
{
    //想想：为什么这里不assert(json)?
    return json && json->type == JSON_STR ? json->str : def;
}
/**
 * 从对象类型的JSON值中获取名字为key的成员(JSON值)
 * @param json 对象类型的JSON值
 * @param key  成员的键名
 * @return 找到的成员
 * @details 要求json是个对象类型
 */
const JSON *json_get_member(const JSON *json, const char *key)
{
    U32 i;
    assert(json);
    assert(json->type == JSON_OBJ);
    //assert(!(json->obj.count > 0 && json->obj.kvs == NULL));
    assert(key);
    assert(key[0]);

    for (i = 0; i < json->obj.count; ++i) {
        if (strcmp(json->obj.kvs[i].key, key) == 0)
            return json->obj.kvs[i].val;
    }
    return NULL;
}
/**
 * 从数组类型的JSON值中获取第idx个元素(子JSON值)
 * @param json 数组类型的JSON值
 * @param idx  元素的索引值
 * @return 找到的元素(JSON值的指针)
 * @details 要求json是个数组
 */
const JSON *json_get_element(const JSON *json, U32 idx)
{
    assert(json);
    assert(json->type == JSON_ARR);
    assert(!(json->arr.count > 0 && json->arr.elems == NULL));

    if (json->type != JSON_ARR)
        return NULL;
    if (idx >= json->arr.count)
        return NULL;
    return json->arr.elems[idx];
}

/**
 * @brief 把JSON值json以YAML格式输出
 * 
 * @param json  JSON值
 * @param fd 文件描述符
 * @return int 0表示成功，<0表示失败
 */
void write_json_to_yaml(const JSON* json, FILE* fd, const int indentation) {
    if (!json) {
        return;
    }
    if (json->type == JSON_NUM) {
        fprintf(fd, "%*s%f,\n", indentation* 2, "",json->num);
    } else if (json->type == JSON_STR) {
        fprintf(fd, "%*s\"%s\",\n",indentation* 2, "",json->str);
    } else if (json->type == JSON_BOL) {
        fprintf(fd, "%*s%s,\n", indentation* 2, "", json->bol ? "true" : "false");
    } else if (json->type == JSON_ARR) {
        fprintf(fd, "%*s[\n", indentation * 2, "");
        for (U32 i = 0;i < json->arr.count; i++) {
            write_json_to_yaml(json->arr.elems[i], fd, indentation + 2);
        }
        fprintf(fd, "%*s]\n",indentation * 2, "");
    } else if (json->type == JSON_OBJ) {
        fprintf(fd, "%*s{\n", indentation*2, "");
        //printf("%d\n", json->obj.count);
        for (U32 i = 0; i < json->obj.count; i++) {
            fprintf(fd, "%*s \"%s\": ", (indentation+1)*2, "", json->obj.kvs[i].key);
            write_json_to_yaml(json->obj.kvs[i].val, fd, indentation + 2);
        }
        fprintf(fd, "%*s}\n", indentation * 2, "");
    } else {
        fprintf(stderr, "write_json_to_yaml: json type error!\n");
    }
    return;

}   


/**
 * @brief 把JSON值json以YAML格式输出，保存到名字为fname的文件中
 * 
 * @param json  JSON值
 * @param fname 输出文件名
 * @return int 0表示成功，<0表示失败
 */
int json_save(const JSON *json, const char *fname)
{
    //TODO:
    if (!json) {
        fprintf(stderr, "json_save:invalid json\n");
        return -1;
    }

    if (!fname) {
        fprintf(stderr, "json_save:invalid file name\n");
        return -1;
    }
    FILE* fd = fopen(fname, "w");
    if (!fd) {
        fprintf(stderr, "json_save:open file failed \n");
        return -1;
    }
    write_json_to_yaml(json, fd, 0.1);
    return 0;
}
//  想想：json_add_member和json_add_element中，val应该是堆分配，还是栈分配？
//  想想：如果json_add_member失败，应该由谁来释放val？
/**
 * @brief 往对象类型的json中增加一个键值对，键名为key，值为val
 * 
 * @param json JSON对象
 * @param key 键名，符合正则：[a-zA-Z_][a-zA-Z_-0-9]*
 * @param val 键值，必须是堆分配拥有所有权的JSON值
 * @return JSON* 成功返回val，失败返回NULL
 * @details
 *  json_add_member会转移val的所有权，所以调用json_add_member之后不用考虑释放val的问题
 * 因为需要支持如下写法：
 *  json_add_member(json, "port", json_new_num(80));
 * 所以需要做到：
 *  1) 允许val为NULL；
 *  2) 当json_add_member内部发生失败时，需要释放val，满足将val的所有权转让给json_add_member的语义设定
 */
JSON *json_add_member(JSON *json, const char *key, JSON *val)
{
    assert(json->type == JSON_OBJ);
    assert(!(json->obj.count > 0 && json->obj.kvs == NULL));
    assert(key);
    assert(key[0]);
    // 想想: 为啥不用assert检查val？
    // 想想：如果json中已经存在名字为key的成员，怎么办？
    // TODO:
    keyvalue* kvs = json->obj.kvs;
    U32 count = json->obj.count;
        
    const JSON *j = json_get_member(json, key);
    if (j != json) {
        json->obj.kvs = (keyvalue*)realloc(kvs, sizeof(keyvalue) * (count + 1));
        if (!json->obj.kvs) {
            fprintf(stderr, "json_add_member: realloc(%s) failed!", "kvs");
            return NULL;
        }
        json->obj.kvs[count].key = strdup(key);
        // json->obj.kvs[count].val = (value*)malloc(sizeof(value));
        //memcpy(json->obj.kvs[count].val, val, sizeof(value));
        json->obj.kvs[count].val = val;
        json->obj.count++;
    }
    return json;
    //return NULL;
}
/**
 * @brief 往数组类型的json中追加一个元素
 * 
 * @param json JSON数组
 * @param val 加入到数组的元素，必须是堆分配拥有所有权的JSON值
 * @details
 *  json_add_element会转移val的所有权，所以调用json_add_element之后不用考虑释放val的问题
 */
JSON *json_add_element(JSON *json, JSON *val)
{
    assert(json);
    assert(json->type == JSON_ARR);
    assert(!(json->arr.count > 0 && json->arr.elems == NULL));

    //想想：为啥不用assert检查val？
    //TODO:
    value** elems = json->arr.elems;
    U32 count = json->arr.count;

    json->arr.elems = (value**)realloc(elems, sizeof(value*) * (count + 1));
    if (!json->arr.elems) {
        fprintf(stderr, "json_add_element: realloc(%s) failed!", "elems");
        return NULL;
    }

    json->arr.elems[count] = (value*) malloc(sizeof(value));
    memcpy(json->arr.elems[count], val, sizeof(value));
    json->arr.count++;
    return json;
}

#if ACTIVE_PLAN == 1
/**
 * 获取名字为key，类型为expect_type的子节点（JSON值）
 * @param json 对象类型的JSON值
 * @param key   键名
 * @param expect_type 期望类型
 * @return 找到的JSON值
 */
static const JSON *get_child(const JSON *json, const char *key, json_e expect_type)
{
    const JSON *child;

    child = json_get_member(json, key);
    if (!child)
        return NULL;
    if (child->type != expect_type)
        return NULL;
    return child;
}
/**
 * 获取JSON对象中键名为key的数值，如果获取不到，或者类型不对，返回def
 * @param json json对象
 * @param key  成员键名
 * @param def  取不到结果时返回的默认值
 * @return double 获取到的数值
 */
double json_obj_get_num(const JSON *json, const char *key, double def)
{
    const JSON *child = get_child(json, key, JSON_NUM);
    if (!child)
        return def;
    return child->num;
}
/**
 * 获取JSON对象中键名为key的BOOL值，如果获取不到，或者类型不对，返回false
 * @param json json对象
 * @param key  成员键名
 * @return BOOL 获取到的键值
 */
BOOL json_obj_get_bool(const JSON *json, const char *key)
{
    const JSON *child = get_child(json, key, JSON_BOL);
    if (!child)
        return FALSE;
    return child->bol;
}
/**
 * 获取JSON对象中键名为key的值，如果获取不到，则返回缺省值def
 * @param json 对象类型的JSON值
 * @param key  键名
 * @param def  找不到时返回的缺省值
 * @return 获取到的字符串结果
 * @details
 * 如果json不是对象类型，则返回def
 * 如果对应的值不是字符串类型，则返回def
 * 如: 
 *  json: {"key": "str"}
 *  json_obj_get_str(json, "key", NULL) = "str"
 *  json_obj_get_str(json, "noexist", NULL) = NULL
 *  json_obj_get_str(json, "noexist", "") = ""
 *  
 */
const char *json_obj_get_str(const JSON *json, const char *key, const char *def)
{
    const JSON *child = get_child(json, key, JSON_STR);
    if (!child)
        return def;
    return child->str;
}

int json_obj_set_num(JSON *json, const char *key, double val)
{
    const JSON *child = get_child(json, key, JSON_NUM);
    if (child && child->type == JSON_NUM) { 
        double* v = (double*)&(child->num);
        *v = val;
    }
    return val;
}

int json_obj_set_bool(JSON *json, const char *key, BOOL val)
{
    const JSON *child = get_child(json, key, JSON_BOL);
    if (child && child->type == JSON_BOL) {
        BOOL* v = (BOOL*)&(child->bol);
        *v = val;
    }
    return val;
}

int json_obj_set_str(JSON *json, const char *key, const char *val)
{
    const JSON *child = get_child(json, key, JSON_STR);
    if (child && child->type == JSON_STR) {
        char* v = (char*)&(child->str);
        v = (char*)malloc(strlen(val) + 1);
        strcpy(v, val);
    }
    return 1;
}

int json_arr_count(const JSON *json)
{
    if (!json || json->type != JSON_ARR)
        return -1;
    return json->arr.count;
}

double json_arr_get_num(const JSON *json, int idx, double def)
{
    if (!json || json->type != JSON_ARR) {
        return -1;
    }
    const JSON *j = json_get_element(json, idx);
    
    return json_num(j, def);
}

BOOL json_arr_get_bool(const JSON *json, int idx)
{
    if (!json || json->type != JSON_BOL) {
        return -1;
    }
    const JSON *j = json_get_element(json, idx);
    
    return json_bool(j);
}

const char *json_arr_get_str(const JSON *json, int idx, const char *def)
{
    if (!json || json->type != JSON_STR) {
        return NULL;
    }
    const JSON *j = json_get_element(json, idx);
    return json_str(j, def);
}

int json_arr_add_num(JSON *json, double val)
{
    if (!json || json->type != JSON_ARR) {
        return -1;
    }
    U32 count = json->arr.count;
    value** elems = json->arr.elems;
    json->arr.elems = (value**) realloc(elems, sizeof(value*) + 1);
    if (!json->arr.elems) {
        fprintf(stderr, "json_arr_add_num: realloc(%s) failed!\n", "elems");
        return -1;
    }
    json->arr.elems[count] = (value*)malloc(sizeof(value));
    json->arr.elems[count]->type = JSON_NUM;
    json->arr.elems[count]->num = val;
    json->arr.count++;

    return 1;
}

int json_arr_add_bool(JSON *json, BOOL val)
{
    if (!json || json->type != JSON_BOL) {
        return -1;
    }
    U32 count = json->arr.count;
    value** elems = json->arr.elems;
    json->arr.elems = (value**) realloc(elems, sizeof(value*) + 1);
    if (!json->arr.elems) {
        fprintf(stderr, "json_arr_add_num: realloc(%s) failed!\n", "elems");
        return -1;
    }
    json->arr.elems[count] = (value*)malloc(sizeof(value));
    json->arr.elems[count]->type = JSON_BOL;
    json->arr.elems[count]->bol = val;
    json->arr.count++;

    return 1;
}

int json_arr_add_str(JSON *json, const char *val)
{
    if (!json || json->type != JSON_ARR) {
        return -1;
    }
    U32 count = json->arr.count;
    value** elems = json->arr.elems;
    json->arr.elems = (value**) realloc(elems, sizeof(value*) + 1);
    if (!json->arr.elems) {
        fprintf(stderr, "json_arr_add_num: realloc(%s) failed!\n", "elems");
        return -1;
    }
    json->arr.elems[count] = (value*)malloc(sizeof(value));
    json->arr.elems[count]->type = JSON_STR;
    json->arr.elems[count]->str = strdup(val);
    json->arr.count++;

    return 1;
}

//#elif ACTIVE_PLAN == 2
/*
json_get和json_get所使用的路径表达式语法：

root ::= member | index;
member ::= <name> child;
index ::= '[' <number> ']' child;
child ::= dot_member | index | EOF;
dot_member ::= '.' member;
 */

/**
 * @brief 交换两个json值的内容
 * 
 * @param lhs 左手侧JSON值
 * @param rhs 右手侧JSON值
 */
static void json_swap(JSON *lhs, JSON *rhs)
{
    JSON tmp;
    memcpy(&tmp, lhs, sizeof(tmp));
    memcpy(lhs, rhs, sizeof(*lhs));
    memcpy(rhs, &tmp, sizeof(tmp));
}
/**
 * @brief 路径解析的上下文
 */
typedef struct query_ctx {
    JSON *root;         //待查找的根JSON值
    JSON *val;          //待替换的JSON值
    const char *path;   //原始路径
} query_ctx;

/**
 * @brief 处理查询结果
 * 
 * @param json  查找到的JSON值
 * @param val   需要替换的JSON值，如果val不为空，表示需要将json替换为val
 * @return const JSON* 查找结果
 */
static const JSON *deal_query_result(JSON *json, JSON *val)
{
    assert(json);

    if (val) {
        json_swap(json, val);
        json_free(val);
    }
    return json;
}
/**
 * @brief 报告路径解析过程发现的语法错误
 * 
 * @param ctx   路径解析的上下文
 * @param info  错误说明
 * @param cur   出错位置
 */
static void report_syntax_error(const query_ctx *ctx, const char *info, const char *cur)
{
    fprintf(stderr, "%s\n", info);
    fprintf(stderr, "path: %s\n", ctx->path);
    fprintf(stderr, "%*s^\n", (int)(cur - ctx->path + 6), " ");
}

static const JSON *query_child(const query_ctx *ctx, JSON *json, const char *cur);

/**
 * @brief 期待解析结束，即希望接下来的是结束符
 * 
 * @param json  JSON值
 * @param cur   当前解析位置
 * @param val   待替换的JSON值。val为NULL，表示只查询，否则将查到的子孙成员值替换为val
 * @return const JSON* 查找到的子孙成员
 */
static const JSON *query_eof(const query_ctx *ctx, JSON *json, const char *cur)
{
    assert(ctx);
    assert(json);
    assert(cur);

    if (*cur == '\0') {
        return deal_query_result(json, ctx->val);
    } else {
        report_syntax_error(ctx, "JSON path invalid", cur);
        json_free(ctx->val);
        return NULL;
    }    
}
/**
 * @brief 在JSON值json中查询MEMBER表达式cur对应的子孙成员
 * 
 * @param json  JSON值
 * @param cur  MEMBER表达式
 * @param val   待替换的JSON值。val为NULL，表示只查询，否则将查到的子孙成员值替换为val
 * @return const JSON* 查找到的子孙成员
 */
static const JSON *query_member(const query_ctx *ctx, JSON *json, const char *cur)
{
    //TODO:
    return NULL;
}
/**
 * @brief 在JSON值json中查询DOT_MEMBER表达式cur对应的子孙成员
 * 
 * @param json  JSON值
 * @param cur  DOT_MEMBER表达式
 * @param val   待替换的JSON值。val为NULL，表示只查询，否则将查到的子孙成员值替换为val
 * @return const JSON* 查找到的子孙成员
 */
static const JSON *query_dot_member(const query_ctx *ctx, JSON *json, const char *cur)
{
    assert(ctx);
    assert(json);
    assert(json->type == JSON_OBJ);
    assert(cur);

    if (*cur == '.') {
        if (cur[1] == '\0') {
            report_syntax_error(ctx, "unexpected end", cur);
            json_free(ctx->val);
            return NULL;
        }
        return query_member(ctx, json, cur + 1);
    } else {
        return query_eof(ctx, json, cur);
    }
}
/**
 * @brief 在JSON值json中查询INDEX表达式cur对应的子孙成员
 * 
 * @param json  JSON值
 * @param cur  INDEX表达式
 * @param val   待替换的JSON值。val为NULL，表示只查询，否则将查到的子孙成员值替换为val
 * @return const JSON* 查找到的子孙成员
 */
static const JSON *query_index(const query_ctx *ctx, JSON *json, const char *cur)
{
    //TODO:
    return NULL;
}
/**
 * @brief 在JSON值json中查询CHILD表达式cur对应的子孙成员
 * 
 * @param json  JSON值
 * @param cur  CHILD表达式
 * @param val   待替换的JSON值。val为NULL，表示只查询，否则将查到的子孙成员值替换为val
 * @return const JSON* 查找到的子孙成员
 */
static const JSON *query_child(const query_ctx *ctx, JSON *json, const char *cur)
{
    assert(ctx);
    assert(json);
    assert(cur);

    switch (json_type(json)) {
    case JSON_NUM: case JSON_STR: case JSON_BOL: case JSON_NONE:
        return query_eof(ctx, json, cur);
    case JSON_ARR:
        return query_index(ctx, json, cur);
    case JSON_OBJ: 
        return query_dot_member(ctx, json, cur);
    default:
        assert(!"dead code");
        return NULL;
    }
}

/**
 * @brief 在JSON值json中查询ROOT表达式cur对应的子孙成员
 * 
 * @param json  JSON值
 * @param cur  ROOT表达式
 * @param val   待替换的JSON值。val为NULL，表示只查询，否则将查到的子孙成员值替换为val
 * @return const JSON* 查找到的子孙成员
 */
static const JSON *query_root(JSON *json, const char *path, JSON *val)
{
    query_ctx ctx = {0};

    assert(json);
    assert(path);

    ctx.root = json;
    ctx.path = path;
    ctx.val = val;

    if (json_type(json) == JSON_OBJ) {
        return query_member(&ctx, json, path);
    } else {    
        return query_child(&ctx, json, path);
    }
}
/**
 * 在JSON值json中找到路径为path的成员，将其值修改为val
 * @param json JSON值
 * @param path 待修改成员的路径，如：basic.dns[1]，空串表示本身
 * @param val 新的值
 * @return <0表示失败，否则表示成功
 */
int json_set(JSON *json, const char *path, JSON *val)
{
    assert(json);
    assert(path);

    if (!val)
        return -1;
    if (query_root(json, path, val))
        return 0;
    return -1;
}
/**
 * 在JSON值json中找到路径为path的成员
 * @param json JSON值
 * @param path 路径表达式，待查找成员的路径，如：basic.dns[1]，空串表示本身
 * @return 路径path指示的成员值，不存在则返回NULL
 */
const JSON *json_get(const JSON *json, const char *path)
{
    assert(json);
    assert(path);

    return query_root((JSON *)json, path, NULL);
}
//#elif ACTIVE_PLAN == 3
/**
 * @brief 设置json成员的值
 * 
 * @param json JSON值
 * @param path 待操作的子成员在json中的位置，如：basic.dns[0]。NULL表示json本身
 * @param value 子成员的新值，以字符串形式表示，五种可能，字符串："200.200.0.1"，数值：8080，BOOL值：true/false，空数组：[], 空对象：{}
 * @return JSON* path指向的子成员JSON值
 * @details
 *  如果path表示的子成员不存在，将自动创建，如果存在，将替换成新值。
 *  两种情况下不会自动创建，第一种是父对象不存在，第二种是数组成员的前一个兄弟不存在。
 */
JSON *json_set_value(JSON *json, const char *path, const char *value)
{
    assert(json);
    assert(value);
    //TODO:
    return NULL;
}
/**
 * @brief 从JSON值json中获取一个子成员的值，子成员所在位置由路径path标识
 * 
 * @param json JSON值
 * @param path 待操作的子成员在json中的位置，如：basic.dns[0]。NULL表示json本身
 * @return const JSON* path指向的子成员
 */
const JSON *json_get_value(const JSON *json, const char *path)
{
    //TODO:
    return NULL;
}
/**
 * @brief 从JSON值json中读取一个INT类型配置项的值，配置项的位置由路径path标识
 * 
 * @param json JSON值
 * @param path 待操作的子成员在json中的位置，如：basic.dns[0]。NULL表示json本身
 * @param def 如果配置项不存在或类型不匹配，返回该值作为缺省值
 * @return int 配置项的值
 */
int json_get_int(const JSON *json, const char *path, int def)
{
    const JSON *child = json_get_value(json, path);
    if (!child)
        return def;
    return (int)json_num(child, def);
}
/**
 * @brief 从JSON值json中读取一个BOOL类型配置项的值，配置项的位置由路径path标识
 * 
 * @param json JSON值
 * @param path 待操作的子成员在json中的位置，如：basic.dns[0].enable。NULL表示json本身
 * @return BOOL 配置项的值
 * @details 如果配置项不存在或类型不匹配，返回FALSE，当作不启用的意思
 */
BOOL json_get_bool(const JSON *json, const char *path)
{
    const JSON *child = json_get_value(json, path);
    if (!child)
        return FALSE;
    return json_bool(child);
}
/**
 * @brief 从JSON值json中读取一个字符串类型配置项的值，配置项的位置由路径path标识
 * 
 * @param json JSON值
 * @param path 待操作的子成员在json中的位置，如：basic.dns[0].ip。NULL表示json本身
 * @return const char* 如果配置项不存在或类型不匹配，返回该值作为缺省值
 */
const char *json_get_str(const JSON *json, const char *path, const char *def)
{
    const JSON *child = json_get_value(json, path);
    if (!child)
        return FALSE;
    return json_str(child, def);
}

#endif //ACTIVE_PLAN

