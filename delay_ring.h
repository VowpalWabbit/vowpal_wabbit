#ifndef DELAY_RING_H
#define DELAY_RING_H

void initialize_delay_ring();
void destroy_delay_ring();
example* get_delay_example(size_t thread);
example* blocking_get_delay_example(size_t thread);
void delay_example(example* ec);
bool thread_done(size_t thread);

#endif
