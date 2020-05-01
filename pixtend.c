
#include "pixtend.h"


uint16_t crc16_calc(uint16_t crc, uint8_t data)
{
    int i;
    crc ^= data;

    for (i = 0; i < 8; ++i)
    {
        if (crc & 1)
        {
            crc = (crc >> 1) ^ 0xA001;
        }
        else
        {
            crc = (crc >> 1);
        }
    }
    return crc;
}


uint16_t crc16(uint16_t crc, uint8_t * data, size_t len)
{
    size_t i;

    for(i=0; i<len; ++i)
    {
        crc = crc16_calc(crc, data[i]);
    }

    return crc;
}





// eof

