//=============================================================================
// FILE  : ble_device.cpp
// DESC  :
// AUTHOR: PINBO
// CREATE: 2015-06-02
// MODIFY:
//=============================================================================

#include <stdlib.h>
#include <string.h>
#include "ble_defines.h"


//=============================================================================
//=============================================================================

char* string_check(char *p_str)
{
    int n = strlen(p_str);
    if (n>=1)
    {
        if ((0xC0 == ((uint8_t)p_str[n-1] & 0xC0)) || 
            (0xE0 == ((uint8_t)p_str[n-1] & 0xE0)))
            p_str[n-1] = '\0';
    }
    if (n>=2)
    {
        if ((0xE0 == ((uint8_t)p_str[n-2] & 0xE0)))
            p_str[n-2] = '\0';
    }
    return p_str;
}

uint8_t uint16_encode(uint16_t value, uint8_t *p_encoded_data)
{
    p_encoded_data[0] = (uint8_t) ((value & 0x00FF) >> 0);
    p_encoded_data[1] = (uint8_t) ((value & 0xFF00) >> 8);
    return sizeof(uint16_t);
}

uint8_t uint32_encode(uint32_t value, uint8_t *p_encoded_data)
{
    p_encoded_data[0] = (uint8_t) ((value & 0x000000FF) >> 0);
    p_encoded_data[1] = (uint8_t) ((value & 0x0000FF00) >> 8);
    p_encoded_data[2] = (uint8_t) ((value & 0x00FF0000) >> 16);
    p_encoded_data[3] = (uint8_t) ((value & 0xFF000000) >> 24);
    return sizeof(uint32_t);
}

uint16_t uint16_decode(const uint8_t *p_encoded_data)
{
        return ( (((uint16_t)((uint8_t *)p_encoded_data)[0]) << 0 ) |
                 (((uint16_t)((uint8_t *)p_encoded_data)[1]) << 8 ));
}

uint32_t uint32_decode(const uint8_t *p_encoded_data)
{
    return ( (((uint32_t)((uint8_t *)p_encoded_data)[0]) << 0)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[1]) << 8)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[2]) << 16) |
             (((uint32_t)((uint8_t *)p_encoded_data)[3]) << 24 ));
}

uint8_t big_uint16_encode(uint16_t value, uint8_t *p_encoded_data)
{
    p_encoded_data[0] = (uint8_t) ((value & 0xFF00) >> 8);
    p_encoded_data[1] = (uint8_t) ((value & 0x00FF) >> 0);
    return sizeof(uint16_t);
}

uint8_t big_uint32_encode(uint32_t value, uint8_t *p_encoded_data)
{
    p_encoded_data[0] = (uint8_t) ((value & 0xFF000000) >> 24);
    p_encoded_data[1] = (uint8_t) ((value & 0x00FF0000) >> 16);
    p_encoded_data[2] = (uint8_t) ((value & 0x0000FF00) >> 8);
    p_encoded_data[3] = (uint8_t) ((value & 0x000000FF) >> 0);
    return sizeof(uint32_t);
}

uint16_t big_uint16_decode(const uint8_t *p_encoded_data)
{
    return (((uint16_t)p_encoded_data[0] << 8) |
            ((uint16_t)p_encoded_data[1] << 0));
}

uint32_t big_uint32_decode(const uint8_t *p_encoded_data)
{
    return (((uint32_t)p_encoded_data[0] << 24) |
            ((uint32_t)p_encoded_data[1] << 16) |
            ((uint32_t)p_encoded_data[2] <<  8) |
            ((uint32_t)p_encoded_data[3] <<  0));
}

//=============================================================================
//=============================================================================
uint8_t MsgChecksum(const uint8_t *p_msg, uint8_t msg_len)
{
    uint8_t i = 1;
    uint8_t value;

    value = p_msg[0];
    for (i = 1; i < msg_len; i++)
        value ^= p_msg[i];

    return value;
}

uint8_t MsgHex2ASCII(char *p_out, const uint8_t *p_in, uint8_t msg_len)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t value;

    while (i < msg_len)
    {
        value = ((p_in[i] & 0xF0) >> 4);
        if ((value >= 0) && (value <= 9))
            value += 0x30;          // 0 ~ 9
        else if ((value >= 10) && (value <= 15))
            value += 0x37;          // A ~ F
        else
            value = 0;

        p_out[j] = value;
        j++;

        value = ((p_in[i] & 0x0F) >> 0);
        if ((value >= 0) && (value <= 9))
            value += 0x30;          // 0 ~ 9
        else if ((value >= 10) && (value <= 15))
            value += 0x37;          // A ~ F
        else
            value = 0;

        p_out[j] = value;
        i++;
        j++;
    }

    p_out[j] = '\0';
    return j;
}

uint8_t MsgASCII2Hex(uint8_t *p_out, const char *p_in, uint8_t msg_len)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t value;

    while (i < msg_len)
    {
        value = p_in[i];
        if ((value >= 0x30) && (value <= 0x39))
            value -= 0x30;          // '0' ~ '9'
        else if ((value >= 0x40) && (value <= 0x46))
            value -= 0x37;          // 'A' ~ 'Z'
        else if ((value >= 0x61) && (value <= 0x66))
            value -= 0x57;          // 'a' ~ 'z'
        else
            value = 0;

        p_out[j] = (value << 4);
        if (++i >= msg_len)
            return j;

        value = p_in[i];
        if ((value >= 0x30) && (value <= 0x39))
            value -= 0x30;          // '0' ~ '9'
        else if ((value >= 0x40) && (value <= 0x46))
            value -= 0x37;          // 'A' ~ 'Z'
        else if ((value >= 0x61) && (value <= 0x66))
            value -= 0x57;          // 'a' ~ 'z'
        else
            value = 0;

        p_out[j] |= (value << 0);
        i++;
        j++;
    }

    return j;
}

//=============================================================================
//=============================================================================

/*
 * URL Scheme Prefix:
 *  +---------+------------+----------
 *  |Decimal  | Hex        | Expansion
 *  +---------+------------+----------
 *  |0        | 0x00       | `http://www.`
 *  |1        | 0x01       | `https://www.`
 *  |2        | 0x02       | `http://`
 *  |3        | 0x03       | `https://`
 *  +---------+------------+----------
 */

static const char *eurl_prefix[4] = {
    "http://www.",
    "https://www.",
    "http://",
    "https://",
};

/*
 * Eddystone-URL HTTP URL encoding:
 *  +---------+------------+----------
 *  |Decimal  | Hex        | Expansion
 *  +---------+------------+----------
 *  |0        | 0x00       | .com/
 *  |1        | 0x01       | .org/
 *  |2        | 0x02       | .edu/
 *  |3        | 0x03       | .net/
 *  |4        | 0x04       | .info/
 *  |5        | 0x05       | .biz/
 *  |6        | 0x06       | .gov/
 *  |7        | 0x07       | .com
 *  |8        | 0x08       | .org
 *  |9        | 0x09       | .edu
 *  |10       | 0x0a       | .net
 *  |11       | 0x0b       | .info
 *  |12       | 0x0c       | .biz
 *  |13       | 0x0d       | .gov
 *  |14..32   | 0x0e..0x20 | Reserved for Future Use
 *  |127..255 | 0x7F..0xFF | Reserved for Future Use
 *  +---------+------------+----------
 */

static const char *eurl_encoding[14] = {
    ".com/",
    ".org/",
    ".edu/",
    ".net/",
    ".info/",
    ".biz/",
    ".gov/",
    ".com",
    ".org",
    ".edu",
    ".net",
    ".info",
    ".biz",
    ".gov",
};

uint8_t Hex2EddystoneURL(char *p_out, const uint8_t *p_in, uint8_t hex_len)
{
    if (hex_len == 0 || hex_len > BLE_MAX_EURL_LEN)
    {
        p_out[0] = '\0';
        return 0;
    }

    // converting the URL prefix
    uint8_t str_len = 0;
    if (p_in[0] <= 3)
    {
        strcpy(&p_out[str_len], eurl_prefix[p_in[0]]);
        str_len += strlen(eurl_prefix[p_in[0]]);
    }

    // encoding the URL
    for (uint8_t i = 1; i < hex_len; i++)
    {
        if (p_in[i] <= 13)
        {
            strcpy(&p_out[str_len], eurl_encoding[p_in[i]]);
            str_len += strlen(eurl_encoding[p_in[i]]);
        }
        else
        {
            p_out[str_len] = p_in[i];
            str_len++;
        }
    }

    // return the actual URL length
    p_out[str_len] = '\0';
    return str_len;
}

uint8_t EddystoneURL2Hex(uint8_t *p_out, const char *p_in, uint8_t str_len)
{
    const char *p = p_in;
    const char *s = NULL;

    uint8_t i;
    uint8_t hex_len = 0;

    // parsing the URL prefix
    for (i = 0; i <= 3; i++)
    {
        s = strstr(p, eurl_prefix[i]);
        if (s != NULL && s == p)
        {
            p_out[hex_len++] = i;
            p += strlen(eurl_prefix[i]);
            break;
        }
    }

    // decoding the URL
    while (p < (p_in + str_len))
    {
        for (i = 0; i <= 13; i++)
        {
            s = strstr(p, eurl_encoding[i]);
            if (s != NULL && s == p)
            {
                p_out[hex_len++] = i;
                p += strlen(eurl_encoding[i]);
                break;
            }
        }

        if (i > 13)
        {
            p_out[hex_len++] = *p;
            p++;
        }
    }

    // return the HEX length
    return hex_len;
}
