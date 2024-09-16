
#include "jrn_internal.h"
#include "jrn_record.h"
#include "jrn_memory.h"

// FWD

extern i_jrn_settings_t i_jrn_settings;

extern const char *
jrn_get_log_type(jrn_level_t log_type);

extern int
jrn_file_try_get_or_create_default(void);
extern void
jrn_folder_write_to_current_file(int file_d, const char *message, size_t offset);
extern bool
jrn_file_is_empty(int file_d);

static inline void
i_jrn_html_write_memory(char *template_dest)
{
    jrn_memory_t *mem = i_jrn_settings.get_memory();

    if (mem==NULL){
        jrn_internal_signal(JRN_OK, "Memory is not initialized");
        return;
    }
    

    char graph_buf[JRN_BUF] = "";
    jrn_memory_fill_graph(mem, graph_buf);

    str_n_cat(template_dest, ",\"memory_ticks\":[");
    str_n_cat(template_dest, graph_buf);

    str_n_cat(template_dest, "]");
}

static inline void 
i_jrn_file_json_write_line(char *json_record, const char *key, const char *value, bool is_numeric, bool is_last)
{
    str_n_cat(json_record, "\"");
    str_n_cat(json_record, key);
    str_n_cat(json_record, "\":");
    if (!is_numeric)
    {
        str_n_cat(json_record, "\"");
        str_n_cat(json_record, value);
        str_n_cat(json_record, "\"");
    }
    else
    {
        str_n_cat(json_record, value);
    }
    if (!is_last)
    {
        str_n_cat(json_record, ",");
    }
    str_n_cat(json_record, "\n");
}

void i_jrn_file_json_create_entry(jrn_record_t *record)
{
    int file_d = jrn_file_try_get_or_create_default();

    const char *level_str =
        jrn_get_log_type(record->debug_level);

    char record_uuid[JRN_BUF_64] = "";
    jrn_generate_uuid(record_uuid);

    char json_record[JRN_BUF] = "";

    // record start
    str_n_cat(json_record, "\n{\n");

    i_jrn_file_json_write_line(json_record, "uuid", record_uuid, false, false);
    i_jrn_file_json_write_line(json_record, "type", level_str, false, false);
    i_jrn_file_json_write_line(json_record, "time", record->time, false, false);
    i_jrn_file_json_write_line(json_record, "user", record->user, false, false);
    i_jrn_file_json_write_line(json_record, "message", record->message, false, false);
    i_jrn_file_json_write_line(json_record, "file", record->__file__, false, false);
    i_jrn_file_json_write_line(json_record, "line", record->__line__, true, true);

    if (record->stacktrace != NULL)
    {
        str_n_cat(json_record, ",\n\"stack\": [\n");
        for (size_t i = 0; i != record->stacktrace_sz; i++)
        {
            str_n_cat(json_record, "\"");
            str_n_cat(json_record, record->stacktrace[i]);
            str_n_cat(json_record, "\"");
            if (i < (size_t)record->stacktrace_sz - 1)
            {
                str_n_cat(json_record, ",");
            }
            str_n_cat(json_record, "\n");
        }

        str_n_cat(json_record, "]\n");
    }

    i_jrn_html_write_memory(json_record);

    str_n_cat(json_record, "},");
    // record end

    char template_footer[JRN_BUF] = "{}\n]";

    if (jrn_file_is_empty(file_d))
    {
        char template_header[JRN_BUF] = "[\n";

        str_n_cat(template_header, json_record);
        str_n_cat(template_header, template_footer);

        jrn_folder_write_to_current_file(file_d, template_header, 0);
    }
    else
    {
        str_n_cat(json_record, template_footer);
        jrn_folder_write_to_current_file(file_d, json_record, strlen(template_footer));
    }
    close(file_d);

    jrn_internal_signal(JRN_INFO_FILE, "i_jrn_file_json_create_entry written");
}
