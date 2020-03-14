/*
 * File: adapt_gps3.c
 *
 * MATLAB Coder version            : 2.7
 * C/C++ source code generated on  : 15-Apr-2019 16:08:52
 */

/* Include Files */
#include "rt_nonfinite.h"
#include "adapt_gps3.h"

/* Variable Definitions */
static double dt;
static double g;
static boolean_T g_not_empty;
static double q[4];
static double tlpf;
static double v1f[3];
static double v2f[3];
static double P[9];
static double Q[9];
static double a[4];

/* Function Declarations */
static void adapt_gps3_init(void);
static void diag(const double v[3], double d[9]);
static void mpower(const double b_a[9], double c[9]);
static double rt_atan2d_snf(double u0, double u1);

/* Function Definitions */

/*
 * Arguments    : void
 * Return Type  : void
 */
static void adapt_gps3_init(void)
{
  int i;
  static const double dv2[4] = { 0.01, 10.0, 0.0, 0.05 };

  for (i = 0; i < 4; i++) {
    a[i] = dv2[i];
  }

  g = 9.81;
  dt = 0.01;
  tlpf = 0.1;
  for (i = 0; i < 3; i++) {
    v1f[i] = 0.0;
    v2f[i] = 0.0;
  }
}

/*
 * Arguments    : const double v[3]
 *                double d[9]
 * Return Type  : void
 */
static void diag(const double v[3], double d[9])
{
  int j;
  memset(&d[0], 0, 9U * sizeof(double));
  for (j = 0; j < 3; j++) {
    d[j + 3 * j] = v[j];
  }
}

/*
 * Arguments    : const double b_a[9]
 *                double c[9]
 * Return Type  : void
 */
static void mpower(const double b_a[9], double c[9])
{
  double x[9];
  int p1;
  int p2;
  int p3;
  double absx11;
  double absx21;
  double absx31;
  int itmp;
  double y;
  memcpy(&x[0], &b_a[0], 9U * sizeof(double));
  p1 = 0;
  p2 = 3;
  p3 = 6;
  absx11 = fabs(b_a[0]);
  absx21 = fabs(b_a[1]);
  absx31 = fabs(b_a[2]);
  if ((absx21 > absx11) && (absx21 > absx31)) {
    p1 = 3;
    p2 = 0;
    x[0] = b_a[1];
    x[1] = b_a[0];
    x[3] = b_a[4];
    x[4] = b_a[3];
    x[6] = b_a[7];
    x[7] = b_a[6];
  } else {
    if (absx31 > absx11) {
      p1 = 6;
      p3 = 0;
      x[0] = b_a[2];
      x[2] = b_a[0];
      x[3] = b_a[5];
      x[5] = b_a[3];
      x[6] = b_a[8];
      x[8] = b_a[6];
    }
  }

  absx21 = x[1] / x[0];
  x[1] /= x[0];
  absx11 = x[2] / x[0];
  x[2] /= x[0];
  x[4] -= absx21 * x[3];
  x[5] -= absx11 * x[3];
  x[7] -= absx21 * x[6];
  x[8] -= absx11 * x[6];
  if (fabs(x[5]) > fabs(x[4])) {
    itmp = p2;
    p2 = p3;
    p3 = itmp;
    x[1] = absx11;
    x[2] = absx21;
    absx11 = x[4];
    x[4] = x[5];
    x[5] = absx11;
    absx11 = x[7];
    x[7] = x[8];
    x[8] = absx11;
  }

  absx31 = x[5];
  y = x[4];
  absx21 = x[5] / x[4];
  x[8] -= absx21 * x[7];
  absx11 = (absx21 * x[1] - x[2]) / x[8];
  absx21 = -(x[1] + x[7] * absx11) / x[4];
  c[p1] = ((1.0 - x[3] * absx21) - x[6] * absx11) / x[0];
  c[p1 + 1] = absx21;
  c[p1 + 2] = absx11;
  absx11 = -(absx31 / y) / x[8];
  absx21 = (1.0 - x[7] * absx11) / x[4];
  c[p2] = -(x[3] * absx21 + x[6] * absx11) / x[0];
  c[p2 + 1] = absx21;
  c[p2 + 2] = absx11;
  absx11 = 1.0 / x[8];
  absx21 = -x[7] * absx11 / x[4];
  c[p3] = -(x[3] * absx21 + x[6] * absx11) / x[0];
  c[p3 + 1] = absx21;
  c[p3 + 2] = absx11;
}

/*
 * Arguments    : double u0
 *                double u1
 * Return Type  : double
 */
static double rt_atan2d_snf(double u0, double u1)
{
  double y;
  int b_u0;
  int b_u1;
  if (rtIsNaN(u0) || rtIsNaN(u1)) {
    y = rtNaN;
  } else if (rtIsInf(u0) && rtIsInf(u1)) {
    if (u0 > 0.0) {
      b_u0 = 1;
    } else {
      b_u0 = -1;
    }

    if (u1 > 0.0) {
      b_u1 = 1;
    } else {
      b_u1 = -1;
    }

    y = atan2(b_u0, b_u1);
  } else if (u1 == 0.0) {
    if (u0 > 0.0) {
      y = RT_PI / 2.0;
    } else if (u0 < 0.0) {
      y = -(double)(RT_PI / 2.0);
    } else {
      y = 0.0;
    }
  } else {
    y = atan2(u0, u1);
  }

  return y;
}

/*
 * Arguments    : const double zom[3]
 *                const double za[3]
 *                const double Vsns[3]
 *                boolean_T status_sns
 *                double B_SNS
 *                double *psi
 *                double *theta
 *                double *b_gamma
 * Return Type  : void
 */
void adapt_gps3(const double zom[3], const double za[3], const double Vsns[3],
                boolean_T status_sns, double B_SNS, double *psi, double *theta,
                double *b_gamma)
{
  double b_za[3];
  int i;
  double qm;
  double R;
  double alfa[3];
  double dv0[16];
  double dv1[4];
  int i0;
  double C_B_N[9];
  double Vxyz_sns[3];
  double Hk[9];
  double b_R[9];
  double b_Hk[9];
  double y[9];
  int i1;
  double c_Hk[9];
  double b[9];
  double K[9];
  double b_y[9];
  double c_y[9];
  double d_y;
  double e_y;
  double Zcount[3];
  double c_za[3];
  if (!g_not_empty) {
    g_not_empty = true;

    /*      q=[1 0 0 0]'; */
    /*  a=[ake1; ake2; an1; an2] */
    /* 'ideal_data.txt' ( при dw=60 deg/h ) */
    /*      Q=diag([2.91e-4; 2.91e-4; 2.91e-3].^2);   */
    /*      P=diag([0.01; 0.01; 0.01]); */
    /*  для 'mnk_19_09_18.txt' */
    for (i = 0; i < 3; i++) {
      b_za[i] = 7.6212899999999993E-7;
    }

    diag(b_za, Q);
    for (i = 0; i < 3; i++) {
      b_za[i] = 0.05;
    }

    diag(b_za, P);

    /*  постоянная времени ФНЧ */
    /*  для 'flight_8_22103_82103.txt' */
    /*  Q=diag([0.1e-6; 0.1e-6; 0.1e-6]); */
    /*      P=diag([0.005; 0.005; 0.005]); */
    /*      tlpf=0.01; % постоянная времени ФНЧ */
    /*  'ideal_data.txt' */
    /*      gamma=0.574*pi/180; */
    /*      theta=-2.034*pi/180; */
    /*      theta=0; */
    /*      gamma=0; */
    /*      psi=0; */
    *b_gamma = rt_atan2d_snf(-za[2], za[1]);
    *theta = atan(za[0] / sqrt(za[1] * za[1] + za[2] * za[2]));
    qm = sin(*theta);
    R = cos(*theta);
    *psi = rt_atan2d_snf(((zom[0] * sin(*b_gamma) * sin(*theta) - zom[2] * cos
      (*theta)) - sin(*b_gamma) * (qm * qm) * 7.292115E-5 * sin(B_SNS)) - sin
                         (*b_gamma) * (R * R) * 7.292115E-5 * sin(B_SNS), zom[0]
                         * cos(*b_gamma) - cos(*b_gamma) * sin(*theta) *
                         7.292115E-5 * sin(B_SNS));
    for (i = 0; i < 4; i++) {
      q[i] = 0.0;
    }

    q[0] = cos(*psi / 2.0) * cos(*theta / 2.0) * cos(*b_gamma / 2.0) + sin(*psi /
      2.0) * sin(*theta / 2.0) * sin(*b_gamma / 2.0);
    q[1] = cos(*psi / 2.0) * cos(*theta / 2.0) * sin(*b_gamma / 2.0) - sin(*psi /
      2.0) * sin(*theta / 2.0) * cos(*b_gamma / 2.0);
    q[2] = cos(*psi / 2.0) * sin(*theta / 2.0) * sin(*b_gamma / 2.0) - sin(*psi /
      2.0) * cos(*theta / 2.0) * cos(*b_gamma / 2.0);
    q[3] = cos(*psi / 2.0) * sin(*theta / 2.0) * cos(*b_gamma / 2.0) + sin(*psi /
      2.0) * cos(*theta / 2.0) * sin(*b_gamma / 2.0);
  }

  alfa[0] = zom[0] * dt;
  alfa[1] = zom[1] * dt;
  alfa[2] = zom[2] * dt;
  dv0[0] = 0.0;
  dv0[4] = -alfa[0];
  dv0[8] = -alfa[1];
  dv0[12] = -alfa[2];
  dv0[1] = alfa[0];
  dv0[5] = 0.0;
  dv0[9] = alfa[2];
  dv0[13] = -alfa[1];
  dv0[2] = alfa[1];
  dv0[6] = -alfa[2];
  dv0[10] = 0.0;
  dv0[14] = alfa[0];
  dv0[3] = alfa[2];
  dv0[7] = alfa[1];
  dv0[11] = -alfa[0];
  dv0[15] = 0.0;
  for (i = 0; i < 4; i++) {
    R = 0.0;
    for (i0 = 0; i0 < 4; i0++) {
      R += dv0[i + (i0 << 2)] * q[i0];
    }

    dv1[i] = q[i] + R / 2.0;
  }

  for (i = 0; i < 4; i++) {
    q[i] = dv1[i];
  }

  /*  умножили на dt в alfa */
  qm = sqrt(((q[0] * q[0] + q[1] * q[1]) + q[2] * q[2]) + q[3] * q[3]);
  if (qm > 1.0E-7) {
    for (i = 0; i < 4; i++) {
      q[i] /= qm;
    }
  }

  /*  Матрица перехода из ССК в НСК */
  C_B_N[0] = ((q[0] * q[0] + q[1] * q[1]) - q[2] * q[2]) - q[3] * q[3];
  C_B_N[3] = 2.0 * (q[1] * q[2] - q[0] * q[3]);
  C_B_N[6] = 2.0 * (q[1] * q[3] + q[0] * q[2]);
  C_B_N[1] = 2.0 * (q[2] * q[1] + q[0] * q[3]);
  C_B_N[4] = ((q[0] * q[0] - q[1] * q[1]) + q[2] * q[2]) - q[3] * q[3];
  C_B_N[7] = 2.0 * (q[2] * q[3] - q[0] * q[1]);
  C_B_N[2] = 2.0 * (q[3] * q[1] - q[0] * q[2]);
  C_B_N[5] = 2.0 * (q[2] * q[3] + q[0] * q[1]);
  C_B_N[8] = ((q[0] * q[0] - q[1] * q[1]) - q[2] * q[2]) + q[3] * q[3];
  *theta = atan(C_B_N[1] / sqrt(C_B_N[4] * C_B_N[4] + C_B_N[7] * C_B_N[7]));
  *b_gamma = -rt_atan2d_snf(C_B_N[7], C_B_N[4]);
  *psi = rt_atan2d_snf(C_B_N[2], C_B_N[0]);
  if (status_sns) {
    /*  Расчетные параметры БИНС */
    /*      Yq=[theta gamma psi]'; */
    for (i = 0; i < 3; i++) {
      Vxyz_sns[i] = 0.0;
      for (i0 = 0; i0 < 3; i0++) {
        Vxyz_sns[i] += C_B_N[i0 + 3 * i] * Vsns[i0];
      }
    }

    /* Матрица наблюдений */
    Hk[0] = cos(*theta) + (zom[1] * ((Vsns[2] * (sin(*b_gamma) * sin(*psi) * cos
                              (*theta)) + Vsns[0] * (cos(*psi) * sin(*b_gamma) *
      cos(*theta))) + Vsns[1] * sin(*theta) * sin(*b_gamma)) - zom[2] * ((Vsns[2]
      * (-cos(*b_gamma) * sin(*psi) * cos(*theta)) - Vsns[0] * (cos(*b_gamma) *
      cos(*psi) * cos(*theta))) - Vsns[1] * cos(*b_gamma) * sin(*theta))) / g;
    Hk[3] = (zom[1] * ((Vsns[2] * (-sin(*b_gamma) * cos(*psi) + cos(*b_gamma) *
                sin(*psi) * sin(*theta)) + Vsns[0] * (sin(*b_gamma) * sin(*psi)
                + cos(*psi) * cos(*b_gamma) * sin(*theta))) - Vsns[1] * cos
                       (*theta) * cos(*b_gamma)) - zom[2] * ((Vsns[2] * (cos
                (*psi) * cos(*b_gamma) + sin(*b_gamma) * sin(*psi) * sin(*theta))
               - Vsns[0] * (cos(*b_gamma) * sin(*psi) - sin(*b_gamma) * cos(*psi)
                * sin(*theta))) - Vsns[1] * sin(*b_gamma) * cos(*theta))) / g;
    Hk[6] = (zom[1] * (Vsns[2] * (-cos(*b_gamma) * sin(*psi) + sin(*b_gamma) *
               cos(*psi) * sin(*theta)) - Vsns[0] * (cos(*b_gamma) * cos(*psi) +
               sin(*psi) * sin(*b_gamma) * sin(*theta))) - zom[2] * (Vsns[2] * (
               -sin(*psi) * sin(*b_gamma) - cos(*b_gamma) * cos(*psi) * sin
               (*theta)) - Vsns[0] * (sin(*b_gamma) * cos(*psi) - cos(*b_gamma) *
               sin(*psi) * sin(*theta)))) / g;
    Hk[1] = -cos(*b_gamma) * sin(*theta) + (zom[2] * ((Vsns[1] * cos(*theta) -
      Vsns[0] * cos(*psi) * sin(*theta)) - Vsns[2] * sin(*theta) * sin(*psi)) -
      zom[0] * ((Vsns[2] * (sin(*b_gamma) * sin(*psi) * cos(*theta)) + Vsns[0] *
                 (cos(*psi) * sin(*b_gamma) * cos(*theta))) + Vsns[1] * sin
                (*theta) * sin(*b_gamma))) / g;
    Hk[4] = -sin(*b_gamma) * cos(*theta) - zom[0] * ((Vsns[2] * (-sin(*b_gamma) *
      cos(*psi) + cos(*b_gamma) * sin(*psi) * sin(*theta)) + Vsns[0] * (sin
      (*b_gamma) * sin(*psi) - cos(*psi) * cos(*b_gamma) * sin(*theta))) - Vsns
      [1] * cos(*theta) * cos(*b_gamma)) / g;
    Hk[7] = (zom[2] * (-Vsns[0] * cos(*psi) * sin(*theta) + Vsns[2] * cos(*theta)
                       * cos(*psi)) - zom[0] * (Vsns[2] * (-cos(*b_gamma) * sin(*
                psi) + sin(*b_gamma) * cos(*psi) * sin(*theta)) - Vsns[0] * (cos
               (*b_gamma) * cos(*psi) + sin(*psi) * sin(*b_gamma) * sin(*theta))))
      / g;
    Hk[2] = sin(*b_gamma) * sin(*theta) + (zom[0] * ((Vsns[2] * (-cos(*b_gamma) *
      sin(*psi) * cos(*theta)) - Vsns[0] * (cos(*b_gamma) * cos(*psi) * cos
      (*theta))) - Vsns[1] * cos(*b_gamma) * sin(*theta)) - zom[1] * ((Vsns[1] *
      cos(*theta) - Vsns[0] * cos(*psi) * sin(*theta)) - Vsns[2] * sin(*theta) *
      sin(*psi))) / g;
    Hk[5] = -cos(*b_gamma) * cos(*theta) + zom[0] * ((Vsns[2] * (cos(*psi) * cos
      (*b_gamma) + sin(*b_gamma) * sin(*psi) * sin(*theta)) - Vsns[0] * (cos
      (*b_gamma) * sin(*psi) - sin(*b_gamma) * cos(*psi) * sin(*theta))) - Vsns
      [1] * sin(*b_gamma) * cos(*theta)) / g;
    Hk[8] = (zom[0] * (Vsns[2] * (-sin(*psi) * sin(*b_gamma) - cos(*b_gamma) *
               cos(*psi) * sin(*theta)) - Vsns[0] * (sin(*b_gamma) * cos(*psi) -
               cos(*b_gamma) * sin(*psi) * sin(*theta))) - zom[1] * (-Vsns[0] *
              sin(*psi) * cos(*theta) + Vsns[2] * cos(*theta) * cos(*psi))) / g;

    /*  a=[ake1; ake2; an1; an2] */
    R = a[1];
    qm = fabs(sqrt((za[0] * za[0] + za[1] * za[1]) + za[2] * za[2]) - 1.0);
    if (qm < a[3]) {
      R = a[0] + (a[1] - a[0]) / (a[3] - a[2]) * (qm - a[2]);
    }

    alfa[0] = R;
    alfa[1] = R;
    alfa[2] = R;
    memset(&b_R[0], 0, 9U * sizeof(double));
    for (i = 0; i < 3; i++) {
      b_R[i + 3 * i] = alfa[i];
    }

    for (i = 0; i < 9; i++) {
      P[i] += Q[i];
    }

    for (i = 0; i < 3; i++) {
      for (i0 = 0; i0 < 3; i0++) {
        y[i + 3 * i0] = 0.0;
        for (i1 = 0; i1 < 3; i1++) {
          y[i + 3 * i0] += P[i + 3 * i1] * Hk[i0 + 3 * i1];
        }

        b_Hk[i + 3 * i0] = 0.0;
        for (i1 = 0; i1 < 3; i1++) {
          b_Hk[i + 3 * i0] += Hk[i + 3 * i1] * P[i1 + 3 * i0];
        }
      }
    }

    for (i = 0; i < 3; i++) {
      for (i0 = 0; i0 < 3; i0++) {
        R = 0.0;
        for (i1 = 0; i1 < 3; i1++) {
          R += b_Hk[i + 3 * i1] * Hk[i0 + 3 * i1];
        }

        c_Hk[i + 3 * i0] = R + b_R[i + 3 * i0];
      }
    }

    mpower(c_Hk, b);
    for (i = 0; i < 3; i++) {
      for (i0 = 0; i0 < 3; i0++) {
        K[i + 3 * i0] = 0.0;
        for (i1 = 0; i1 < 3; i1++) {
          K[i + 3 * i0] += y[i + 3 * i1] * b[i1 + 3 * i0];
        }
      }
    }

    memset(&y[0], 0, 9U * sizeof(double));
    for (i = 0; i < 3; i++) {
      y[i + 3 * i] = 1.0;
    }

    memset(&b[0], 0, 9U * sizeof(double));
    for (i = 0; i < 3; i++) {
      b[i + 3 * i] = 1.0;
    }

    for (i = 0; i < 3; i++) {
      for (i0 = 0; i0 < 3; i0++) {
        R = 0.0;
        for (i1 = 0; i1 < 3; i1++) {
          R += K[i + 3 * i1] * Hk[i1 + 3 * i0];
        }

        b_y[i + 3 * i0] = y[i + 3 * i0] - R;
      }
    }

    for (i = 0; i < 3; i++) {
      for (i0 = 0; i0 < 3; i0++) {
        c_y[i + 3 * i0] = 0.0;
        for (i1 = 0; i1 < 3; i1++) {
          c_y[i + 3 * i0] += b_y[i + 3 * i1] * P[i1 + 3 * i0];
        }
      }
    }

    for (i = 0; i < 3; i++) {
      for (i0 = 0; i0 < 3; i0++) {
        R = 0.0;
        for (i1 = 0; i1 < 3; i1++) {
          R += K[i0 + 3 * i1] * Hk[i1 + 3 * i];
        }

        c_Hk[i + 3 * i0] = b[i0 + 3 * i] - R;
      }
    }

    for (i = 0; i < 3; i++) {
      for (i0 = 0; i0 < 3; i0++) {
        b_Hk[i + 3 * i0] = 0.0;
        for (i1 = 0; i1 < 3; i1++) {
          b_Hk[i + 3 * i0] += K[i + 3 * i1] * b_R[i1 + 3 * i0];
        }

        b_y[i + 3 * i0] = 0.0;
        for (i1 = 0; i1 < 3; i1++) {
          b_y[i + 3 * i0] += c_y[i + 3 * i1] * c_Hk[i1 + 3 * i0];
        }
      }

      for (i0 = 0; i0 < 3; i0++) {
        Hk[i + 3 * i0] = 0.0;
        for (i1 = 0; i1 < 3; i1++) {
          Hk[i + 3 * i0] += b_Hk[i + 3 * i1] * K[i0 + 3 * i1];
        }
      }
    }

    qm = -1.0 / tlpf;
    R = 1.0 / tlpf;
    d_y = -1.0 / tlpf;
    e_y = 1.0 / tlpf;
    for (i = 0; i < 3; i++) {
      for (i0 = 0; i0 < 3; i0++) {
        P[i0 + 3 * i] = b_y[i0 + 3 * i] + Hk[i0 + 3 * i];
      }

      alfa[i] = d_y * v2f[i] + e_y * v1f[i];
      v1f[i] += (qm * v1f[i] + R * Vxyz_sns[i]) * dt;
      v2f[i] += alfa[i] * dt;
    }

    Zcount[0] = sin(*theta) + ((alfa[0] + Vxyz_sns[2] * zom[1]) - zom[2] *
      Vxyz_sns[1]) / g;
    Zcount[1] = cos(*theta) * cos(*b_gamma) + ((alfa[1] + Vxyz_sns[0] * zom[2])
      - zom[0] * Vxyz_sns[2]) / g;
    Zcount[2] = -sin(*b_gamma) * cos(*theta) + ((alfa[2] + Vxyz_sns[1] * zom[0])
      - zom[1] * Vxyz_sns[0]) / g;
    c_za[0] = za[0];
    c_za[1] = za[1];
    c_za[2] = za[2];
    for (i = 0; i < 3; i++) {
      b_za[i] = c_za[i] - Zcount[i];
    }

    for (i = 0; i < 3; i++) {
      alfa[i] = 0.0;
      for (i0 = 0; i0 < 3; i0++) {
        alfa[i] += K[i + 3 * i0] * b_za[i0];
      }
    }

    /*  Оценка параметров ориентации */
    /*      Yf=Yq + KNu; */
    *theta += alfa[0];
    *b_gamma = -rt_atan2d_snf(C_B_N[7], C_B_N[4]) + alfa[1];
    *psi += alfa[2];

    /* ----------------- конец фильтра--------------------------- */
    /*  Пересчет кватерниона  */
    q[0] = cos(*psi / 2.0) * cos(*theta / 2.0) * cos(*b_gamma / 2.0) + sin(*psi /
      2.0) * sin(*theta / 2.0) * sin(*b_gamma / 2.0);
    q[1] = cos(*psi / 2.0) * cos(*theta / 2.0) * sin(*b_gamma / 2.0) - sin(*psi /
      2.0) * sin(*theta / 2.0) * cos(*b_gamma / 2.0);
    q[2] = cos(*psi / 2.0) * sin(*theta / 2.0) * sin(*b_gamma / 2.0) - sin(*psi /
      2.0) * cos(*theta / 2.0) * cos(*b_gamma / 2.0);
    q[3] = cos(*psi / 2.0) * sin(*theta / 2.0) * cos(*b_gamma / 2.0) + sin(*psi /
      2.0) * cos(*theta / 2.0) * sin(*b_gamma / 2.0);
    qm = sqrt(((q[0] * q[0] + q[1] * q[1]) + q[2] * q[2]) + q[3] * q[3]);
    for (i = 0; i < 4; i++) {
      q[i] /= qm;
    }
  }
}

/*
 * Arguments    : void
 * Return Type  : void
 */
void adapt_gps3_initialize(void)
{
  rt_InitInfAndNaN(8U);
  g_not_empty = false;
  adapt_gps3_init();
}

/*
 * Arguments    : void
 * Return Type  : void
 */
void adapt_gps3_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for adapt_gps3.c
 *
 * [EOF]
 */
