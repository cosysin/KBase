/*
 @ Kingsley Chen
*/

// Revised from the openssl version.

/*
* This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
* MD5 Message-Digest Algorithm (RFC 1321).
*
* Homepage:
* http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
*
* Author:
* Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
*
* This software was written by Alexander Peslyak in 2001.  No copyright is
* claimed, and the software is hereby placed in the public domain.
* In case this attempt to disclaim copyright and place the software in the
* public domain is deemed null and void, then the software is
* Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
* general public under the following terms:
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted.
*
* There's ABSOLUTELY NO WARRANTY, express or implied.
*
* (This is a heavily cut-down "BSD license".)
*
* This differs from Colin Plumb's older public domain implementation in that
* no exactly 32-bit integer data type is required (any 32-bit or wider
* unsigned integer data type will do), there's no compile-time endianness
* configuration, and the function prototypes match OpenSSL's.  No code from
* Colin Plumb's implementation has been reused; this comment merely compares
* the properties of the two independent implementations.
*
* The primary goals of this implementation are portability and ease of use.
* It is meant to be fast, but not as fast as possible.  Some known
* optimizations are not included to reduce source code size and avoid
* compile-time configuration.
*/

#include "kbase\md5.h"

#include <cstring>

namespace {

using kbase::MD5Context;
using kbase::MD5uint32;

// The basic MD5 functions.
// F and G are optimized compared to their RFC 1321 definitions for
// architectures that lack an AND-NOT instruction, just like in Colin Plumb's
// implementation.
#define F(x, y, z)  ((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z)  ((y) ^ ((z) & ((x) ^ (y))))
#define H(x, y, z)  ((x) ^ (y) ^ (z))
#define I(x, y, z)  ((y) ^ ((x) | ~(z)))

// The MD5 transformation for all four rounds.
#define STEP(f, a, b, c, d, x, t, s) \
    (a) += f((b), (c), (d)) + (x) + (t); \
    (a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s)))); \
    (a) += (b);

// SET reads 4 input bytes in little-endian byte order and stores them
// in a properly aligned word in host byte order.
// The check for little-endian architectures that tolerate unaligned
// memory accesses is just an optimization. Nothing will break if it
// doesn't work.
#if defined(_MSC_VER) ||defined(__i386__) || defined(__x86_64__)
#define SET(n) \
    (*(MD5uint32 *)&ptr[(n) * 4])
#define GET(n) \
    SET(n)
#else
#define SET(n) \
    (context->block[(n)] = \
    (MD5uint32)ptr[(n) * 4] | \
    ((MD5uint32)ptr[(n) * 4 + 1] << 8) | \
    ((MD5uint32)ptr[(n) * 4 + 2] << 16) | \
    ((MD5uint32)ptr[(n) * 4 + 3] << 24))
#define GET(n) \
    (context->block[(n)])
#endif

// This processes one or more 64-byte data blocks, but does NOT update
// the bit counters. There are no alignment requirements.
const void* Transform(MD5Context* context, const void* data, size_t size)
{
    MD5uint32 a, b, c, d;
    MD5uint32 saved_a, saved_b, saved_c, saved_d;
    const unsigned char* ptr = static_cast<const unsigned char*>(data);

    a = context->a;
    b = context->b;
    c = context->c;
    d = context->d;

    do {
        saved_a = a;
        saved_b = b;
        saved_c = c;
        saved_d = d;

        /* Round 1 */
        STEP(F, a, b, c, d, SET(0), 0xd76aa478, 7)
        STEP(F, d, a, b, c, SET(1), 0xe8c7b756, 12)
        STEP(F, c, d, a, b, SET(2), 0x242070db, 17)
        STEP(F, b, c, d, a, SET(3), 0xc1bdceee, 22)
        STEP(F, a, b, c, d, SET(4), 0xf57c0faf, 7)
        STEP(F, d, a, b, c, SET(5), 0x4787c62a, 12)
        STEP(F, c, d, a, b, SET(6), 0xa8304613, 17)
        STEP(F, b, c, d, a, SET(7), 0xfd469501, 22)
        STEP(F, a, b, c, d, SET(8), 0x698098d8, 7)
        STEP(F, d, a, b, c, SET(9), 0x8b44f7af, 12)
        STEP(F, c, d, a, b, SET(10), 0xffff5bb1, 17)
        STEP(F, b, c, d, a, SET(11), 0x895cd7be, 22)
        STEP(F, a, b, c, d, SET(12), 0x6b901122, 7)
        STEP(F, d, a, b, c, SET(13), 0xfd987193, 12)
        STEP(F, c, d, a, b, SET(14), 0xa679438e, 17)
        STEP(F, b, c, d, a, SET(15), 0x49b40821, 22)

        /* Round 2 */
        STEP(G, a, b, c, d, GET(1), 0xf61e2562, 5)
        STEP(G, d, a, b, c, GET(6), 0xc040b340, 9)
        STEP(G, c, d, a, b, GET(11), 0x265e5a51, 14)
        STEP(G, b, c, d, a, GET(0), 0xe9b6c7aa, 20)
        STEP(G, a, b, c, d, GET(5), 0xd62f105d, 5)
        STEP(G, d, a, b, c, GET(10), 0x02441453, 9)
        STEP(G, c, d, a, b, GET(15), 0xd8a1e681, 14)
        STEP(G, b, c, d, a, GET(4), 0xe7d3fbc8, 20)
        STEP(G, a, b, c, d, GET(9), 0x21e1cde6, 5)
        STEP(G, d, a, b, c, GET(14), 0xc33707d6, 9)
        STEP(G, c, d, a, b, GET(3), 0xf4d50d87, 14)
        STEP(G, b, c, d, a, GET(8), 0x455a14ed, 20)
        STEP(G, a, b, c, d, GET(13), 0xa9e3e905, 5)
        STEP(G, d, a, b, c, GET(2), 0xfcefa3f8, 9)
        STEP(G, c, d, a, b, GET(7), 0x676f02d9, 14)
        STEP(G, b, c, d, a, GET(12), 0x8d2a4c8a, 20)

        /* Round 3 */
        STEP(H, a, b, c, d, GET(5), 0xfffa3942, 4)
        STEP(H, d, a, b, c, GET(8), 0x8771f681, 11)
        STEP(H, c, d, a, b, GET(11), 0x6d9d6122, 16)
        STEP(H, b, c, d, a, GET(14), 0xfde5380c, 23)
        STEP(H, a, b, c, d, GET(1), 0xa4beea44, 4)
        STEP(H, d, a, b, c, GET(4), 0x4bdecfa9, 11)
        STEP(H, c, d, a, b, GET(7), 0xf6bb4b60, 16)
        STEP(H, b, c, d, a, GET(10), 0xbebfbc70, 23)
        STEP(H, a, b, c, d, GET(13), 0x289b7ec6, 4)
        STEP(H, d, a, b, c, GET(0), 0xeaa127fa, 11)
        STEP(H, c, d, a, b, GET(3), 0xd4ef3085, 16)
        STEP(H, b, c, d, a, GET(6), 0x04881d05, 23)
        STEP(H, a, b, c, d, GET(9), 0xd9d4d039, 4)
        STEP(H, d, a, b, c, GET(12), 0xe6db99e5, 11)
        STEP(H, c, d, a, b, GET(15), 0x1fa27cf8, 16)
        STEP(H, b, c, d, a, GET(2), 0xc4ac5665, 23)

        /* Round 4 */
        STEP(I, a, b, c, d, GET(0), 0xf4292244, 6)
        STEP(I, d, a, b, c, GET(7), 0x432aff97, 10)
        STEP(I, c, d, a, b, GET(14), 0xab9423a7, 15)
        STEP(I, b, c, d, a, GET(5), 0xfc93a039, 21)
        STEP(I, a, b, c, d, GET(12), 0x655b59c3, 6)
        STEP(I, d, a, b, c, GET(3), 0x8f0ccc92, 10)
        STEP(I, c, d, a, b, GET(10), 0xffeff47d, 15)
        STEP(I, b, c, d, a, GET(1), 0x85845dd1, 21)
        STEP(I, a, b, c, d, GET(8), 0x6fa87e4f, 6)
        STEP(I, d, a, b, c, GET(15), 0xfe2ce6e0, 10)
        STEP(I, c, d, a, b, GET(6), 0xa3014314, 15)
        STEP(I, b, c, d, a, GET(13), 0x4e0811a1, 21)
        STEP(I, a, b, c, d, GET(4), 0xf7537e82, 6)
        STEP(I, d, a, b, c, GET(11), 0xbd3af235, 10)
        STEP(I, c, d, a, b, GET(2), 0x2ad7d2bb, 15)
        STEP(I, b, c, d, a, GET(9), 0xeb86d391, 21)

        a += saved_a;
        b += saved_b;
        c += saved_c;
        d += saved_d;

        ptr += 64;
    } while (size -= 64);

    context->a = a;
    context->b = b;
    context->c = c;
    context->d = d;

    return ptr;
}

}   // namespace

namespace kbase {

void MD5Init(MD5Context* context)
{
    context->a = 0x67452301;
    context->b = 0xefcdab89;
    context->c = 0x98badcfe;
    context->d = 0x10325476;

    context->lo = 0;
    context->hi = 0;
}

void MD5Update(MD5Context* context, const void* data, size_t size)
{
    MD5uint32 saved_lo = context->lo;
    unsigned long used, free;

    if ((context->lo = (saved_lo + size) & 0x1fffffff) < saved_lo) {
        context->hi++;
    }

    context->hi += size >> 29;
    used = saved_lo & 0x3f;

    if (used) {
        free = 64 - used;
        if (size < free) {
            memcpy(&context->buffer[used], data, size);
            return;
        }

        memcpy(&context->buffer[used], data, free);
        data = (unsigned char *)data + free;
        size -= free;
        Transform(context, context->buffer, 64);
    }

    if (size >= 64) {
        data = Transform(context, data, size & ~(unsigned long)0x3f);
        size &= 0x3f;
    }

    memcpy(context->buffer, data, size);
}

void MD5Final(unsigned char* result, MD5Context* context)
{
    unsigned long used, free;
    used = context->lo & 0x3f;
    context->buffer[used++] = 0x80;
    free = 64 - used;

    if (free < 8) {
        memset(&context->buffer[used], 0, free);
        Transform(context, context->buffer, 64);
        used = 0;
        free = 64;
    }

    memset(&context->buffer[used], 0, free - 8);

    context->lo <<= 3;
    context->buffer[56] = static_cast<unsigned char>(context->lo);
    context->buffer[57] = static_cast<unsigned char>(context->lo >> 8);
    context->buffer[58] = static_cast<unsigned char>(context->lo >> 16);
    context->buffer[59] = static_cast<unsigned char>(context->lo >> 24);
    context->buffer[60] = static_cast<unsigned char>(context->hi);
    context->buffer[61] = static_cast<unsigned char>(context->hi >> 8);
    context->buffer[62] = static_cast<unsigned char>(context->hi >> 16);
    context->buffer[63] = static_cast<unsigned char>(context->hi >> 24);

    Transform(context, context->buffer, 64);

    result[0] = static_cast<unsigned char>(context->a);
    result[1] = static_cast<unsigned char>(context->a >> 8);
    result[2] = static_cast<unsigned char>(context->a >> 16);
    result[3] = static_cast<unsigned char>(context->a >> 24);
    result[4] = static_cast<unsigned char>(context->b);
    result[5] = static_cast<unsigned char>(context->b >> 8);
    result[6] = static_cast<unsigned char>(context->b >> 16);
    result[7] = static_cast<unsigned char>(context->b >> 24);
    result[8] = static_cast<unsigned char>(context->c);
    result[9] = static_cast<unsigned char>(context->c >> 8);
    result[10] = static_cast<unsigned char>(context->c >> 16);
    result[11] = static_cast<unsigned char>(context->c >> 24);
    result[12] = static_cast<unsigned char>(context->d);
    result[13] = static_cast<unsigned char>(context->d >> 8);
    result[14] = static_cast<unsigned char>(context->d >> 16);
    result[15] = static_cast<unsigned char>(context->d >> 24);

    memset(context, 0, sizeof(*context));
}

}   // namespace kbase