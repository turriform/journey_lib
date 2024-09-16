#include "jrn_internal.h"
#include "jrn_record.h"

// FWD

extern const char *
jrn_get_log_type(jrn_level_t log_type);
extern int
jrn_file_try_get_or_create_default(void);
extern void
jrn_folder_write_to_current_file(int file_d, const char *message, size_t offset);


void i_jrn_file_text_create_entry(jrn_record_t *record)
{

    int file_d = jrn_file_try_get_or_create_default();

    const char *level_str =
        jrn_get_log_type(record->debug_level);

    char text_record[JRN_BUF] = "";

    strncat(text_record, level_str, strlen(level_str));
    strncat(text_record, "\t", JRN_BUF_16);
    strncat(text_record, record->time, strlen(record->time));
    strncat(text_record, "\t", JRN_BUF_16);
    strncat(text_record, record->user, strlen(record->user));
    strncat(text_record, "\t", JRN_BUF_16);
    strncat(text_record, record->message, strlen(record->message));
    strncat(text_record, "\t", JRN_BUF_16);
    strncat(text_record, record->__file__, strlen(record->__file__));
    strncat(text_record, "\t", JRN_BUF_16);
    strncat(text_record, record->__line__, strlen(record->__line__));
    strncat(text_record, "\n", JRN_BUF_16);
    if (record->stacktrace != NULL)
    {

        for (size_t i = 0; i != record->stacktrace_sz; i++)
        {
            strncat(text_record, record->stacktrace[i], strlen(record->stacktrace[i]));
            strncat(text_record, "\n", JRN_BUF_16);
        }
    }
    strncat(text_record, "\n", JRN_BUF_16);

    jrn_folder_write_to_current_file(file_d, text_record, 0);

    close(file_d);

    jrn_internal_signal(JRN_INFO_FILE, "i_jrn_file_text_create_entry written");
}
