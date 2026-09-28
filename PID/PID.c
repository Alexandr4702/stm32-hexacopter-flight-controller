/*
 * PID.c
 *
 *  Created on: Apr 16, 2019
 *      Author: gilg
 */

#include "main.h"
#include "PID.h"
#include "cmsis_os.h"

extern QueueHandle_t copter_queue;

const float dt = 0.01f;

/*motor */

#define c_0 0.12 /* 180/500 */
#define c_1 0.06 /*30/500   */
#define c_2 0.06 /*30/500   */

void PWM_TO_ANGLE(__IO uint16_t *uhDutyCycle, float *angle)
{
    angle[1] = (uhDutyCycle[3] - 1500) * c_0; // omega_psi 		y
    angle[2] = (uhDutyCycle[0] - 1500) * c_1; // theta 	z
    angle[0] = (uhDutyCycle[1] - 1500) * c_2; // gamma 	x
}

void PID__(__IO uint16_t *uhDutyCycle, double *current_angle, uint16_t *motor_power)
{
    static float P_c[3] = {4.0f, 4.3f, 3.0f};
    static float I_c[3] = {0.8f, 0.0f, 0.80f};
    static float D_c[3] = {0.9f, 0.0f, 1.0f};
    static float angle[3] = {0.0f, 0.0f, 0.0f};
    static float p_error[3] = {0.0f, 0.0f, 0.0f};
    float error[3];
    float P[3];
    static float I[3];
    float D[3];
    float PID[3];

    static uint32_t cnt = 0;

    PWM_TO_ANGLE(uhDutyCycle, angle);

    error[0] = angle[0] - (float)current_angle[0];
    error[1] = angle[1] - (float)current_angle[1];
    error[2] = angle[2] - (float)current_angle[2];

    P[0] = error[0];
    P[1] = error[1];
    P[2] = error[2];

    I[0] += error[0] * dt;
    I[1] += error[1] * dt;
    I[2] += error[2] * dt;

    D[0] = (error[0] - p_error[0]) / dt;
    D[1] = (error[1] - p_error[1]) / dt;
    D[2] = (error[2] - p_error[2]) / dt;

    I[0] = (I[0] * I_c[0] > 100.0f) ? 100.0f / I_c[0] : I[0];
    I[0] = (I[0] * I_c[0] < -100.0f) ? -100.0f / I_c[0] : I[0];

    I[2] = (I[2] * I_c[2] > 100.0f) ? 100.0f / I_c[2] : I[2];
    I[2] = (I[2] * I_c[2] < -100.0f) ? -100.0f / I_c[2] : I[2];

    D[0] = (D[0] * D_c[0] > 120.0f) ? 120.0f / D_c[0] : D[0];
    D[0] = (D[0] * D_c[0] < -120.0f) ? -120.0f / D_c[0] : D[0];

    D[2] = (D[2] * D_c[2] > 120.0f) ? 120.0f / D_c[2] : D[2];
    D[2] = (D[2] * D_c[2] < -120.0f) ? -120.0f / D_c[2] : D[2];

    PID[0] = P[0] * P_c[0] + I[0] * I_c[0] + D[0] * D_c[0];
    PID[1] = P[1] * P_c[1] + I[1] * I_c[1] + D[1] * D_c[1];
    PID[2] = P[2] * P_c[2] + I[2] * I_c[2] + D[2] * D_c[2];

    angle_to_pwm(PID, uhDutyCycle, motor_power);

    copter cp = {0};

    cp.I[0] = (float)current_angle[0];
    cp.I[1] = (float)current_angle[1];
    cp.I[2] = (float)current_angle[2];
    cp.D[0] = cnt * dt;
    cp.D[1] = P_c[1];

    cnt++;

    memcpy(cp.PID, PID, sizeof(float) * 3);
    memcpy(cp.P, angle, sizeof(float) * 3);
    xQueueSend(copter_queue, (void *)&cp, 0);

    memcpy(p_error, error, sizeof(error));
}

void angle_to_pwm(float *PID, __IO uint16_t *uhDutyCycle, uint16_t *motor_power)
{
    const uint16_t stopped_pwm = 1000;
    const float minimum_pwm = 1150.0f;
    const float maximum_pwm = 1950.0f;

    if ((uhDutyCycle[2] < 1000U) || (uhDutyCycle[2] > 2000U))
    {
        for (uint8_t i = 0; i < 6; i++)
        {
            motor_power[i] = stopped_pwm;
        }
        return;
    }

    float mixed_power[6];
    mixed_power[0] = uhDutyCycle[2] + PID[1] + PID[2] - PID[0];
    mixed_power[1] = uhDutyCycle[2] - PID[1] - PID[0];
    mixed_power[2] = uhDutyCycle[2] + PID[1] - PID[2] - PID[0];
    mixed_power[3] = uhDutyCycle[2] - PID[1] - PID[2] + PID[0];
    mixed_power[4] = uhDutyCycle[2] + PID[1] + PID[0];
    mixed_power[5] = uhDutyCycle[2] - PID[1] + PID[2] + PID[0];

    for (uint8_t i = 0; i < 6; i++)
    {
        float limited_power = mixed_power[i];
        if (limited_power < minimum_pwm)
        {
            limited_power = minimum_pwm;
        }
        else if (limited_power > maximum_pwm)
        {
            limited_power = maximum_pwm;
        }
        motor_power[i] = (uint16_t)limited_power;
    }

    if (uhDutyCycle[4] < 1500)
    {
        motor_power[0] = 1100; // motor_1;
        motor_power[1] = 1100; // motor_2
        motor_power[2] = 1100; // motor_3
        motor_power[3] = 1100; // motor_4
        motor_power[4] = 1100; // motor_5
        motor_power[5] = 1100;
    }
}
