/*
 * Library:   lmfit (Levenberg-Marquardt least squares fitting)
 *
 * File:      lmcurve.c
 *
 * Contents:  Implements lmcurve_tyd(), a variant of lmcurve() that weighs
 *            data points y(t) with the inverse of the standard deviations dy.
 *
 * Copyright: Joachim Wuttke, Forschungszentrum Juelich GmbH (2004-2013)
 *
 * License:   see ../COPYING (FreeBSD)
 *
 * Homepage:  apps.jcns.fz-juelich.de/lmfit
 */

#include "compv/base/math/lmfit-6.1/lmcurve_tyd.h"
#include "compv/base/math/lmfit-6.1/lmmin.h"

typedef struct {
    const double* t;
    const double* y;
    const double* dy;
    double (*f)(const double t, const double* par);
} lmcurve_tyd_data_struct;

void lmcurve_tyd_evaluate(
    const double* par, const int m_dat, const void* data, double* fvec,
    int* info)
{
    lmcurve_tyd_data_struct* D = (lmcurve_tyd_data_struct*)data;
    int i;
    for (i = 0; i < m_dat; i++)
        fvec[i] = ( D->y[i] - D->f(D->t[i], par) ) / D->dy[i];
}

void lmcurve_tyd(
    const int n_par, double* par, const int m_dat,
    const double* t, const double* y, const double* dy,
    double (*f)(const double t, const double* par),
    const lm_control_struct* control, lm_status_struct* status)
{
    lmcurve_tyd_data_struct data = { t, y, dy, f };

    lmmin(n_par, par, m_dat, (const void*)&data, lmcurve_tyd_evaluate,
          control, status);
}
