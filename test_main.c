#include "json.h"
#include "xtest.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>

//  完成使用场景的测试
TEST(test, scene)
{
    JSON *json = json_new(JSON_OBJ);
    ASSERT_TRUE(json != NULL);
    JSON *basic = json_new(JSON_OBJ);
    ASSERT_TRUE(basic != NULL);

    ASSERT_TRUE(NULL != json_add_member(json, "basic", basic));

    ASSERT_TRUE(NULL != json_add_member(basic, "enable", json_new_bool(TRUE)));
    EXPECT_EQ(TRUE, json_obj_get_bool(basic, "enable"));

    ASSERT_TRUE(NULL != json_add_member(basic, "port", json_new_num(389)));
    EXPECT_EQ(389, json_obj_get_num(basic, "port", 0));
//...
    ASSERT_TRUE(NULL != json_add_member(basic, "ip", json_new_str("200.200.3.61")));
    const char *ip = json_obj_get_str(basic, "ip", NULL);
    ASSERT_STRCASEEQ("200.200.3.61", ip);

    json_free(json);
}

//  测试键值对存在的情况
TEST(json_obj_get_str, exist)
{
    JSON *json = json_new(JSON_OBJ);
    ASSERT_TRUE(json != NULL);

    ASSERT_TRUE(NULL != json_add_member(json, "ip", json_new_str("200.200.3.61")));
    const char *ip = json_obj_get_str(json, "ip", NULL);
    ASSERT_TRUE(ip != NULL);
    ASSERT_STRCASEEQ("200.200.3.61", ip);

    json_free(json);
}

//  测试键值对不存在的情况
TEST(json_obj_get_str, notexist)
{
    JSON *json = json_new(JSON_OBJ);
    ASSERT_TRUE(json != NULL);

    ASSERT_TRUE(NULL != json_add_member(json, "ip", json_new_str("200.200.3.61")));
    const char *ip = json_obj_get_str(json, "ip2", NULL);
    ASSERT_TRUE(ip == NULL);

    ip = json_obj_get_str(json, "ip3", "default");
    ASSERT_TRUE(ip != NULL);
    ASSERT_STRCASEEQ("default", ip);

    json_free(json);
}

TEST(json_get, self)
{
    JSON *json = json_new_num(20);
    const JSON *ret;

    ASSERT_TRUE(json != NULL);

    ret = json_get(json, "");
    ASSERT_TRUE(json == ret);

    ret = json_get(json, "hello");
    ASSERT_TRUE(ret == NULL);

    json_free(json);
}
//----------------------------------------------------------------------------------------------------
//  json_save
//----------------------------------------------------------------------------------------------------

typedef struct buf_t {
    char *str;
    unsigned int size;
} buf_t;

int read_file(buf_t *buf, const char *fname)
{
    FILE *fp;
    long len;
    long realsize;

    assert(buf);
    assert(fname);
    assert(fname[0]);

    fp = fopen(fname, "rb");
    if (!fp) {
        fprintf(stderr, "open file [%s] failed\n", fname);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    if (len <= 0) {
        fclose(fp);
        fprintf(stderr, "ftell failed, errno: %d\n", errno);
        return -1;
    }
    fseek(fp, 0, SEEK_SET);
    buf->str = (char *)malloc(len + 1);
    if (!buf->str) {
        fclose(fp);
        buf->size = 0;
        fprintf(stderr, "malloc(%ld) failed\n", len + 1);
        return -1;
    }
    buf->size = len + 1;
    realsize = fread(buf->str, 1, len, fp);
    fclose(fp);

    buf->str[realsize] = '\0';
    return 0;
}

TEST(json_save, str)
{
    JSON *json;
    buf_t result;
    const char *expect = "hello world";

    json = json_new_str("hello json");
    EXPECT_EQ(0, json_save(json, "test.yml"));
    EXPECT_EQ(0, read_file(&result, "test.yml"));

    ASSERT_TRUE(strcmp(result.str, expect) == 0);
    free(result.str);
    json_free(json);
}

TEST(json_save, special_str)
{
    JSON *json;
    buf_t result;
    const char *expect = "hello\\nworld";

    json = json_new_str("hello\njson");
    EXPECT_EQ(0, json_save(json, "test.yml"));
    EXPECT_EQ(0, read_file(&result, "test.yml"));

    ASSERT_TRUE(strcmp(result.str, expect) == 0);
    free(result.str);
    json_free(json);
}

TEST(json_save, obj)
{

    buf_t result;
    const char *expect = "key: hello\nname: world\n";
    JSON* j = json_new(JSON_OBJ);

    JSON* json = json_new(JSON_OBJ);
    json_add_member(j, "basic", json);
    json_add_member(json, "key", json_new_str("hello"));
    json_add_member(json, "name", json_new_str("world"));

    
    EXPECT_EQ(0, json_save(j, "test-obj.yml"));
    EXPECT_EQ(0, read_file(&result, "test-obj.yml"));

    ASSERT_TRUE(strcmp(result.str, expect) == 0);
    free(result.str);
    json_free(json);
}

int main(int argc, char **argv)
{
	return xtest_start_test(argc, argv);
}

