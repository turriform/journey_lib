#include "jrn_test_set.h"
#include "journey.h"
#include <execinfo.h>
#include <stdio.h>

typedef struct TestStruct
{
	size_t one;
	size_t two;
} test_struct_t;

void test_from_other_fnc(void)
{
	jrn_debug("This is the debug call test_from_other_fnc");
}

void test_loop_init(void)
{

	char buf[2] = {0};

	while (buf[0] != 'q')
	{
		fgets(buf, sizeof(buf), stdin);

		jrn_debug("Debug call");
	}
}

void test_error(void)
{
	jrn_init("test", "log", LOG_LEVEL_DEBUG, LOG_FILE_JSON, true);

	// fail malloc
	char *string = malloc(1000000000000000000);

	// jrn_internal_status will force exit and call jrn_destory() at this point;
	// following code is unreachable;

	free(string);
	jrn_destroy();
}

void test_double_free_json(void)
{
	jrn_init("test", "log", LOG_LEVEL_DEBUG, LOG_FILE_JSON, true);

	// fail malloc
	char *string = malloc(20);

	free(string);
	// warning triggered
	free(string);
	jrn_destroy();
}

void test_double_free_html(void)
{
	jrn_init("test", "log", LOG_LEVEL_DEBUG, LOG_FILE_HTML, true);

	// fail malloc
	char *string = malloc(20);

	free(string);
	// warning triggered
	free(string);
	jrn_destroy();
}

void test_init(void)
{

	jrn_init("test", "log", LOG_LEVEL_DEBUG, LOG_FILE_JSON, true);

	// TODO: create record and write
	jrn_debug("This is the debug call test_init");

	char *string = malloc(30);

	size_t ts_arr_sz = 100;

	test_struct_t **ts_arr = calloc(ts_arr_sz, sizeof(**ts_arr));

	free(string);
	// double free
	free(string);
	for (size_t i = 0; i != 100; i++)
	{
		jrn_info("Allocating test_struct_t");
		test_struct_t *ts = malloc(sizeof(test_struct_t));
		free(ts);
	}

	for (size_t i = 0; i != 100; i++)
	{
		jrn_info("Allocating test_struct_t 2");
		test_struct_t *ts = malloc(sizeof(test_struct_t));
		ts_arr[i] = ts;
	}

	for (size_t i = 0; i != 100; i++)
	{
		jrn_info("Freeing test_struct_t 2");
		free(ts_arr[i]);
	}

	free(ts_arr);

	jrn_destroy();
}

void run_main_tests(void)
{
	// test_init();
	test_double_free_html();
	test_double_free_json();
	test_error();
}
