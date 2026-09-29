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

static const float dt = 0.01f;

/*motor */

static const float roll_scale = 0.12f;  /* 180 / 500 */
static const float pitch_scale = 0.06f; /* 30 / 500 */
static const float yaw_scale = 0.06f;   /* 30 / 500 */

static void pwm_to_angle(const uint16_t *rc_pulse, float *angle)
{
    angle[1] = (rc_pulse[3] - 1500) * roll_scale;
    angle[2] = (rc_pulse[0] - 1500) * pitch_scale;
    angle[0] = (rc_pulse[1] - 1500) * yaw_scale;
}

static void angle_to_pwm(const float *pid, const uint16_t *rc_pulse, uint16_t *motor_power);

void pid_update(const uint16_t *rc_pulse, const double *current_angle, uint16_t *motor_power)
{
    static const float P_c[3] = {4.0f, 4.3f, 3.0f};
    static const float I_c[3] = {0.8f, 0.0f, 0.80f};
    static const float D_c[3] = {0.9f, 0.0f, 1.0f};
    static float angle[3] = {0.0f, 0.0f, 0.0f};
    static float p_error[3] = {0.0f, 0.0f, 0.0f};
    float error[3];
    float P[3];
    static float I[3];
    float D[3];
    float PID[3];

    static uint32_t cnt = 0;

    pwm_to_angle(rc_pulse, angle);

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

    angle_to_pwm(PID, rc_pulse, motor_power);

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

static void angle_to_pwm(const float *pid, const uint16_t *rc_pulse, uint16_t *motor_power)
{
    const uint16_t stopped_pwm = 1000;
    const float minimum_pwm = 1150.0f;
    const float maximum_pwm = 1950.0f;

    if ((rc_pulse[2] < 1000U) || (rc_pulse[2] > 2000U))
    {
        for (uint8_t i = 0; i < 6; i++)
        {
            motor_power[i] = stopped_pwm;
        }
        return;
    }

    float mixed_power[6];
    mixed_power[0] = rc_pulse[2] + pid[1] + pid[2] - pid[0];
    mixed_power[1] = rc_pulse[2] - pid[1] - pid[0];
    mixed_power[2] = rc_pulse[2] + pid[1] - pid[2] - pid[0];
    mixed_power[3] = rc_pulse[2] - pid[1] - pid[2] + pid[0];
    mixed_power[4] = rc_pulse[2] + pid[1] + pid[0];
    mixed_power[5] = rc_pulse[2] - pid[1] + pid[2] + pid[0];

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

    if (rc_pulse[4] < 1500)
    {
        motor_power[0] = 1100; // motor_1;
        motor_power[1] = 1100; // motor_2
        motor_power[2] = 1100; // motor_3
        motor_power[3] = 1100; // motor_4
        motor_power[4] = 1100; // motor_5
        motor_power[5] = 1100;
    }
}
