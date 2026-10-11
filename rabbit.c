#include <stdlib.h>
#include "rabbit.h"
// Square a 32-bit number to obtain the 64-bit result and return
// the upper 32 bit XOR the lower 32 bit
uint32 g_func(uint32 x) {
    // Construct high and low argument for squaring
    uint32 a = x&0xFFFF;
    uint32 b = x>>16;
    // Calculate high and low result of squaring
    uint32 h = ((((a*a)>>17) + (a*b))>>15) + b*b;
    uint32 l = x*x;
    // Return high XOR low;
    return hˆl;
}
// Calculate the next internal state
void next_state(t_instance *p_instance)
{
    // Temporary data
    uint32 g[8], c_old[8], i;
    // Save old counter values
    for (i=0; i<8; i++)
        c_old[i] = p_instance->c[i];
    // Calculate new counter values
    p_instance->c[0] += 0x4D34D34D + p_instance->carry;
    p_instance->c[1] += 0xD34D34D3 + (p_instance->c[0] < c_old[0]);
    p_instance->c[2] += 0x34D34D34 + (p_instance->c[1] < c_old[1]);
    p_instance->c[3] += 0x4D34D34D + (p_instance->c[2] < c_old[2]);
    p_instance->c[4] += 0xD34D34D3 + (p_instance->c[3] < c_old[3]);
    p_instance->c[5] += 0x34D34D34 + (p_instance->c[4] < c_old[4]);
    p_instance->c[6] += 0x4D34D34D + (p_instance->c[5] < c_old[5]);
    p_instance->c[7] += 0xD34D34D3 + (p_instance->c[6] < c_old[6]);
    p_instance->carry = (p_instance->c[7] < c_old[7]);
    // Calculate the g-functions
    for (i=0;i<8;i++)
        g[i] = g_func(p_instance->x[i] + p_instance->c[i]);
    // Calculate new state values
    p_instance->x[0] = g[0] + _rotl(g[7],16) + _rotl(g[6],16);
    p_instance->x[1] = g[1] + _rotl(g[0], 8) + g[7];
    p_instance->x[2] = g[2] + _rotl(g[1],16) + _rotl(g[0],16);
    p_instance->x[3] = g[3] + _rotl(g[2], 8) + g[1];
    p_instance->x[4] = g[4] + _rotl(g[3],16) + _rotl(g[2],16);
    p_instance->x[5] = g[5] + _rotl(g[4], 8) + g[3];
    p_instance->x[6] = g[6] + _rotl(g[5],16) + _rotl(g[4],16);
    p_instance->x[7] = g[7] + _rotl(g[6], 8) + g[5];
}
// key_setup
void key_setup(t_instance *p_instance, const byte *p_key)
{
    // Temporary data
    uint32 k0, k1, k2, k3, i;
    // Generate four subkeys
    k0 = *(uint32*)(p_key+ 0);
    k1 = *(uint32*)(p_key+ 4);
    k2 = *(uint32*)(p_key+ 8);
    k3 = *(uint32*)(p_key+12);
    // Generate initial state variables
    p_instance->x[0] = k0;
    p_instance->x[2] = k1;
    p_instance->x[4] = k2;
    p_instance->x[6] = k3;
    p_instance->x[1] = (k3<<16) | (k2>>16);
    p_instance->x[3] = (k0<<16) | (k3>>16);
    p_instance->x[5] = (k1<<16) | (k0>>16);
    p_instance->x[7] = (k2<<16) | (k1>>16);
    // Generate initial counter values
    p_instance->c[0] = _rotl(k2,16);
    p_instance->c[2] = _rotl(k3,16);
    p_instance->c[4] = _rotl(k0,16);
    p_instance->c[6] = _rotl(k1,16);
    p_instance->c[1] = (k0&0xFFFF0000) | (k1&0xFFFF);
    p_instance->c[3] = (k1&0xFFFF0000) | (k2&0xFFFF);
    p_instance->c[5] = (k2&0xFFFF0000) | (k3&0xFFFF);
    p_instance->c[7] = (k3&0xFFFF0000) | (k0&0xFFFF);
    // Reset carry flag
    p_instance->carry = 0;
    // Iterate the system four times
    for (i=0;i<4;i++)
        next_state(p_instance);
    // Modify the counters
    for (i=0;i<8;i++)
        p_instance->c[(i+4)&0x7] ˆ= p_instance->x[i];
}
// Encrypt or decrypt a block of data
void cipher(t_instance *p_instance, const byte *p_src,
byte *p_dest, size_t data_size)
{
    uint32 i;
    for (i=0; i<data_size; i+=16)
    {
    // Iterate the system
    next_state(p_instance);
    // Encrypt 16 bytes of data
    *(uint32*)(p_dest+ 0) = *(uint32*)(p_src+ 0) ˆ 
                            p_instance->x[0] ˆ
                            (p_instance->x[5]>>16) ˆ
                            (p_instance->x[3]<<16);
    *(uint32*)(p_dest+ 4) = *(uint32*)(p_src+ 4) ˆ
                            p_instance->x[2] ˆ
                            (p_instance->x[7]>>16) ˆ
                            (p_instance->x[5]<<16);
    *(uint32*)(p_dest+ 8) = *(uint32*)(p_src+ 8) ˆ
                            p_instance->x[4] ˆ
                            (p_instance->x[1]>>16) ˆ
                            (p_instance->x[7]<<16);
    *(uint32*)(p_dest+12) = *(uint32*)(p_src+12) ˆ
                            p_instance->x[6] ˆ
                            (p_instance->x[3]>>16) ˆ
                            (p_instance->x[1]<<16);
    // Increment pointers to source and destination data
    p_src += 16;
    p_dest += 16;
    }
}