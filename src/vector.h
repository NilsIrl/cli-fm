#pragma once

#include <stdlib.h>

struct vector {
	void **vector;
	size_t length, capacity;
};

void vector_init(struct vector *vector);
void vector_push(struct vector *vector, void *item);
void vector_init_with_capacity(struct vector *vector, const size_t capacity);
