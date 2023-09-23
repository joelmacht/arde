#include "vector.h"

void arde_vector_add(const arde_vector_t* first, const arde_vector_t* second, arde_vector_t* result)
{
    result->data[0] = first->data[0] + second->data[0];
    result->data[1] = first->data[1] + second->data[1];
}

void arde_vector_multiply(const arde_vector_t* first, const arde_vector_t* second, arde_vector_t* result)
{
    result->data[0] = first->data[0] * second->data[0];
    result->data[1] = first->data[1] * second->data[1];
}

void arde_matrix_vector_multiply(const arde_matrix_t* matrix, const arde_vector_t* vector, arde_vector_t* result)
{
    result->data[0] = vector->data[0] * matrix->data[0] + vector->data[1] * matrix->data[1];
    result->data[1] = vector->data[0] * matrix->data[2] + vector->data[1] * matrix->data[3];
}

void arde_transform_position(const arde_transform_t* transform, const arde_vector_t* position, arde_vector_t* result)
{
    arde_matrix_vector_multiply(&transform->rotation, position, result);
    arde_vector_t scratchboard;
    arde_vector_multiply(&transform->scaling, result, &scratchboard);
    arde_vector_add(&transform->translation, &scratchboard, result);
}

void arde_transform_direction(const arde_transform_t* transform, const arde_vector_t* direction, arde_vector_t* result)
{
    arde_vector_t scratchboard;
    arde_matrix_vector_multiply(&transform->rotation, direction, &scratchboard);
    arde_vector_multiply(&transform->scaling, &scratchboard, result);
}
