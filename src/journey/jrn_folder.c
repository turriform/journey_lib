#include "jrn_internal.h"

#include <stdio.h>

extern i_jrn_settings_t i_jrn_settings;

extern void i_jrn_file_html_create_entry(jrn_record_t *record);
extern void i_jrn_file_text_create_entry(jrn_record_t *record);
extern void i_jrn_file_json_create_entry(jrn_record_t *record);


void jrn_folder_init_from_settings(void);

void jrn_folder_get_abs_path(char *buf);
DIR *jrn_folder_try_get_or_create(void);

size_t jrn_folder_get_file_count(void);

void jrn_folder_bulk_rename_create_default(void);

void jrn_file_get_abs_path(char *buf, size_t order);
void jrn_file_get_default_name(char *buf);
void jrn_file_get_next_abs_file_path(char *buf_new, char *old);

int jrn_file_try_get_or_create_default(void);
bool jrn_file_is_ready_to_write(int file_d);
bool jrn_file_is_empty(int file_d);
bool jrn_file_name_conforms(const char *filename);

void jrn_folder_write_to_current_file(int file_d, const char *message, size_t offset);
///////////////////

void jrn_folder_init_from_settings(void)
{
    /*
        will create specific folder if it doesnt exist
        utilises params from i_jrn_settings
    */
    char path[JRN_BUF_MAX_PATH] = "";
    jrn_folder_get_abs_path(path);

    jrn_internal_signal(JRN_INFO_DIR, path);

    char filepath[JRN_BUF_MAX_PATH] = "";
    jrn_file_get_abs_path(filepath, 0);
    jrn_internal_signal(JRN_INFO_FILE, filepath);

    DIR *dir = jrn_folder_try_get_or_create();
    closedir(dir);

    int file_d = jrn_file_try_get_or_create_default();
    close(file_d);
    jrn_file_name_conforms("log.txt");

    jrn_folder_get_file_count();
}

void jrn_folder_get_abs_path(char *buf)
{

    const char *folder_name = i_jrn_settings.get_logfolder_name();
    if (getcwd(buf, JRN_BUF_MAX_PATH) == NULL)
    {
        jrn_internal_signal(JRN_ERR_CWD, "Error getting cwd");
    }
    strncat(buf, JRN_PATH_SEP, JRN_BUF_16);
    strncat(buf, folder_name, JRN_BUF_MAX_FILENAME);
    strncat(buf, JRN_PATH_SEP, JRN_BUF_16);
}

void jrn_file_get_default_name(char *buf)
{
    /* writes the default name such as log.txt mylog.html etc*/

    const char *file_name = i_jrn_settings.get_basefile_name();
    const char *ext = i_jrn_settings.get_file_ext();
    strncat(buf, file_name, strlen(file_name));
    strncat(buf, ext, JRN_BUF_16);
}

void jrn_file_get_abs_path(char *buf, size_t order)
{
    /*
        @param          buf         buffer to write to
        @param          order       order of file index, 0 will result the default file /path/to/file/log.html

    */

    const char *file_name = i_jrn_settings.get_basefile_name();
    const char *ext = i_jrn_settings.get_file_ext();

    jrn_folder_get_abs_path(buf);

    if (order == 0)
    {
        jrn_file_get_default_name(buf);
    }
    else
    {
        char num[JRN_BUF_16] = "";
        jrn_itoa_u(num, JRN_BUF_16, order, 10);

        strncat(buf, file_name, strlen(file_name));
        strncat(buf, "-", JRN_BUF_16);
        strncat(buf, num, strlen(num));
        strncat(buf, ext, strlen(ext));
    }
}

void jrn_file_get_next_abs_file_path(char *buf_new, char *old)
{
    /*
        paths should be relative to


    */
    char abs_path[JRN_BUF_MAX_PATH] = "";

    jrn_folder_get_abs_path(abs_path);

    char old_relative[JRN_BUF_MAX_FILENAME] = "";

    size_t i = 0, j = 0;
    while (abs_path[i] == old[i])
        i++;

    if (i > 0) //  size_t correctness
        i--;

    while (i != strlen(old) + 1)
    {
        old_relative[j++] = old[i++];
    }

    char default_name[JRN_BUF_MAX_FILENAME] = "";
    jrn_file_get_default_name(default_name);

    if (strncmp(old_relative, default_name, strlen(old_relative)) == 0)
    {
        // path is default
        jrn_file_get_abs_path(buf_new, 1);
    }
    else
    {
        // path is not the default one calculating next order

        char num[JRN_BUF_16] = "";
        i = 0, j = 0;
        while (old_relative[i] != '-')
            i++;

        while (old_relative[i++] != '.')
        {
            num[j++] = old_relative[i];
        }

        size_t n = atoll(num) + 1;

        jrn_file_get_abs_path(buf_new, n);
    }
}

DIR *jrn_folder_try_get_or_create(void)
{
    DIR *dir;
    char path[JRN_BUF_MAX_PATH] = "";
    jrn_folder_get_abs_path(path);

    dir = opendir(path);
    if (dir == NULL)
    {
        jrn_internal_signal(JRN_INFO_DIR, "Directory didn't exist, creating new");

        if (mkdir(path, JRN_DIRMODE) != 0)
        {
            jrn_internal_signal(JRN_ERR_DIR, "Cannot create the directory");
        }
    }
    else
    {
        jrn_internal_signal(JRN_OK, "Directory existed: opening...");
    }

    return dir;
}

size_t jrn_folder_get_file_count(void)
{

    /*
        getting the number of actual log files in the folder
        since the folder can contain archive and index
        names are compared agains the default filename
    */
    size_t count = 0;

    char default_name[JRN_BUF_MAX_FILENAME] = "";
    jrn_file_get_default_name(default_name);

    DIR *dir = jrn_folder_try_get_or_create();
    struct dirent *dn;

    while ((dn = readdir(dir)))
    {
        if (jrn_file_name_conforms(dn->d_name) && dn->d_name[0] != '.')
            count++;
    }

    closedir(dir);

    char count_str[JRN_BUF_16] = "";
    jrn_itoa_u(count_str, JRN_BUF_16, count, 10);
    char message[JRN_BUF_256] = "Files in the folder that conform to default: ";
    strncat(message, count_str, strlen(count_str));

    jrn_internal_signal(JRN_INFO_DIR, message);

    return count;
}

void jrn_folder_bulk_rename_create_default(void)
{
    /*
        this function is called when current file is not ok to write
        renaming is handled from up to down for an obviouse reason
    */
    size_t conformed_files = jrn_folder_get_file_count();

    char newname[JRN_BUF_MAX_PATH] = "";

    char oldname[JRN_BUF_MAX_PATH] = "";

    for (int i = conformed_files - 1; i != -1; i--)
    {

        jrn_file_get_abs_path(oldname, i);
        jrn_file_get_abs_path(newname, i + 1);
        rename(oldname, newname);
    }

    int file_d = jrn_file_try_get_or_create_default();
    close(file_d);
}

////////////////////////////////////////////FILES

bool jrn_file_is_ready_to_write(int file_d)
{
    // determines if files should be bull renamed in a folder
    size_t max_file_size = i_jrn_settings.get_max_file_size();
    struct stat fs;
    fstat(file_d, &fs);
    return fs.st_size < (off_t)max_file_size;
}

bool jrn_file_is_empty(int file_d)
{
    struct stat fs;
    fstat(file_d, &fs);
    return fs.st_size == 0;
}

bool jrn_file_name_conforms(const char *filename)
{
    /* ad-hoc to check if the filename belongs to log*/
    char default_name[JRN_BUF_MAX_FILENAME] = "";
    jrn_file_get_default_name(default_name);

    // case where filename itself is canonical
    if (strncmp(default_name, filename, strlen(default_name)) == 0)
    {
        return true;
    }

    size_t i = 0, j = 0;
    while (default_name[i++] == filename[j++])
        ;

    while (filename[j++] != '.')
    {
        if (filename[j] == '\0')
            return false;
    }

    while (j != strlen(filename))
    {
        i++;
        j++;
        if (default_name[i] != filename[j])
        {
            return false;
        }
    }

    return true;
}

int jrn_file_try_get_or_create_default(void)
{
    char new[JRN_BUF_MAX_PATH] = "";
    jrn_file_get_abs_path(new, 0);

    int file_d;

    if ((file_d = open(new, O_WRONLY)) < 0)
    {
        jrn_internal_signal(JRN_INFO_FILE, "File didn't exist creating ...");

        if ((file_d = creat(new, JRN_FILEMODE)) < 0)
        {
            jrn_internal_signal(JRN_ERR_FILE, "Cannot create file, you should probably create the directory first");
        }
        else
        {
            jrn_internal_signal(JRN_INFO_FILE, "File created");
        }
    }

    else
    {
        jrn_internal_signal(JRN_INFO_FILE, "File existed");
    }

    bool is_file_ok = jrn_file_is_ready_to_write(file_d);
    i_jrn_settings.set_is_ready_to_write(is_file_ok);
    jrn_internal_signal(JRN_INFO_FILE, (is_file_ok ? "FILE OK" : "FILE NOT OK"));
    close(file_d);

    if (!is_file_ok)
    {
        jrn_folder_bulk_rename_create_default();
        jrn_internal_signal(JRN_INFO_FILE, "File was not ok to write, bulk renaming all folder and creating new default");
    }

    // now try open on append
    if ((file_d = open(new, O_WRONLY)) < 0)
    {

        jrn_internal_signal(JRN_ERR_FILE, "Cannot open file to append");
    }
    jrn_internal_signal(JRN_INFO_FILE, "File is open and ready to write");

    return file_d;
}

void jrn_folder_write_to_current_file(int file_d, const char *message, size_t offset)
{

    // TODO mutexlock

    size_t offs = offset * -1 > 0 ? offset * -1 : 0;

    lseek(file_d, offs, SEEK_END);

    if (write(file_d, message, strlen(message)) < 0)
    {
        jrn_internal_signal(JRN_ERR_FILE, "Error writing to file");
    }
    else
    {
        jrn_internal_signal(JRN_INFO_FILE, "Record created, all good");
    }

    // TODO mutexunlock
}

void *jrn_folder_get_file_handler(void)
{


    jrn_file_type_t filetype = i_jrn_settings.get_filetype();
    switch (filetype)
    {
    case LOG_FILE_HTML:
        return &i_jrn_file_html_create_entry;

    case LOG_FILE_JSON:
        return &i_jrn_file_json_create_entry;
    default:
        return &i_jrn_file_text_create_entry;
    }
    //to avoid warnings
    return &i_jrn_file_text_create_entry;
}