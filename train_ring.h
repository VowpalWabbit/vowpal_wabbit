#ifndef TRAIN_RING_H
#define TRAIN_RING_H

void initialize_train_ring();
void destroy_train_ring();
example* get_train_example(size_t thread);
void insert_example(example* ec);
bool thread_done(size_t thread);

#endif
