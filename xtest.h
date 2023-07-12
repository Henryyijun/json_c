#ifndef __XTEST_H__
#define __XTEST_H__

#include <limits.h>
#include <string.h>
#include <stdlib.h>

typedef void (*xtest_entry_func_t)(void);

#ifndef __cplusplus

/* GCC */

/**
 * 定义单元测试案例
 * \code
TEST(catagory, name)
{
	printf("this is test 1\n");
}
 * \endcode
 * \param catagory 案例类别
 * \param name 案例名称
 */
#define TEST(catagory, name) 										\
static void _X_TEST_##catagory##_##name##_FUNC(void);				\
__attribute__((constructor)) 										\
static void _X_TEST_##catagory##_##name##_CONSTRUCT(void) {			\
	xtest_register(#catagory, #name, __FILE__, __LINE__,			\
				   NULL, _X_TEST_##catagory##_##name##_FUNC, NULL);	\
}																	\
void _X_TEST_##catagory##_##name##_FUNC(void)

/**
 * 定义单元测试案例
 * \code
TEST(catagory, name, init, fini)
{
	printf("this is test 1\n");
}
 * \endcode
 * \param catagory 案例类别
 * \param name 案例名称
 * \param init 初始化函数
 * \param fini 清理函数
 */
#define TEST_F(catagory, name, init, fini) 							\
static void _X_TEST_##catagory##_##name##_FUNC(void);				\
__attribute__((constructor)) 										\
static void _X_TEST_##catagory##_##name##_CONSTRUCT(void) {			\
	xtest_register(#catagory, #name, __FILE__, __LINE__,			\
				   init, _X_TEST_##catagory##_##name##_FUNC, fini);	\
}																	\
void _X_TEST_##catagory##_##name##_FUNC(void)

#else

/* C++版本的TEST和TEST_F */

#define TEST(catagory, name)										\
static void _X_TEST_##catagory##_##name##_FUNC(void);				\
class _X_TEST_CLASS_##catagory##_##name {							\
	public: _X_TEST_CLASS_##catagory##_##name(void) {				\
		xtest_register(#catagory, #name, __FILE__, __LINE__,		\
					NULL, _X_TEST_##catagory##_##name##_FUNC, NULL);\
	}																\
} _X_TEST_global_object_##catagory##_##name; 						\
void _X_TEST_##catagory##_##name##_FUNC(void)


#define TEST_F(catagory, name, init, fini) 							\
static void _X_TEST_##catagory##_##name##_FUNC(void);				\
class _X_TEST_CLASS_##catagory##_##name {							\
	public: _X_TEST_CLASS_##catagory##_##name(void) {				\
		xtest_register(#catagory, #name, __FILE__, __LINE__,		\
				   init, _X_TEST_##catagory##_##name##_FUNC, fini);	\
	}																\
} _X_TEST_global_object_##catagory##_##name; 						\
void _X_TEST_##catagory##_##name##_FUNC(void)

#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 打印红色命令行日志
 */
#define LOG_RED(format, args...) \
	do { printf("\033[31m"format"\033[0m", ##args); }while(0)

/**
 * 打印绿色命令行日志
 */
#define LOG_GREEN(format, args...) \
	do { printf("\033[32m"format"\033[0m", ##args); }while(0)

void xtest_fail_message(const char *file, int lineno, const char *fmt, ...);
void xtest_exit(int code);
int xtest_start_test(int argc, char **argv);
void xtest_register(const char *catagory, const char *name, 
					const char *file, int lineno,
					xtest_entry_func_t init, 
					xtest_entry_func_t entry, 
					xtest_entry_func_t fini);


/* EXPECT_* */

#define EXPECT_TRUE(expr) do {										\
	if ((expr) == 0) {												\
		xtest_fail_message(__FILE__, __LINE__,						\
						   "%s is not true",						\
						   #expr);									\
	}																\
} while(0)

#define EXPECT_FALSE(expr) do {										\
	if ((expr) != 0) {												\
		xtest_fail_message(__FILE__, __LINE__,						\
						   "%s is not false",						\
						   #expr);									\
	}																\
} while(0)

#define EXPECT_EQ(value, expr) do {									\
	if ((expr) != (value)) {										\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   "%s is not equal to %s", 				\
						   #expr, #value);							\
	} 																\
} while(0)

#define EXPECT_NE(value, expr) do {									\
	if ((expr) == (value)) {										\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   "%s is equal to %s", 					\
						   #expr, #value);							\
	} 																\
} while(0)

#define EXPECT_LT(val1, val2) do {									\
	if ((val1) >= (val2)) {											\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   "%s is not less then %s", 				\
						   #val1, #val2);							\
	} 																\
} while(0)

#define EXPECT_LE(val1, val2) do {									\
	if ((val1) > (val2)) {											\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   "%s is not less or equal then %s", 		\
						   #val1, #val2);							\
	} 																\
} while(0)

#define EXPECT_GT(val1, val2) do {									\
	if ((val1) <= (val2)) {											\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   "%s is not greater then %s",		 		\
						   #val1, #val2);							\
	} 																\
} while(0)

#define EXPECT_GE(val1, val2) do {									\
	if ((val1) < (val2)) {											\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   "%s is not greater or equal then %s",	\
						   #val1, #val2);							\
	} 																\
} while(0)

#define EXPECT_STREQ(val1, val2) do {								\
	if (strcmp((val1), (val2)) != 0) {								\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   "%s is not equal to %s",					\
						   #val1, #val2);							\
	} 																\
} while(0)

#define EXPECT_STRNE(val1, val2) do {								\
	if (strcmp((val1), (val2)) == 0) {								\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   "%s is equal to %s",						\
						   #val1, #val2);							\
	} 																\
} while(0)

#define EXPECT_STRCASEEQ(val1, val2) do {							\
	if (strcasecmp((val1), (val2)) != 0) {							\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   "%s is not case equal to %s",			\
						   #val1, #val2);							\
	} 																\
} while(0)

#define EXPECT_STRCASENE(val1, val2) do {							\
	if (strcasecmp((val1), (val2)) == 0) {							\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   "%s is case equal to %s",				\
						   #val1, #val2);							\
	} 																\
} while(0)

#define EPSILON		(0.00001f)

#define EXPECT_FLOAT_EQ(value, expr) do {							\
	if (fabs((expr) - (value)) > EPSILON) {							\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   "%s is not equal to %s", 				\
						   #expr, #value);							\
	} 																\
} while(0)

#define EXPECT_DOUBLE_EQ EXPECT_FLOAT_EQ





/* ASSERT_* */

#define ASSERT_PRFX			"Assertion fail: "
#define ASSERT_TRUE(expr) do {										\
	if ((expr) == 0) {												\
		xtest_fail_message(__FILE__, __LINE__,						\
						   ASSERT_PRFX"%s is not true",				\
						   #expr);									\
		xtest_exit(0);												\
	}																\
} while(0)

#define ASSERT_FALSE(expr) do {										\
	if ((expr) != 0) {												\
		xtest_fail_message(__FILE__, __LINE__,						\
						   ASSERT_PRFX"%s is not false",			\
						   #expr);									\
		xtest_exit(0);												\
	}																\
} while(0)


#define ASSERT_EQ(value, expr) do {									\
	if ((expr) != (value)) {										\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   ASSERT_PRFX"%s is not equal to %s", 		\
						   #expr, #value);							\
		xtest_exit(0);												\
	} 																\
} while(0)

#define ASSERT_NE(value, expr) do {									\
	if ((expr) == (value)) {										\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   ASSERT_PRFX"%s is equal to %s", 			\
						   #expr, #value);							\
		xtest_exit(0);												\
	} 																\
} while(0)

#define ASSERT_LT(val1, val2) do {									\
	if ((val1) >= (val2)) {											\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   ASSERT_PRFX"%s is not less then %s", 	\
						   #val1, #val2);							\
		xtest_exit(0);												\
	} 																\
} while(0)

#define ASSERT_LE(val1, val2) do {											\
	if ((val1) > (val2)) {													\
		xtest_fail_message(__FILE__, __LINE__, 								\
						   ASSERT_PRFX"%s is not less or equal then %s", 	\
						   #val1, #val2);									\
		xtest_exit(0);														\
	} 																		\
} while(0)

#define ASSERT_GT(val1, val2) do {									\
	if ((val1) <= (val2)) {											\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   ASSERT_PRFX"%s is not greater then %s",	\
						   #val1, #val2);							\
		xtest_exit(0);												\
	} 																\
} while(0)

#define ASSERT_GE(val1, val2) do {											\
	if ((val1) < (val2)) {													\
		xtest_fail_message(__FILE__, __LINE__, 								\
						   ASSERT_PRFX"%s is not greater or equal then %s",	\
						   #val1, #val2);									\
		xtest_exit(0);														\
	} 																		\
} while(0)

#define ASSERT_STREQ(val1, val2) do {								\
	if (strcmp((val1), (val2)) != 0) {								\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   ASSERT_PRFX"%s is not equal to %s",		\
						   #val1, #val2);							\
		xtest_exit(0);												\
	} 																\
} while(0)

#define ASSERT_STRNE(val1, val2) do {								\
	if (strcmp((val1), (val2)) == 0) {								\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   ASSERT_PRFX"%s is equal to %s",			\
						   #val1, #val2);							\
		xtest_exit(0);												\
	} 																\
} while(0)

#define ASSERT_STRCASEEQ(val1, val2) do {							\
	if (strcasecmp((val1), (val2)) != 0) {							\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   ASSERT_PRFX"%s is not case equal to %s",	\
						   #val1, #val2);							\
		xtest_exit(0);												\
	} 																\
} while(0)

#define ASSERT_STRCASENE(val1, val2) do {							\
	if (strcasecmp((val1), (val2)) == 0) {							\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   ASSERT_PRFX"%s is case equal to %s",		\
						   #val1, #val2);							\
		xtest_exit(0);												\
	} 																\
} while(0)

#define EPSILON		(0.00001f)

#define ASSERT_FLOAT_EQ(value, expr) do {							\
	if (fabs((expr) - (value)) > EPSILON) {							\
		xtest_fail_message(__FILE__, __LINE__, 						\
						   ASSERT_PRFX"%s is not equal to %s", 		\
						   #expr, #value);							\
		xtest_exit(0);												\
	} 																\
} while(0)

#define ASSERT_DOUBLE_EQ ASSERT_FLOAT_EQ

#ifdef __cplusplus
}
#endif

#endif
