#include "jrn_internal.h"
#include <uuid/uuid.h>
#include <pwd.h>
#define JRN_ITOA_MAX
#define JRN_ASCII_ZERO 48
#define JRN_ASCII_ALPHA_SHIFT 55

void jrn_reverse(char *buf)
{

    // empty str guaranees sz 1
    size_t i = 0, j = strlen(buf);
    if (j < 1)
        return;

    j -= 1;

    /* size_t j safety guaranteed since i<j has to be true */
    for (; i < j; i++, j--)
    {
        char c = buf[i];
        buf[i] = buf[j];
        buf[j] = c;
    }
}

jrn_status_t jrn_itoa_u(char *buf, uint8_t buf_size, uint64_t n, uint8_t base)
{
    memset(buf, 0, buf_size);
    size_t i = 0;

    do
    {
        if (i >= buf_size)
        {
            return JRN_WARN_BUF;
        }
        char c = ((n % base) < 10) ? (n % base) + JRN_ASCII_ZERO : (n % base) + JRN_ASCII_ALPHA_SHIFT;
        buf[i++] = c;
    } while (n /= base);

    buf[i] = '\0';
    jrn_reverse(buf);
    return JRN_OK;
}

void jrn_get_current_time(char *buf)
{
    time_t temp;
    struct tm *timeptr;

    temp = time(NULL);
    timeptr = localtime(&temp);

    strftime(buf, JRN_BUF_64, "%A, %b %d Time: %r", timeptr);
}

void jrn_get_current_user(char *buf)
{
    struct passwd *passwd;
    passwd = getpwuid(getuid());
    strncpy(buf, passwd->pw_name, strlen(passwd->pw_name));
}

void jrn_generate_uuid(char *buf)
{
    uuid_t uuid;
    uuid_generate_random(uuid);
    char *uuid_str = malloc(37);

    uuid_unparse_lower(uuid, uuid_str);

    str_n_cat(buf, uuid_str);

    free(uuid_str);
}

void jrn_read_from_file(char *buf, const char *filename)
{   

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        jrn_internal_signal(JRN_ERR_FILE, "Cannot open file to read");
    }

    fread(buf, JRN_BUF, 1, fp);

    fclose(fp);
}
