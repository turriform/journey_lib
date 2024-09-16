
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
i_jrn_file_html_write_tag(char *template_dest, const char *tag, const char *tag_id, const char *tag_class, const char *content)
{
    str_n_cat(template_dest, "\t\t\t\t\t\t<");
    str_n_cat(template_dest, tag);

    if (strlen(tag_id) > 0)
    {
        str_n_cat(template_dest, " id=\"");
        str_n_cat(template_dest, tag_id);
        str_n_cat(template_dest, "\"");
    }

    str_n_cat(template_dest, " class=\"");
    str_n_cat(template_dest, tag_class);
    str_n_cat(template_dest, "\">");
    str_n_cat(template_dest, content);
    str_n_cat(template_dest, "</");
    str_n_cat(template_dest, tag);
    str_n_cat(template_dest, ">\n");
}

static inline void
i_jrn_html_write_memory(char *template_dest, char *uuid)
{
    jrn_memory_t *mem = i_jrn_settings.get_memory();

    str_n_cat(template_dest, "\t\t\t\t\t<div id=\"");
    str_n_cat(template_dest, uuid);
    str_n_cat(template_dest,
              "\" style=\"width:100%; max-width:600px; height:200px;\"></div>\n"
              "\t\t\t\t\t\t<script>\n"
              "\t\t\t\t\t\t\tgoogle.charts.load('current', { packages: ['corechart'] });\n"
              "\t\t\t\t\t\t\tgoogle.charts.setOnLoadCallback(drawChart);\n"
              "\t\t\t\t\t\t\tfunction drawChart() {\n"
              "\t\t\t\t\t\t\tconst data = google.visualization.arrayToDataTable([\n");

    char graph_buf[JRN_BUF] = "";
    str_n_cat(graph_buf, "\t\t\t\t\t\t\t\t['Ticks', 'Allocated', 'Freed'], ");

    jrn_memory_fill_graph(mem, graph_buf);
    str_n_cat(template_dest, graph_buf);

    str_n_cat(template_dest,
              "\n\t\t\t\t\t\t\t\t]);\n"
              "\t\t\t\t\t\t\tconst options = {\n"
              "\t\t\t\t\t\t\t\thAxis: { title: 'Ticks' , format: '0'},\n"
              "\t\t\t\t\t\t\t\tvAxis: { title: 'Allocations', format: '0' },\n"
              "\t\t\t\t\t\t\t\tlegend: 'Allocations',\n"
              "\t\t\t\t\t\t\t\tseries: { 0: { color: '#73c026' }, 1: { color: '#eb4034' }}"
              "\t\t\t\t\t\t\t};\n"
              "\t\t\t\t\t\t\tconst chart = new google.visualization.LineChart(document.getElementById('");

    str_n_cat(template_dest, uuid);

    str_n_cat(template_dest,
              "'));\n"
              "\t\t\t\t\t\t\tchart.draw(data, options);\n"
              "\t\t\t\t\t\t\t}\n"
              "\t\t\t\t\t\t</script>\n"

    );
}

void i_jrn_file_html_create_entry(jrn_record_t *record)
{

    int file_d = jrn_file_try_get_or_create_default();

    const char *level_str =
        jrn_get_log_type(record->debug_level);


    char record_uuid[JRN_BUF_64] = "";
    jrn_generate_uuid(record_uuid);

    char html_record[JRN_BUF] = "";

    str_n_cat(html_record, "\n\t\t\t\t\t<tr class=\"record\">\n");
    i_jrn_file_html_write_tag(html_record, "th", "", level_str, level_str);
    i_jrn_file_html_write_tag(html_record, "th", "", "time", record->time);
    i_jrn_file_html_write_tag(html_record, "th", "", "user", record->user);
    i_jrn_file_html_write_tag(html_record, "th", "", "message", record->message);
    i_jrn_file_html_write_tag(html_record, "th", "", "file", record->__file__);
    i_jrn_file_html_write_tag(html_record, "th", "", "line", record->__line__);
    str_n_cat(html_record, "\t\t\t\t\t</tr>\n");

    str_n_cat(html_record, "\t\t\t<tr class=\"extra\">\n");

    if (record->stacktrace != NULL)
    {
        str_n_cat(html_record, "\t\t\t\t\t<td colspan=\"2\">\n");
        for (size_t i = 0; i != record->stacktrace_sz; i++)
        {

            i_jrn_file_html_write_tag(html_record, "div", "", "trace", record->stacktrace[i]);
        }

        str_n_cat(html_record, "\t\t\t\t\t</td>\n");
    }

    // mem graph

    str_n_cat(html_record, "\t\t\t\t\t<td colspan=\"4\">\n");

    i_jrn_html_write_memory(html_record, record_uuid);

    str_n_cat(html_record, "\t\t\t\t\t</td>\n");

    // end mem graph

    str_n_cat(html_record, "\t\t\t\t\t</tr>\n");

    //
    char template_footer[JRN_BUF] = "";
    jrn_read_from_file(template_footer, "./html/log.footer.html");

    if (jrn_file_is_empty(file_d))
    {
        char template_header[JRN_BUF] = "";
        jrn_read_from_file(template_header, "./html/log.header.html");

        str_n_cat(template_header, html_record);
        str_n_cat(template_header, template_footer);

        jrn_folder_write_to_current_file(file_d, template_header, 0);
    }
    else
    {
        str_n_cat(html_record, template_footer);
        jrn_folder_write_to_current_file(file_d, html_record, strlen(template_footer));
    }
    close(file_d);

    jrn_internal_signal(JRN_INFO_FILE, "i_jrn_file_html_create_entry written");
}

// Runs on init
