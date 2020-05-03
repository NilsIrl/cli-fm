#include "vector.h"

void vector_init_with_capacity(struct vector *vector, const size_t capacity) {
	vector->capacity = capacity;
	vector->length = 0;
	vector->vector = malloc(vector->capacity * sizeof(*vector->vector));
}

void vector_init(struct vector *vector) {
	vector_init_with_capacity(vector, 10);
}

void vector_reallocate_if_full(struct vector *vector) {
	if (vector->capacity == vector->length) {
		vector->capacity *= 2;
		vector->vector = realloc(vector->vector, vector->capacity * sizeof(*vector->vector));
	}
}

void vector_push(struct vector *vector, void *item) {
	vector_reallocate_if_full(vector);
	vector->vector[vector->length++] = item;
}

