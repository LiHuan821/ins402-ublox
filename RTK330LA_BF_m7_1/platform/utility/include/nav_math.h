
#ifndef _NAV_MATH_H
#define _NAV_MATH_H

#include <stdint.h>

void mat_vec_mult3d(const double* M, const double* v, double* mv);
void matmulvec3x1(const double M[3][3], const double *v, double *mv);

#endif /* _NAV_MATH_H */