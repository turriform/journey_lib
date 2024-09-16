#include "jrn_memory.h"

jrn_memory_t *jrn_memory_init(void)
{
    jrn_memory_t *memory = malloc(sizeof(jrn_memory_t));
    if (memory == NULL)
    {
        jrn_internal_signal(JRN_ERR_MALLOC, "Error allocating memory");
    }

    memory->ticks_alloced = JRN_MEM_FILL_INIT;

    memory->memory_ticks = calloc(memory->ticks_alloced, sizeof(jrn_memory_tick_t *));

    if (memory->memory_ticks == NULL)
    {
        jrn_internal_signal(JRN_ERR_CALLOC, "Cannot allocate memory ticks");
    }

    jrn_memory_tick_t *first_tick = jrn_memory_tick_create(0, 0);

    // inserting first tick

    memory->ticks_size = 1;

    memory->memory_ticks[0] = first_tick;

    memory->cur_alloced = 0;
    memory->cur_freed = 0;

    return memory;
}
void jrn_memory_free(jrn_memory_t *self)
{
    if (self != NULL)
    {
        for (size_t i = 0; i != self->ticks_size; i++)
            free(self->memory_ticks[i]);

        free(self->memory_ticks);
        free(self);
    }
}

void jrn_memory_append_tick(jrn_memory_t *self, size_t mem_alloced, size_t mem_freed)
{
    self->cur_alloced += mem_alloced;
    self->cur_freed += mem_freed;

    if ((double)(self->ticks_size / self->ticks_alloced) >= (double)0.5)
    {

        self->memory_ticks = realloc(self->memory_ticks, (self->ticks_alloced *= 2) * 8);
        if (self->memory_ticks == NULL)
        {
            jrn_internal_signal(JRN_ERR_REALLOC, "Cannot realloc memory ticks");
        }
        jrn_internal_signal(JRN_OK, "Reallocating memory ticks");
    }

    jrn_memory_tick_t *new_tick =
        jrn_memory_tick_create(self->cur_alloced, self->cur_freed);

    self->ticks_size += 1;

    self->memory_ticks[self->ticks_size - 1] = new_tick;
}

jrn_memory_tick_t *jrn_memory_tick_create(size_t mem_alloced, size_t mem_freed)
{
    jrn_memory_tick_t *tick = malloc(sizeof(*tick));

    jrn_internal_signal(JRN_OK, "Adding memory tick");

    tick->memory_freed = mem_freed;
    tick->memory_allocated = mem_alloced;
    return tick;
}

void jrn_memory_tick_print(jrn_memory_tick_t *self)
{
    printf("[Alloced:%ld\t] [Freed:%ld\t]\n", self->memory_allocated, self->memory_freed);
}

void jrn_memory_summary_print(jrn_memory_t *self)
{
    printf("ticks_alloced:\t%ld\n"
           "ticks_size\t\t%ld\n"
           "cur_alloced\t%ld\n"
           "cur_freed\t\t%ld\n",
           self->ticks_alloced, self->ticks_size, self->cur_alloced, self->cur_freed);

    for (size_t i = 0; i != self->ticks_size; i++)
    {
        jrn_memory_tick_print(self->memory_ticks[i]);
    }
}

void jrn_memory_fill_graph(jrn_memory_t *self, char *buf)
{

    const size_t limit = (size_t)JRN_MEM_GRAPH_LIMIT;

    size_t i = 0;
    if (self->ticks_size >= limit)
    {
        i = self->ticks_size - limit;
    }

    for (; i != self->ticks_size; i++)
    {
        char tick[JRN_BUF_16] = "";
        char alloced[JRN_BUF_16] = "";
        char freed[JRN_BUF_16] = "";

        jrn_itoa_u(tick, sizeof(tick), i, 10);
        jrn_itoa_u(alloced, sizeof(alloced), self->memory_ticks[i]->memory_allocated, 10);
        jrn_itoa_u(freed, sizeof(freed), self->memory_ticks[i]->memory_freed, 10);

        str_n_cat(buf, "[");
        str_n_cat(buf, tick);
        str_n_cat(buf, ", ");

        str_n_cat(buf, alloced);
        str_n_cat(buf, ", ");
        str_n_cat(buf, freed);

        if (i == self->ticks_size - 1)
            str_n_cat(buf, "]");
        else
            str_n_cat(buf, "],");
    }
}