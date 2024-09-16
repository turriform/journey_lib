#if !defined(__JRN_RECORD_H)
#define __JRN_RECORD_H
#include "jrn_internal.h"

typedef struct JrnRecord
{

    jrn_level_t debug_level;
    const char *time;
    const char *message;
    const char *__file__;
    const char *__line__;
    const char *user;
    char ** stacktrace;
    uint8_t stacktrace_sz;

} jrn_record_t;


//not impl
typedef struct JrnBacktraceLine{
    const char * prog;
    const char * func;
    const char * addr;

} jrn_backtrace_line_t;

void jrn_back_trace_line_fill(jrn_backtrace_line_t * backtrace_line, const char * string); //not impl

#endif // __JRN_RECORD_H
