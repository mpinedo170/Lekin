#pragma once

/////////////////////////////////////////////////////////////////////////////
// Mathematical global functions and constants

namespace AFLibMath {  // not-a-number in various types
AFLIB extern const double Nan;
AFLIB extern const float NanF;
AFLIB extern const int NanI;
AFLIB extern const UINT NanUI;
AFLIB extern const INT64 NanI64;
AFLIB extern const UINT64 NanUI64;
AFLIB extern const double Const100;

// invert matrix
AFLIB bool MatrixInvert(int n, const double* a, double* b);

// Cholesky decomposition of matrix a into matrix b
AFLIB bool Cholesky(int n, double* a, double* b);

// delete row and column from a matrix
AFLIB void MatrixDelete(int n, double* a, int rc);

// normal distribution density function
AFLIB double NormalDensity(double x);

// normal distribution cumulative function
AFLIB double NormalDistribution(double x);

// round the value to the closest value with given number of decimal digits
AFLIB double Round(double value, int digits = 0);

// round the value to the closest integer
AFLIB int Rint(double value);

// is value undefined or infinity?
AFLIB bool IsNan(double a);

// is value undefined or infinity?
AFLIB bool IsNan(float a);

// is value undefined or infinity?
inline bool IsNan(int a)
{
    return a == NanI;
}

// is value undefined or infinity?
inline bool IsNan(UINT a)
{
    return a == NanUI;
}

// is value undefined or infinity?
inline bool IsNan(INT64 a)
{
    return a == NanI64;
}

// is value undefined or infinity?
inline bool IsNan(UINT64 a)
{
    return a == NanUI64;
}

// sign of a numeric value: 0, 1, or -1
template <class T>
inline int sgn(T a)
{
    return a > 0 ? 1 : a < 0 ? -1 : 0;
}
}  // namespace AFLibMath
