#if !defined(_L_JRN_FRAMEWORK_H)
#define _L_JRN_FRAMEWORK_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>


#include <pthread.h>
#include <errno.h>

#define JRN_FILEMODE                            (S_IRUSR | S_IWUSR | S_IRGRP | S_IXOTH)
#define JRN_DIRMODE                             (JRN_FILEMODE | S_IXUSR | S_IXGRP | S_IXOTH)

#define JRN_BUF                                 4096U
#define JRN_BUF_16                              16U
#define JRN_BUF_64                              64U
#define JRN_BUF_256                             256U
#define JRN_BUF_512                             512U
#define JRN_BUF_1024                            1024U


#define JRN_BUF_MAX_PATH                        JRN_BUF
#define JRN_BUF_MAX_FILENAME                    JRN_BUF_64
#define JRN_BUF_MAX_LINE                        JRN_BUF
#define JRN_BUF_GRAPH                           JRN_BUF * 4U

//EOT
#define JRN_EOT                                 0x4U 

/* platform*/
#define JRN_PATH_SEP                            "/"




/* End Defaults */

#endif // _L_JRN_FRAMEWORK_H
