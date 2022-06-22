#include <math.h>

#include "constants.h"
#include "nav_math.h"
#include "gpsAPI.h"
// #include "model.h"
#define matrix_at(A, x, y, rownum)    ((A)[(x) + (rownum) * (y)]) 

void mat_vec_mult3d(const double* M, const double* v, double* mv)
{
	for (int32_t i = 0; i < 3; i++) {
		mv[i] = 0.0;
		for (int32_t j = 0; j < 3; j++) {
			mv[i] += matrix_at(M, i, j, 3) * v[j];
		}
	}
}

void matmulvec3x1(const double M[3][3], const double *v, double *mv)
{
    mv[0] = M[0][0] * v[0] + M[1][0] * v[1] + M[2][0] * v[2];
	mv[1] = M[0][1] * v[0] + M[1][1] * v[1] + M[2][1] * v[2];
	mv[2] = M[0][2] * v[0] + M[1][2] * v[1] + M[2][2] * v[2];
}

void ecef2ned(const double *xyz, double *ned)
{
    double C_en[3][3];
    double blh[3] = {0.0};

    ecef2pos(xyz, blh);
    blh2C_en(blh, C_en);

	ned[0] = C_en[0][0] * xyz[0] + C_en[1][0] * xyz[1] + C_en[2][0] * xyz[2];
	ned[1] = C_en[0][1] * xyz[0] + C_en[1][1] * xyz[1] + C_en[2][1] * xyz[2];
	ned[2] = C_en[0][2] * xyz[0] + C_en[1][2] * xyz[1] + C_en[2][2] * xyz[2];
}

