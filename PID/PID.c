/*
 * PID.c
 *
 *  Created on: Apr 16, 2019
 *      Author: gilg
 */

#include "main.h"
#include "PID.h"

/*PID*/
double P_x = 2.7;
double P_y = 3.0;
double D_x = 0.5;
double D_y = 0.8;

#define P_x0 5.0
#define P_y0 5.0
#define D_x0 0.5
#define D_y0 0.8

#define P_z 7
#define D_z 0.00001

#define I_x 0.8f
#define I_y 0.8f

#include "nrf24l01.h"

extern nrf_handle nrf;

extern QueueHandle_t copter_queue;

//----------------------------------------------------------------------------
extern UART_HandleTypeDef huart1;

const float dt = 0.01;

/*motor */

#define c_0 0.12 /* 180/500 */
#define c_1 0.06 /*30/500   */
#define c_2 0.06 /*30/500   */

double psi, thetta, gamma_;

void PWM_TO_ANGLE(__IO uint16_t *uhDutyCycle, float *angle)
{
    angle[1] = (uhDutyCycle[3] - 1500) * c_0; // omega_psi 		y
    angle[2] = (uhDutyCycle[0] - 1500) * c_1; // thetta 	z
    angle[0] = (uhDutyCycle[1] - 1500) * c_2; // gamma 	x

    //(uhDutyCycle[15]-1500)

    // uint8_t str[200];
    // int strlength=sprintf((char*)str,"%15.5f %15.5f %15.5f \r\n ",angle[1],angle[2],angle[0]);
    // HAL_UART_Transmit(&huart1,str,strlength,0xff);
}

void PID__(__IO uint16_t *uhDutyCycle, double *c_anlge /*�������������� ��������*/,
           uint16_t *motor_power)
{
    float p_variate = (uhDutyCycle[5] - 1500) * 10 / 500.0;
    //------------------------------------------------------
    static float P_c[3] = {4.0, 4.3, 3.0}; // ���������������������� PID
    static float I_c[3] = {0.8, 0 * 1.0, 0.80};
    static float D_c[3] = {0.9, 0.0, 1.0};
    // P_c[0]=10+p_variate;

    //
    static float angle[3] = {0.0, 0.0, 0.0}; // ���������������� ��������
    static float p_error[3] = {0.0, 0.0, 0.0}; // �������������������� ������������
    float error[3];
    float P[3];
    static float I[3];
    float D[3];
    float PID[3];

    static uint32_t cnt = 0;

    PWM_TO_ANGLE(uhDutyCycle, angle); // �������������������� ������������������ ��������

    error[0] =
        (float)angle[0] - (float)c_anlge[0]; // �������������������� ������������ ����������������
                                             // �������� -�������������� // ��������
    error[2] = (float)angle[2] - (float)c_anlge[2]; // ������������
    error[1] =
        (float)angle[1] - (float)c_anlge[1]; // ������ �������������� ���������������� // ��������

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

    /*
    uint8_t str[300];
    int strlength=sprintf((char*)str,""
            "%10.5f %10.5f %10.5f |"
            "%10.5f %10.5f %10.5f |"
            "%10.5f %10.5f %10.5f |"
            "%10.5f %10.5f %10.5f "
            "\r\n",
            PID[0],PID[1],PID[2],
            P[0],P[1],P[2],
            I[0],I[1],I[2],
            angle[0],angle[1],angle[2]);
    HAL_UART_Transmit_DMA(&huart1,str,strlength);
*/

    copter cp;

    cp.I[0] = (float)c_anlge[0];
    cp.I[1] = (float)c_anlge[1];
    cp.I[2] = (float)c_anlge[2];
    cp.D[0] = cnt * dt;
    cp.D[1] = P_c[1];

    cnt++;

    memcpy(cp.PID, PID, sizeof(float) * 3);
    memcpy(cp.P, angle, sizeof(float) * 3);
    // memcpy(cp.I,I,sizeof(float)*3);
    // memcpy(cp.D,D,sizeof(float)*3);
    xQueueSend(copter_queue, (void *)&cp, 0);

    memcpy(p_error, error, sizeof(error));
}

void angle_to_pwm(float *PID, __IO uint16_t *uhDutyCycle, uint16_t *motor_power)
{
    uint16_t m_power[6];

    m_power[0] = uhDutyCycle[2] + PID[1] + PID[2] - PID[0];
    m_power[1] = uhDutyCycle[2] - PID[1] - PID[0];
    m_power[2] = uhDutyCycle[2] + PID[1] - PID[2] - PID[0];
    m_power[3] = uhDutyCycle[2] - PID[1] - PID[2] + PID[0];
    m_power[4] = uhDutyCycle[2] + PID[1] + PID[0];
    m_power[5] = uhDutyCycle[2] - PID[1] + PID[2] + PID[0];

    if (uhDutyCycle[2] < 1000 && uhDutyCycle[2] > 2000)
        uhDutyCycle[2] = 1400;

    //___________________________________________________________________________________________________________________
    if (((m_power[0]) < 1950) && ((m_power[0] > 1150))) // motor_1
    {
        motor_power[0] = m_power[0]; // motor_1;
    }
    else
    {
        if ((m_power[0] < 1150))
            m_power[0] = 1149;
        if ((m_power[0] > 1950))
            m_power[0] = 1949;
    }
    //___________________________________________________________________________________________________________________

    if (((m_power[1]) < 1950) && ((m_power[1]) > 1150)) // motor_2
    {
        motor_power[1] = m_power[1]; // motor_2;
    }
    else
    {
        if ((m_power[1]) < 1150)
            motor_power[1] = 1149;
        if ((m_power[1]) > 1950)
            motor_power[1] = 1949;
    }
    //___________________________________________________________________________________________________________________

    if (((m_power[2]) < 1950) && ((m_power[2]) > 1150)) // motor_3
    {
        motor_power[2] = m_power[2]; // motor_3;
    }
    else
    {
        if ((m_power[2]) < 1150)
            motor_power[2] = 1149;
        if ((m_power[2]) > 1950)
            motor_power[2] = 1949;
    }

    //___________________________________________________________________________________________________________________

    if (((m_power[3]) < 1950) && ((m_power[3]) > 1150)) // motor_4
    {
        motor_power[3] = m_power[3]; // motor_4;
    }
    else
    {
        if ((m_power[3]) < 1150)
            motor_power[3] = 1149;
        if ((m_power[3]) > 1950)
            motor_power[3] = 1949;
    }
    //___________________________________________________________________________________________________________________

    if (((m_power[4]) < 1950) && ((m_power[4]) > 1150)) // motor_5
    {
        motor_power[4] = m_power[4]; // motor_5;
    }
    else
    {
        if ((m_power[4]) < 1150)
            motor_power[4] = 1149;
        if ((m_power[4]) > 1950)
            motor_power[4] = 1949;
    }
    //___________________________________________________________________________________________________________________

    if (((m_power[5]) < 1950) && ((m_power[5]) > 1150)) // motor_6
    {
        motor_power[5] = m_power[5]; // motor_1;
    }
    else
    {
        if ((m_power[5]) < 1150)
            motor_power[5] = 1149;
        if ((m_power[5]) > 1950)
            motor_power[5] = 1949;
    }

    //---------------------------------------------------------------------------------------

    /*
        uint8_t str[300];
        int strlength=sprintf((char*)str,""
                "%4u %4u %4u |"
                "%4u %4u %4u |"
                "%10.5f %10.5f %10.5f |"
                "\r\n ",
                motor_power[0],motor_power[1],motor_power[2],
                motor_power[3],motor_power[4],motor_power[5],
                PID[0],			PID[1],			PID[2]
        );
        HAL_UART_Transmit(&huart1,str,strlength,0xff);
    */

    if (uhDutyCycle[4] < 1500)
    {
        motor_power[0] = 1100; // motor_1;
        motor_power[1] = 1100; // motor_2
        motor_power[2] = 1100; // motor_3
        motor_power[3] = 1100; // motor_4
        motor_power[4] = 1100; // motor_5
        motor_power[5] = 1100;
    }

    /*

        //PID((uint16_t *)uhDutyCycle);
        if(uhDutyCycle[2]<1000 && uhDutyCycle[2]>2000)uhDutyCycle[2]=1400;
        //___________________________________________________________________________________________________________________
        if(((uhDutyCycle[2]-PID_z-PID_y+PID_x)<1950)&&((uhDutyCycle[2]-PID_z-PID_y+PID_x)>1150))//motor_1
        {
            motor_power[0]=uhDutyCycle[2]-PID_z-PID_y+PID_x;//motor_1;
        }
        else
        {
            if((uhDutyCycle[2]-PID_z-PID_y+PID_x)<1150) motor_power[0]=1149;
            if((uhDutyCycle[2]-PID_z-PID_y+PID_x)>1950) motor_power[0]=1949;
        }
        //___________________________________________________________________________________________________________________

        if(((uhDutyCycle[2]+PID_z		 +PID_x)<1950)&&((uhDutyCycle[2]+PID_z
       +PID_x)>1150))//motor_2
        {
            motor_power[1]=uhDutyCycle[2]+PID_z		 +PID_x;//motor_2;
        }
        else
        {
            if((uhDutyCycle[2]+PID_z		 +PID_x)<1150) motor_power[1]=1149;
            if((uhDutyCycle[2]+PID_z		 +PID_x)>1950) motor_power[1]=1949;
        }
        //___________________________________________________________________________________________________________________


        if(((uhDutyCycle[2]-PID_z+PID_y+PID_x)<1950)&&((uhDutyCycle[2]-PID_z+PID_y+PID_x)>1150))//motor_3
        {
            motor_power[2]=uhDutyCycle[2]-PID_z+PID_y+PID_x;//motor_3;
        }
        else
        {
            if((uhDutyCycle[2]-PID_z+PID_y+PID_x)<1150) motor_power[2]=1149;
            if((uhDutyCycle[2]-PID_z+PID_y+PID_x)>1950) motor_power[2]=1949;
        }

        //___________________________________________________________________________________________________________________

        if(((uhDutyCycle[2]+PID_z+PID_y-PID_x)<1950)&&((uhDutyCycle[2]+PID_z+PID_y-PID_x)>1150))//motor_4
        {
            motor_power[3]=uhDutyCycle[2]+PID_z+PID_y-PID_x;//motor_4;
        }
        else
        {
            if((uhDutyCycle[2]+PID_z+PID_y-PID_x)<1150) motor_power[3]=1149;
            if((uhDutyCycle[2]+PID_z+PID_y-PID_x)>1950) motor_power[3]=1949;
        }
        //___________________________________________________________________________________________________________________

        if(((uhDutyCycle[2]-PID_z		 -PID_x)<1950)&&((uhDutyCycle[2]-PID_z
       -PID_x)>1150))//motor_5
        {
            motor_power[4]=uhDutyCycle[2]-PID_z		 -PID_x;//motor_5;
        }
        else
        {
            if((uhDutyCycle[2]-PID_z		 -PID_x)<1150) motor_power[4]=1149;
            if((uhDutyCycle[2]-PID_z		 -PID_x)>1950) motor_power[4]=1949;
        }
        //___________________________________________________________________________________________________________________

        if(((uhDutyCycle[2]+PID_z-PID_y-PID_x)<1950)&&((uhDutyCycle[2]+PID_z-PID_y-PID_x)>1150))//motor_6
        {
            motor_power[5]=uhDutyCycle[2]+PID_z-PID_y-PID_x;//motor_1;
        }
        else
        {
            if((uhDutyCycle[2]+PID_z-PID_y-PID_x)<1150) motor_power[5]=1149;
            if((uhDutyCycle[2]+PID_z-PID_y-PID_x)>1950) motor_power[5]=1949;
        }


    */
    // motor_power[1]=uhDutyCycle[2]+PID_z		 +PID_x;//motor_2
    // motor_power[2]=uhDutyCycle[2]-PID_z+PID_y+PID_x;//motor_3
    // motor_power[3]=uhDutyCycle[2]+PID_z+PID_y-PID_x;//motor_4
    // motor_power[4]=uhDutyCycle[2]-PID_z		 -PID_x;//motor_5
    // motor_power[5]=uhDutyCycle[2]+PID_z-PID_y-PID_x;//motor_6
}
