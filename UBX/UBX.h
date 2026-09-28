/*
 * UBX.h
 *
 *  Created on: Aug 17, 2018
 *      Author: gilg
 */

#ifndef UBX_H_
#define UBX_H_

#define UART_UBX huart2
#include <stdint.h>

/*
 * brif _mess_ready
 * 1 _NAV_POSLLH_
 * 2 _NAV_STATUS_
 * 4 _NAV_DOP_
 * 8 _NAV_VELNED_
 * 16 error
 */

enum
{
    _NAV_POSLLH_ready = 1,
    _NAV_STATUS_ready = 2,
    _NAV_DOP_ready = 4,
    _NAV_VELNED_ready = 8,
    error_ready = 16
};

enum
{
    class_ = 0x1,
    ID = 0x2,
    length = 0x3,
    payload = 0x4,
    checksum_ = 0x5
};
enum
{
    NAV_POSLLH = 0x02,
    NAV_STATUS = 0x03,
    NAV_DOP = 0x04,
    NAV_VELNED = 0x12
};

typedef struct
{
    double longitude;
    double latitude;
    double Height_above_sea;
    double horizontal_accuracy;
    double vertical_accuracy;
} NAV_POSLLH_;

typedef struct
{
    uint8_t gpsFix;
    uint32_t time_since_restart_ms;
} NAV_STATUS_;

typedef struct
{
    double Vdop;
    double Hdop;

} NAV_DOP_;

typedef struct
{
    double velN;
    double velE;
    double velD;
    double speed;
    double gspeed;
    double heading;
    double sAcc;
    double cAcc;
} NAV_VELNED_;

/*
 * brif _mess_ready
 * 1 _NAV_POSLLH_
 * 2 _NAV_STATUS_
 * 4 _NAV_DOP_
 * 8 _NAV_VELNED_
 * 16 error
 */

typedef struct
{
    NAV_POSLLH_ _NAV_POSLLH_;
    NAV_STATUS_ _NAV_STATUS_;
    NAV_DOP_ _NAV_DOP_;
    NAV_VELNED_ _NAV_VELNED_;
    uint16_t _mess_ready;
} navigation_mes;

void UBX_init(void);
void pars(uint8_t *ptr, uint16_t cnt_bytes, navigation_mes *mes);

#endif /* UBX_H_ */
