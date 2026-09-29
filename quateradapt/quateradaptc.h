#ifndef QUATERADAPTC_H
#define QUATERADAPTC_H

/*
 * Compatibility interface for a legacy quaternion estimator whose
 * implementation is intentionally omitted from this code sample.
 */

extern double gamma1;
extern double teta;
extern double psi;
extern double V;

extern double Wx;
extern double Wy;
extern double Wz;
extern double Nx;
extern double Ny;
extern double Nz;

void QuaterAdapt_initDT(double sample_period_s);
void QuaterAdapt_initNU(void);
void QuaterAdapt_QuaterAdapt(void);
void QuaterAdapt_doCalc(void);

#endif /* QUATERADAPTC_H */
