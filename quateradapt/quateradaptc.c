#include "quateradaptc.h"

/*
 * Interface-only placeholder for the legacy quaternion estimator.
 *
 * The original experimental implementation is intentionally not included in
 * this code sample. The active navigation task uses the separately generated
 * adapt_gps3 estimator; these functions retain the historical project API.
 */

double gamma1 = 0.0;
double teta = 0.0;
double psi = 0.0;
double V = 0.0;

double Wx = 0.0;
double Wy = 0.0;
double Wz = 0.0;
double Nx = 0.0;
double Ny = 0.0;
double Nz = 0.0;

void QuaterAdapt_initDT(double sample_period_s)
{
    (void)sample_period_s;
}

void QuaterAdapt_initNU(void)
{
    gamma1 = 0.0;
    teta = 0.0;
    psi = 0.0;
    V = 0.0;
}

void QuaterAdapt_QuaterAdapt(void)
{
    QuaterAdapt_initNU();
}

void QuaterAdapt_doCalc(void)
{
}
