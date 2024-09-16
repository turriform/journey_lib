
#include <stdio.h>
#include "jrn_internal.h"
#include "jrn_record.h"
#include "jrn_memory.h"

#define JRN_INTERNAL_REPORT

extern jrn_folder_t *jrn_folder_init_from_settings(void);
extern void *jrn_folder_get_file_handler(void);

struct JrnSettings
{
    bool                            is_set;
    jrn_file_type_t                 filetype;       //
    jrn_level_t                     level;              //
    jrn_level_t                     backtrace_level_on; //
    jrn_level_t                     memory_on_at;       //
    uint32_t                        max_file_size;

    uint8_t                         backtrace_limit; //
    bool                            is_streaming;       //
    bool                            is_recording_mutex; //
    bool                            is_ready_to_write;  //

    char                            logfolder_name[JRN_BUF_MAX_FILENAME]; //
    char                            basefile_name[JRN_BUF_MAX_FILENAME];  //

    void *                          mutex;

    jrn_memory_t *                  memory;
};

jrn_settings_t jrn_settings = {
    .is_set                 = false,

    .filetype               = JRN_DEFAULT_FILETYPE,
    .level                  = JRN_DEFAULT_LEVEL,
    .backtrace_level_on     = JRN_DEFAULT_BACKTRACE_LEVEL_ON,
    .memory_on_at           = JRN_DEFAULT_MEMORY_ON_AT,
    .max_file_size          = JRN_DEFAULT_MAX_FILE_SIZE,
    .backtrace_limit        = JRN_DEFAULT_BACKTRACE_LIMIT,
    .is_streaming           = JRN_DEFAULT_IS_STREAMING,
    .is_recording_mutex     = JRN_DEFAULT_IS_RECORDING_MUTEX,
    .logfolder_name         = JRN_DEFAULT_LOGFOLDER_NAME,
    .basefile_name          = JRN_DEFAULT_BASEFILE_NAME,
    .is_ready_to_write      = JRN_DEFAULT_IS_READY_TO_WRITE,
    .mutex                  = NULL,
    .memory                 = NULL,

};

// Setting an interface, simple getters for variouse uses
/* START JRN SETTINGS IFACE DEF*/
jrn_file_type_t             jrn_get_filetype(void)                          { return jrn_settings.filetype; }
jrn_level_t                 jrn_get_level(void)                             { return jrn_settings.level; }
jrn_level_t                 jrn_get_backtrace_level_on(void)                { return jrn_settings.backtrace_level_on; }
jrn_level_t                 jrn_get_memory_on_at(void)                      { return jrn_settings.memory_on_at; }
uint32_t                    jrn_get_max_file_size(void)                     { return jrn_settings.max_file_size; }
uint8_t                     jrn_get_backtrace_limit(void)                   { return jrn_settings.backtrace_limit; }
bool                        jrn_get_is_ready_to_write(void)                 { return jrn_settings.is_ready_to_write; }
bool                        jrn_get_is_streaming(void)                      { return jrn_settings.is_streaming; }
bool                        jrn_get_is_recording_mutex(void)                { return jrn_settings.is_recording_mutex; }
const char *                jrn_get_logfolder_name(void)                    { return jrn_settings.logfolder_name; }
const char *                jrn_get_basefile_name(void)                     { return jrn_settings.basefile_name; }

void *                      jrn_get_mutex(void)                             { return jrn_settings.mutex; }

jrn_memory_t                *jrn_get_memory(void)                           { return jrn_settings.memory; }

void                        jrn_set_is_ready_to_write(bool is_ready)        { jrn_settings.is_ready_to_write = is_ready; }
size_t                      jrn_get_settings_sz(void)                       { return sizeof(jrn_settings); }

const char *jrn_get_file_ext(void);

/* END JRN SETTINGS IFACE DEF*/

i_jrn_settings_t i_jrn_settings = {

    .get_filetype                               = jrn_get_filetype,
    .get_level                                  = jrn_get_level,
    .get_backtrace_level_on                     = jrn_get_backtrace_level_on,
    .get_memory_on_at                           = jrn_get_memory_on_at,
    .get_max_file_size                          = jrn_get_max_file_size,
    .get_backtrace_limit                        = jrn_get_backtrace_limit,
    .get_is_ready_to_write                      = jrn_get_is_ready_to_write,
    .get_is_streaming                           = jrn_get_is_streaming,
    .get_is_recording_mutex                     = jrn_get_is_recording_mutex,

    .get_mutex                                  = jrn_get_mutex,
    .get_memory                                 = jrn_get_memory,

    .get_logfolder_name                         = jrn_get_logfolder_name,
    .get_basefile_name                          = jrn_get_basefile_name,
    .get_settings_sz                            = jrn_get_settings_sz,

    .get_file_ext                               = jrn_get_file_ext,

    /////

    .set_is_ready_to_write                      = jrn_set_is_ready_to_write,
};

const char *
jrn_get_file_ext(void)
{
    return jrn_settings.filetype == LOG_FILE_HTML ? ".html" : jrn_settings.filetype == LOG_FILE_CSV ? ".csv"
                                                          : jrn_settings.filetype == LOG_FILE_JSON  ? ".json"
                                                                                                    : ".txt";
}

const char *
jrn_get_archive_name(void)
{
    return jrn_settings.filetype == LOG_FILE_HTML ? "log_html.tar.gz" : jrn_settings.filetype == LOG_FILE_CSV ? "log_csv.tar.gz"
                                                                    : jrn_settings.filetype == LOG_FILE_JSON  ? "log_json.tar.gz"
                                                                                                              : "log_txt.tar.gz";
}

const char *
jrn_get_log_type(jrn_level_t log_type)
{
    return log_type == LOG_LEVEL_DEBUG ? "Debug" : log_type == LOG_LEVEL_WARNING ? "Warning"
                                               : log_type == LOG_LEVEL_ERROR     ? "Error"
                                                                                 : "Info";
}

void jrn_print(const char *text)
{
    if (write(STDOUT_FILENO, text, strlen(text) + 1) != (ssize_t)strlen(text) + 1)
    {
        exit(EXIT_FAILURE);
    }
}

void jrn_internal_signal_m(jrn_status_t status, const char *message, const char *__file__, const size_t __line__)
{
#ifdef JRN_INTERNAL_REPORT

    jrn_print("\033[38;5;224m[Journey internal]\033[0m\n* ");

    jrn_print(__file__);

    char buf[JRN_BUF_16] = "";
    jrn_itoa_u(buf, JRN_BUF_16, __line__, 10);
    jrn_print(":");
    jrn_print(buf);

    jrn_print("\t\t");
    jrn_print(message);
    jrn_print("\n");
#endif

    if (status > JRN_INTERNAL_SHUTDOWN_SEVERITY)
    {
#ifdef JRN_INTERNAL_REPORT

        jrn_print("* Critical...\n");
#endif
        jrn_destroy();
        exit(EXIT_FAILURE);
    }
}

/*PUBLIC FUNCTIONS DEFINITIONS*/

void jrn_init(
    const char *logfolder_name,
    const char *basefile_name,
    jrn_level_t level,
    jrn_file_type_t filetype,
    bool is_streaming)
{

    memset(jrn_settings.logfolder_name, 0, JRN_BUF_MAX_FILENAME);
    strncpy(jrn_settings.logfolder_name, logfolder_name, JRN_BUF_MAX_FILENAME);

    jrn_settings.filetype = filetype;
    jrn_settings.level = level;
    jrn_settings.is_streaming = is_streaming;

    jrn_settings.is_set = true;

    jrn_settings.memory = jrn_memory_init();

    jrn_folder_init_from_settings();
}

void jrn_mutex_attach(void *mutex)
{
    jrn_settings.mutex = mutex;
}
void jrn_mutex_detach(void)
{
    jrn_settings.mutex = NULL;
}

void jrn_destroy(void)
{
    if (jrn_settings.memory)
    {

        jrn_memory_free(jrn_settings.memory);
    }
}
