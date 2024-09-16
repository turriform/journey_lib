
#include <stdio.h>
#include <execinfo.h>
#include "jrn_internal.h"
#include "jrn_record.h"
#include "jrn_memory.h"

/* LOG FUNCTIONS */
extern i_jrn_settings_t i_jrn_settings;

extern jrn_folder_t *jrn_folder_init_from_settings(void);
extern void *jrn_folder_get_file_handler(void);

extern void i_jrn_terminal_create_entry(jrn_record_t *record);

void jrn_log(jrn_level_t level_type, const char *__file__, size_t __line__, ...)
{

    if (level_type < i_jrn_settings.get_level())
    {
        jrn_internal_signal(JRN_OK, "Level is set higher, not recording");
        return;
    }

    char message[JRN_BUF] = "";
    va_list v;
    va_start(v, __line__);

    char *next;
    do
    {
        next = va_arg(v, char *);
        if (next)
        {
            strncat(message, next, strlen(next));
            strncat(message, " ", JRN_BUF_16);
        }
    } while (next);
    va_end(v);

    /* START RECORD*/

    /* adding stacktrace if aplicable*/

    char **stacktrace = NULL;

    // for portability calling i_jrn_settings here
    uint8_t backtrace_limit = i_jrn_settings.get_backtrace_limit();
    jrn_level_t backtrace_level_on = i_jrn_settings.get_backtrace_level_on();

    if (level_type >= backtrace_level_on)
    {
        void *array[backtrace_limit];
        int size;

        size = backtrace(array, backtrace_limit);
        stacktrace = backtrace_symbols(array, size);
    }
    // end stacktrace assigment

    // seeting record

    char time_buf[JRN_BUF_64] = "";
    jrn_get_current_time(time_buf);

    char user_buf[JRN_BUF_256] = "";
    jrn_get_current_user(user_buf);

    char line_buf[JRN_BUF_16] = "";
    jrn_itoa_u(line_buf, JRN_BUF_16, __line__, 10);

    jrn_record_t record = {
        .message = message,
        .debug_level = level_type,
        .time = time_buf,
        .__line__ = line_buf,
        .__file__ = __file__,
        .user = user_buf,
        .stacktrace = stacktrace,
        .stacktrace_sz = backtrace_limit};

    // send record to the file template handler

    /* specific file format handeling*/

    void (*filehandler)(jrn_record_t *record);

    *(void **)(&filehandler) = jrn_folder_get_file_handler();

    filehandler(&record);

    // streaming

    if (i_jrn_settings.get_is_streaming())
    {
        i_jrn_terminal_create_entry(&record);
    }

    // cleanup

    if (stacktrace != NULL)
    {
        free(stacktrace);
    }

    if (level_type == LOG_LEVEL_ERROR){
        jrn_internal_signal(JRN_ERR, "Exiting on error");
    }
}

/* MEMORY FUNCTIONS */

void *jrn_malloc(size_t size, const char *__file__, size_t __line__)
{
    void *obj_ptr = NULL;

    JRN_MUTEX_LOCK;

    obj_ptr = malloc(size);

    JRN_MUTEX_UNLOCK;

    if (obj_ptr == NULL)
    {
        jrn_log(LOG_LEVEL_ERROR, __file__, __line__, "malloc error", NULL);

    }

    jrn_log(LOG_LEVEL_INFO, __file__, __line__, "Malloc ok", NULL);

    // writing memory tick

    jrn_memory_t *mem = i_jrn_settings.get_memory();

    jrn_memory_append_tick(mem, 1, 0);

    return obj_ptr;
}
void *jrn_calloc(size_t nmem, size_t size, const char *__file__, size_t __line__)
{
    void *obj_ptr = NULL;

    JRN_MUTEX_LOCK;

    obj_ptr = calloc(nmem, size);

    JRN_MUTEX_UNLOCK;

    if (obj_ptr == NULL)
    {
        jrn_log(LOG_LEVEL_ERROR, __file__, __line__, "Calloc error", NULL);
    }

    jrn_log(LOG_LEVEL_INFO, __file__, __line__, "Calloc ok", NULL);

    // writing memory tick

    jrn_memory_t *mem = i_jrn_settings.get_memory();

    jrn_memory_append_tick(mem, 1, 0);

    return obj_ptr;
}
void *jrn_realloc(void *ptr, size_t size, size_t obj_size, const char *__file__, size_t __line__)
{

    if (ptr == NULL)
    {
        jrn_log(LOG_LEVEL_WARNING, __file__, __line__, "NULL realloc attempt... return from realloc()", NULL);
        return ptr;
    }

    JRN_MUTEX_LOCK;

    void *new_ptr = realloc(ptr, size);

    if (new_ptr == NULL)
    {
        jrn_log(LOG_LEVEL_WARNING, __file__, __line__, "Realloc returned null... returning prev pointer()", NULL);
        return ptr;
    }

    JRN_MUTEX_UNLOCK;

    jrn_log(LOG_LEVEL_INFO, __file__, __line__, "Realloc ok", NULL);

    return new_ptr;
}

void jrn_free(void **ptr, const char *__file__, size_t __line__)
{

    if (*ptr == NULL)
    {
        jrn_log(LOG_LEVEL_WARNING, __file__, __line__, "NULL free attempt... return from free()", NULL);
        return;
    }

    JRN_MUTEX_LOCK;

    free(*ptr);

    *ptr = NULL;

    JRN_MUTEX_UNLOCK;

    jrn_memory_t *mem = i_jrn_settings.get_memory();

    jrn_memory_append_tick(mem, 0, 1);

    jrn_log(LOG_LEVEL_INFO, __file__, __line__, "Freed", NULL);
}