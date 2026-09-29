/*
 * PID.h
 *
 *  Created on: Apr 16, 2019
 *      Author: gilg
 */

#ifndef PID_H_
#define PID_H_

#include <stdint.h>

typedef struct
{
    float PID[3];
    float P[3];
    float I[3];
    float D[3];
} copter;

void pid_update(const uint16_t *rc_pulse, const double *current_angle, uint16_t *motor_power);

#endif /* PID_H_ */
