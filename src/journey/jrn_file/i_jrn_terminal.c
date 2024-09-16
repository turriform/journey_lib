#include "jrn_internal.h"
#include "jrn_record.h"

#define JRN_COLOR_DEBUG "\033[38;5;35m"
#define JRN_COLOR_INFO "\033[38;5;46m"
#define JRN_COLOR_WARNING "\033[38;5;222m"
#define JRN_COLOR_ERROR "\033[38;5;201m"
#define JRN_COLOR_NULL "\033[0m"

extern const char *
jrn_get_log_type(jrn_level_t log_type);

static inline const char *i_jrn_terminal_color_str(jrn_level_t level)
{
    return level == LOG_LEVEL_DEBUG ? JRN_COLOR_DEBUG : level == LOG_LEVEL_INFO  ? JRN_COLOR_INFO
                                                    : level == LOG_LEVEL_WARNING ? JRN_COLOR_WARNING
                                                                                 : JRN_COLOR_ERROR;
}

void i_jrn_terminal_create_entry(jrn_record_t *record)
{

    const char *level_str =
        jrn_get_log_type(record->debug_level);

    const char *color_str = i_jrn_terminal_color_str(record->debug_level);

    char terminal_record[JRN_BUF] = "\n";

    str_n_cat(terminal_record, color_str);
    str_n_cat(terminal_record, "[");
    str_n_cat(terminal_record, level_str);
    str_n_cat(terminal_record, "]");
    str_n_cat(terminal_record, JRN_COLOR_NULL);
    str_n_cat(terminal_record, "\n* user:");
    str_n_cat(terminal_record, record->user);
    str_n_cat(terminal_record, "\ttime:");
    str_n_cat(terminal_record, record->time);
    str_n_cat(terminal_record, "\n* ");
    str_n_cat(terminal_record, record->message);
    str_n_cat(terminal_record, "\t");
    str_n_cat(terminal_record, record->__file__);
    str_n_cat(terminal_record, ":");
    str_n_cat(terminal_record, record->__line__);
    str_n_cat(terminal_record, "\n");

    if (record->stacktrace != NULL)
    {
        str_n_cat(terminal_record, "[trace]\n");
        for (size_t i = 0; i != record->stacktrace_sz; i++)
        {
            str_n_cat(terminal_record, "*\t");
            str_n_cat(terminal_record, record->stacktrace[i]);
            str_n_cat(terminal_record, "\n");
        }
    }

    str_n_cat(terminal_record, "\n");
    jrn_print(terminal_record);
}