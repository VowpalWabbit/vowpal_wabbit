#ifndef MEMORY_H
#define MEMORY_H

void* calloc_or_die(size_t nmemb, size_t size);

void free_it(void* ptr);

#endif
