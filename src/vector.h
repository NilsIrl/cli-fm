#pragma once

#define vector(T, N) \
	struct vector_##N { \
		T *vector; \
		size_t length, capacity; \
	}; \
	void vector_##N##_init_with_capacity(struct vector_##N *vector, \
			const size_t capacity) { \
		vector->capacity = capacity; \
		vector->length = 0; \
		vector->vector = malloc(vector->capacity * sizeof(T)); \
	} \
	void vector_##N##_init(struct vector_##N *vector) { \
		vector_##N##_init_with_capacity(vector, 10); \
	} \
	void vector_##N##_reallocate_if_full(struct vector_##N *vector) { \
		if (vector->capacity == vector->length) { \
			vector->capacity *= 2; \
			vector->vector = realloc(vector->vector, vector->capacity * sizeof(T)); \
		} \
	} \
	void vector_##N##_push(struct vector_##N *vector, T item) { \
		vector_##N##_reallocate_if_full(vector); \
		vector->vector[vector->length++] = item; \
	}
