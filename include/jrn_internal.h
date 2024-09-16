#if !defined(L_JOURNEY_LIB_INTERNAL_H)
#define L_JOURNEY_LIB_INTERNAL_H
#include "jrn_framework.h"

#define str_n_cat(dest, src) strncat((char *)(dest), (const char *)(src), strlen((const char *)(src)) + 1)
#define str_n_cmp(dest, src) strncmp((char *)(dest), (const char *)(src), strlen((const char *)(src)))

// Global types
typedef struct      JrnSettings             jrn_settings_t;
typedef struct      JrnFolder               jrn_folder_t;

// Global types ifaces
typedef struct      IJrnSettings            i_jrn_settings_t;
typedef struct      IJrnMemory              i_jrn_memory_t;

// Utility types
typedef struct      JrnFile                 jrn_file_t;
typedef struct      JrnMemory               jrn_memory_t;
typedef struct      JrnMemoryTick           jrn_memory_tick_t;
typedef struct      JrnRecord               jrn_record_t;

typedef enum
{
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,

    LEVEL_N

} jrn_level_t;

typedef enum
{
    LOG_FILE_TEXT,
    LOG_FILE_HTML,
    LOG_FILE_CSV,
    LOG_FILE_JSON,
    LOG_FILE_SQLITE,

    LOG_FILE_N

} jrn_file_type_t;

/* Defaults */

#define JRN_DEFAULT_FILETYPE                            LOG_FILE_HTML             /* default filetype */
#define JRN_DEFAULT_LEVEL                               LOG_LEVEL_DEBUG              /* log level from debug and up*/
#define JRN_DEFAULT_IS_STREAMING                        true                  /* will output log realite */
#define JRN_DEFAULT_BACKTRACE_LIMIT                     5U                 /* stack shows 5 records*/
#define JRN_DEFAULT_BACKTRACE_LEVEL_ON                  LOG_LEVEL_DEBUG /* backtrace called only on errors */
#define JRN_DEFAULT_MEMORY_ON_AT                        LOG_LEVEL_DEBUG       /* mem display starts */
#define JRN_DEFAULT_LOGFOLDER_NAME                      "log"               /* log folder relative path*/
#define JRN_DEFAULT_BASEFILE_NAME                       "log"                /* basefilename for log*/
#define JRN_DEFAULT_IS_RECORDING_MUTEX                  true            /* will log all mutex locks and unlocks*/
#define JRN_DEFAULT_MAX_FILE_SIZE                       512U * 1024U
#define JRN_DEFAULT_IS_READY_TO_WRITE                   false

/* Main journey functions*/

void jrn_log(jrn_level_t level_type, const char *__file__, size_t __line__, ...);
void *jrn_malloc(size_t size, const char *__file__, size_t __line__);
void *jrn_calloc(size_t nmem, size_t size, const char *__file__, size_t __line__);
void *jrn_realloc(void *ptr, size_t obj_size, size_t size, const char *__file__, size_t __line__);
void jrn_free(void **ptr, const char *__file__, size_t __line__);

/* Iface definitions*/

struct IJrnSettings
{
    jrn_file_type_t                 (*get_filetype)(void);
    jrn_level_t                     (*get_level)(void);
    jrn_level_t                     (*get_backtrace_level_on)(void);
    jrn_level_t                     (*get_memory_on_at)(void);
    uint32_t                        (*get_max_file_size)(void);
    uint8_t                         (*get_backtrace_limit)(void);
    bool                            (*get_is_streaming)(void);
    bool                            (*get_is_recording_mutex)(void);
    bool                            (*get_is_ready_to_write)(void);
    const char *                    (*get_logfolder_name)(void);
    const char *                    (*get_basefile_name)(void);

    const char *                    (*get_file_ext)(void);
    size_t                          (*get_settings_sz)(void);

    void                            (*set_is_ready_to_write)(bool is_ready);

    sqlite3 *                       (*get_sqlite)(void);

    void                            *(*get_mutex)(void);

    jrn_memory_t                    *(*get_memory)(void);
};

/* Journey internal status */
typedef enum
{
    JRN_OK = 0,

    JRN_INFO_DIR,
    JRN_INFO_FILE,
    JRN_INFO_DB,
    JRN_MUTEX_INFO,
    JRN_WARN_BUF,
    JRN_WARN_DOUBLE_FREE,
    JRN_ERR_STR,
    // Default critical error start here -> program will termiante at [JRN_INTERNAL_SHUTDOWN_SEVERITY]
    JRN_ERR_DIR,
    JRN_ERR_DB,
    JRN_ERR_FILE,
    JRN_ERR_LSEEK,
    JRN_ERR_WRITE,
    JRN_ERR_READ,
    JRN_ERR_CWD,
    JRN_ERR_MALLOC,
    JRN_ERR_CALLOC,
    JRN_ERR_REALLOC,
    JRN_ERR_FREE,
    JRN_ERR

} jrn_status_t;

#define JRN_INTERNAL_SHUTDOWN_SEVERITY JRN_ERR_CWD

void jrn_internal_signal_m(jrn_status_t status, const char *message, const char *__file__, const size_t __line__);

#define jrn_internal_signal(status, message) jrn_internal_signal_m(status, message, __FILE__, __LINE__)

/* Mutex */

#define JRN_MUTEX_LOCK                                                             \
    do                                                                             \
    {                                                                              \
        if (i_jrn_settings.get_mutex() != NULL)                                    \
        {                                                                          \
            jrn_internal_signal(JRN_MUTEX_INFO, "Locking mutex");                  \
            if (i_jrn_settings.get_is_recording_mutex())                           \
            {                                                                      \
                jrn_log(LOG_LEVEL_INFO, __FILE__, __LINE__, "Mutex locked", NULL); \
            }                                                                      \
                                                                                   \
            pthread_mutex_lock(i_jrn_settings.get_mutex());                        \
        }                                                                          \
        else                                                                       \
        {                                                                          \
            jrn_internal_signal(JRN_MUTEX_INFO, "No mutex passed");                \
        }                                                                          \
    } while (0)

#define JRN_MUTEX_UNLOCK                                                             \
    do                                                                               \
    {                                                                                \
        if (i_jrn_settings.get_mutex() != NULL)                                      \
        {                                                                            \
            jrn_internal_signal(JRN_MUTEX_INFO, "Unlocking mutex");                  \
            pthread_mutex_unlock(i_jrn_settings.get_mutex());                        \
            if (i_jrn_settings.get_is_recording_mutex())                             \
            {                                                                        \
                jrn_log(LOG_LEVEL_INFO, __FILE__, __LINE__, "Mutex unlocked", NULL); \
            }                                                                        \
        }                                                                            \
                                                                                     \
    } while (0)

/* Journey init iface */

void jrn_init(const char *logfolder_name,
              const char *basefile_name,
              jrn_level_t level,
              jrn_file_type_t filetype,
              bool is_streaming);

void jrn_destroy(void);

void jrn_mutex_attach(void *mutex);
void jrn_mutex_detach(void);

/*Journey utils*/

jrn_status_t jrn_itoa_u(char *buf, uint8_t buf_size, uint64_t n, uint8_t base);
void jrn_get_current_time(char *buf);
void jrn_print(const char *text);
void jrn_get_current_user(char *buf);
void jrn_generate_uuid(char *buf);
void jrn_read_from_file(char *buf, const char *filename);

#endif // L_JOURNEY_LIB_INTERNAL_H
