#include "StdAfx.h"

#include "AFLibMathGlobal.h"

#include "AFLibBitMask.h"

using namespace AFLib;

// Inversion b = a^(-1), where a is a (n*n) matrix;
// On error (matrix is singular), returns false, else returns true;
// a and b may occupy same memory space!

namespace AFLibMath {
bool MatrixInvert(int n, const double* a, double* b)
{
    TBitMask bmFinished(0, n);
    int counter, i, i0, i1, ii, j, jj, k, kk, n1 = n - 1;
    static const double eps = 1e-99;
    double bij, bki, xn, xna;

    // first  and last 'non finished' index
    i0 = 0;
    i1 = n1;

    // main loop
    for (counter = 0; counter < n; ++counter)
    {  // find biggest denominator
        i = i0;
        xn = 0.;
        jj = i0 * n;

        for (j = i0; j <= i1; ++j)
        {
            if (!bmFinished.IsSelected(j))
            {
                xna = a[jj + j];
                kk = 0;

                for (k = 0; k < n; ++k)
                {
                    if (bmFinished.IsSelected(k)) xna += b[jj + k] * a[kk + j];
                    kk += n;
                }

                if (fabs(xna) > fabs(xn))
                {
                    xn = xna;
                    i = j;
                }
            }
            jj += n;
        }

        if (fabs(xn) < eps) return false;

        ii = i * n;
        kk = 0;

        // calculate ith column
        for (k = 0; k < n; ++k)
        {
            if (bmFinished.IsSelected(k))
                bki = 0.;
            else
                bki = a[kk + i];

            if (k == i) bki -= 1.;

            jj = 0;
            for (j = 0; j < n; ++j)
            {
                if (bmFinished.IsSelected(j)) bki += b[kk + j] * a[jj + i];
                jj += n;
            }

            b[kk + i] = -bki / xn;
            kk += n;
        }
        b[ii + i] += 1.;
        jj = 0;

        // eval remaining columns
        for (j = 0; j < n; ++j)
        {
            if (bmFinished.IsSelected(j))
            {
                bij = b[ii + j];
                kk = 0;

                for (k = 0; k < n; ++k)
                {
                    b[kk + j] += bij * b[kk + i];
                    kk += n;
                }
                b[ii + j] -= bij;
            }
            jj += n;
        }

        bmFinished.SetBit(i);
        while (i0 < n && bmFinished.IsSelected(i0)) ++i0;
        while (i1 >= 0 && bmFinished.IsSelected(i1)) --i1;
    }

    return true;
}

bool Cholesky(int n, double* a, double* b)
{
    memset(b, 0, n * n * sizeof(double));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < i + 1; ++j)
        {
            double s = 0;
            double v = 0;

            for (int k = 0; k < j; k++) s += b[i * n + k] * b[j * n + k];

            if (i == j)
            {
                v = a[i * n + i] - s;
                if (v < 0) return false;
                v = sqrt(v);
            }
            else
                v = 1.0 / b[j * n + j] * (a[i * n + j] - s);
            b[i * n + j] = v;
        }
    return true;
}

void MatrixDelete(int n, double* a, int rc)
{
    memmove(a + rc * n, a + (rc + 1) * n, (n - rc - 1) * n * sizeof(double));

    for (int i = 0; i < n - 1; ++i)
    {
        double* p = a + i * (n - 1) + rc;
        memmove(p, p + 1, ((n - 1 - i) * n - rc - 1) * sizeof(double));
    }
}
}  // namespace AFLibMath
