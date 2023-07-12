# json_save

## 简介

把JSON值格式化为YAML格式的字符串

YAML格式定义请参考：https://baike.baidu.com/item/YAML/1067697?fr=aladdin

## 思路

```cpp
int json_save(const JSON *json, const char *fname);
```

```cpp
const char *str = ""
"house: \n"
"  family: \n"
"    name: Doe\n"
"    parents: \n"
"      - John\n"
"      - Jane\n"
"    children: \n"
"      - Paul\n"
"      - Mark\n"
"      - Simone\n"
"  address: \n"
"    number: 34\n"
"    street: Main Street\n"
"    city: Nowheretown\n"
"    zipcode: 12345\n";
```

```cpp
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
        fprintf(stderr, "open file [%s] failed, errno: %d\n"
            , fname, errno);
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
        fprintf(stderr, "malloc(%d) failed\n", len + 1);
        return -1;
    }
    buf->size = len + 1;
    realsize = fread(buf->str, 1, len, fp);
    fclose(fp);

    buf->str[realsize] = '\0';
    return 0;
}

TEST_F(json_save, str)
{
    JSON *json;
    buf_t result;
    const char *expect = "hello world";

    json = json_new_str("hello json");
    EXPECT_EQ(0, json_save(json, "test.yml"));
    EXPECT_EQ(0, read_file(&result, "test.yml"));

    ASSERT_EQ(strcmp(result.str, expect) == 0);
    free(result.str);
    json_free(json);
}

TEST_F(json_save, special_str)
{
    JSON *json;
    buf_t result;
    const char *expect = "hello\\nworld";

    json = json_new_str("hello\njson");
    EXPECT_EQ(0, json_save(json, "test.yml"));
    EXPECT_EQ(0, read_file(&result, "test.yml"));

    ASSERT_EQ(strcmp(result.str, expect) == 0);
    free(result.str);
    json_free(json);
}

TEST_F(json_save, obj)
{
    JSON *json;
    buf_t result;
    const char *expect = "key: hello\nname: world\n";

    json = json_new(JSON_OBJ);
    json_set_str(json, "key", "hello");
    json_set_str(json, "name", "world");

    EXPECT_EQ(0, json_save(json, "test-obj.yml"));
    EXPECT_EQ(0, read_file(&result, "test-obj.yml"));

    ASSERT_EQ(strcmp(result.str, expect) == 0);
    free(result.str);
    json_free(json);
}
```
