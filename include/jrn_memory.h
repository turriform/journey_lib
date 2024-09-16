#if !defined(_L_JOURNEY_MEMORY_H)
#define _L_JOURNEY_MEMORY_H
#include "jrn_internal.h"

#define JRN_MEM_FILL_RATIO                      (double)0.65
#define JRN_MEM_FILL_INIT                       16U
#define JRN_MEM_GRAPH_LIMIT                     50U


struct JrnMemoryTick
{
    size_t memory_allocated;
    size_t memory_freed;
};

struct JrnMemory
{

    size_t cur_alloced;
    size_t cur_freed;

    size_t ticks_size;
    size_t ticks_alloced;
    jrn_memory_tick_t **memory_ticks;
};

jrn_memory_t *
jrn_memory_init(void);

void 
jrn_memory_append_tick(jrn_memory_t *self, size_t mem_alloced, size_t mem_freed);

void 
jrn_memory_free(jrn_memory_t *self);

jrn_memory_tick_t *
jrn_memory_tick_create(size_t alloced, size_t freed);

void 
jrn_memory_summary_print(jrn_memory_t *self);

void 
jrn_memory_fill_graph(jrn_memory_t *self, char *buf);
#endif // _L_JOURNEY_MEMORY_H
