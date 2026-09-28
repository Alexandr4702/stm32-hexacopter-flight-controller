/*
 * PID.h
 *
 *  Created on: Apr 16, 2019
 *      Author: gilg
 */

#ifndef PID_H_
#define PID_H_

typedef struct
{
    float PID[3];
    float P[3];
    float I[3];
    float D[3];
} copter;

void PWM_TO_ANGLE(__IO uint16_t *uhDutyCycle, float *angle);
void PID__(__IO uint16_t *uhDutyCycle, double *current_angle, uint16_t *motor_power);
void angle_to_pwm(float *PID, __IO uint16_t *uhDutyCycle, uint16_t *motor_power);

#endif /* PID_H_ */
