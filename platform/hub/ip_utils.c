/*
 *
 * Copyright 2014 Jyotiswarup Raitukar
 * All rights reserved
 */


#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "ip_utils.h"



static const char *hexchars = "0123456789abcdef";

static const char *
inet_ntop4(const uint32_t *addr, char *buf, size_t len)
{
        const uint8_t *ap = (const uint8_t *)addr;
        char tmp[MAX_IPv4_STR_LEN]; /* max length of ipv4 addr string */
        int fulllen;

        /*
         * snprintf returns number of bytes printed (not including NULL) or
         * number of bytes that would have been printed if more than would
         * fit
         */
        fulllen = snprintf(tmp, sizeof(tmp), "%d.%d.%d.%d",
                           ap[0], ap[1], ap[2], ap[3]);
        if (fulllen >= (int)len) {
                return NULL;
        }

        bcopy(tmp, buf, fulllen + 1);

        return buf;
}

static const char *
inet_ntop6(const struct in6_addr *addr, char *dst, size_t size)
{
        char hexa[8][5], tmp[MAX_IPv6_STR_LEN];
        int zr[8];
        size_t len;
        int32_t i, j, k, skip;
        uint8_t x8, hx8;
        uint16_t x16;
        struct in_addr a4;

        if (addr == NULL) return NULL;

        bzero(tmp, sizeof(tmp));

        /*  check for mapped or compat addresses */
        i = IN6_IS_ADDR_V4MAPPED(addr);
        j = IN6_IS_ADDR_V4COMPAT(addr);
        if ((i != 0) || (j != 0)) {
                char tmp2[16]; /* max length of ipv4 addr string */
                a4.s_addr = addr->__in6_u.__u6_addr32[3];
                len = snprintf(tmp, sizeof(tmp), "::%s%s", (i != 0) ? "ffff:" : "",
                               inet_ntop4((const uint32_t *)&a4, tmp2,
                                          sizeof(tmp2)));
                if (len >= size) return NULL;
                bcopy(tmp, dst, len + 1);
                return dst;
        }

        k = 0;
        for (i = 0; i < 16; i += 2) {
                j = 0;
                skip = 1;

                bzero(hexa[k], 5);

                x8 = addr->__in6_u.__u6_addr8[i];

                hx8 = x8 >> 4;
                if (hx8 != 0) {
                        skip = 0;
                        hexa[k][j++] = hexchars[hx8];
                }

                hx8 = x8 & 0x0f;
                if ((skip == 0) || ((skip == 1) && (hx8 != 0))) {
                        skip = 0;
                        hexa[k][j++] = hexchars[hx8];
                }

                x8 = addr->__in6_u.__u6_addr8[i + 1];

                hx8 = x8 >> 4;
                if ((skip == 0) || ((skip == 1) && (hx8 != 0))) {
                        hexa[k][j++] = hexchars[hx8];
                }

                hx8 = x8 & 0x0f;
                hexa[k][j++] = hexchars[hx8];

                k++;
        }

        /* find runs of zeros for :: convention */
        j = 0;
        for (i = 7; i >= 0; i--) {
                zr[i] = j;
                x16 = addr->__in6_u.__u6_addr16[i];
                if (x16 == 0) j++;
                else j = 0;
                zr[i] = j;
        }

        /* find longest run of zeros */
        k = -1;
        j = 0;
        for(i = 0; i < 8; i++) {
                if (zr[i] > j) {
                        k = i;
                        j = zr[i];
                }
        }

        for(i = 0; i < 8; i++) {
                if (i != k) zr[i] = 0;
        }

        len = 0;
        for (i = 0; i < 8; i++) {
                if (zr[i] != 0) {
                        /* check for leading zero */
                        if (i == 0) tmp[len++] = ':';
                        tmp[len++] = ':';
                        i += (zr[i] - 1);
                        continue;
                }
                for (j = 0; hexa[i][j] != '\0'; j++) tmp[len++] = hexa[i][j];
                if (i != 7) tmp[len++] = ':';
        }

        /* trailing NULL */
        len++;

        if (len > size) return NULL;
        bcopy(tmp, dst, len);
        return dst;
}

const char *
vb_inet_ntop(int af, const void *addr, char *buf, size_t len)
{
        if(af==AF_INET6)
                return inet_ntop6(addr, buf, len);
        if(af==AF_INET)
                return inet_ntop4(addr, buf, len);
        return NULL;
}

