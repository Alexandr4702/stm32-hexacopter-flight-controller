#ifndef MADGWICK_ADAPTER_H_
#define MADGWICK_ADAPTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        MADGWICK_UPDATE_OK = 0,
        MADGWICK_UPDATE_INVALID_INPUT
    } madgwick_update_status;

    void madgwick_adapter_reset(void);

    madgwick_update_status madgwick_adapter_update(const double acceleration[3],
                                                   const double angular_velocity[3],
                                                   double sample_period_s, double euler_angles[3]);

#ifdef __cplusplus
}
#endif

#endif /* MADGWICK_ADAPTER_H_ */
