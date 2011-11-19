#ifndef DELAY_RING_H
#define DELAY_RING_H

void initialize_delay_ring();
void destroy_delay_ring();
example* get_delay_example();
void delay_example(example* ex, size_t count);
bool thread_done();

#endif
