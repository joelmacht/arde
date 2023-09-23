#ifndef ARDE_MATH_VECTOR_H_INCLUDED
#define ARDE_MATH_VECTOR_H_INCLUDED

typedef struct {
    float data[2];
} arde_vector_t;

void arde_vector_add(const arde_vector_t* first, const arde_vector_t* second, arde_vector_t* result);
void arde_vector_multiply(const arde_vector_t* first, const arde_vector_t* second, arde_vector_t* result);

typedef struct {
    float data[4];
} arde_matrix_t;

void arde_matrix_vector_multiply(const arde_matrix_t* matrix, const arde_vector_t* vector, arde_vector_t* result);

typedef struct {
    arde_matrix_t rotation;
    arde_vector_t scaling;
    arde_vector_t translation;
} arde_transform_t;

void arde_transform_position(const arde_transform_t* transform, const arde_vector_t* position, arde_vector_t* result);
void arde_transform_direction(const arde_transform_t* transform, const arde_vector_t* direction, arde_vector_t* result);

#endif // ARDE_MATH_VECTOR_H_INCLUDED
