/*MIT License

Copyright (c) 2024 Marie Turriform

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#if !defined(__JOURNEY_LIB_H)
#define __JOURNEY_LIB_H

#include "jrn_internal.h"


#define VARIADIC_USE

typedef enum
{
    ERR_MALLOC,
    ERR_CALLOC,
    ERR_REALLOC,
    ERR_FREE

} jrn_err_code_t;




#define jrn_debug(...)          jrn_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__, NULL)
#define jrn_info(...)           jrn_log(LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__, NULL)
#define jrn_warning(...)        jrn_log(LOG_LEVEL_WARNING, __FILE__, __LINE__, __VA_ARGS__, NULL)
#define jrn_err(...)            jrn_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__, NULL)





#define malloc(size)            jrn_malloc((size), __FILE__, __LINE__)
#define calloc(nmem, size)      jrn_calloc((nmem), (size), __FILE__, __LINE__)
#define realloc(ptr, size)      jrn_realloc(((void*)ptr), (sizeof(*ptr)), size, __FILE__, __LINE__)
#define free(ptr)               jrn_free(((void**)&ptr), __FILE__, __LINE__)

#endif // __JOURNEY_LIB_H
