#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char*out, size_t cap) {
    uint32_t values[MAX_FIELDS]={0};
    size_t count=1;
    uint32_t L=(uint32_t)challenge;
    uint32_t H=(uint32_t)(challenge>>32);
    uint32_t M=(uint32_t)(challenge>>16);
    uint32_t t1=(M^0x63e7u)+H*8u;
    uint32_t t0=(L*3u)^(H+0x31cau);
    uint32_t result=(t0+t1*8u)^0x1f14u;
    values[0]=result;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char*out, size_t cap) {
    uint32_t values[MAX_FIELDS]={0};
    size_t count=6;
    (void)challenge;
    values[0]=(uint32_t)((challenge>>42)&0xfu)^0x10u;
    for (unsigned int i=1; i<6;++i) {
        unsigned int shift=20u+3u*(i-1u);
        uint32_t part=(uint32_t)((challenge>>shift)&0x3fu);
        uint32_t mixed=(part+i)^values[i-1];
        values[i]=mixed*5u;
    }
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char*out, size_t cap) {
    uint32_t values[MAX_FIELDS]={0};
    size_t count=2;
    (void)challenge;
    uint32_t index=(uint32_t)challenge&7u;
    uint32_t x=(uint32_t)(challenge>>32)^(uint32_t)(challenge>>3);
    uint32_t result;
    switch (index) {
        case 0: result=5u*x+0x2c7eu; break;
        case 1: result=(2u*x)^0x7b2u; break;
        case 2: result=5u*x+0x3cf37u; break;
        case 3: result=5u*x+0x26adeu; break;
        case 4: result=5u*x+0xa423u; break;
        case 5: result=(x^0x2fa6u)+(x>>2); break;
        case 6: result=(2u*x)^0xb661u; break;
        case 7: result=(x^0x4948u)+(x>>2); break;
        default: return 1;
    }
    values[0]=index;
    values[1]=result;
    return write_answer(out, cap, values, count);
}

static uint32_t phase4_walk(uint32_t target, uint32_t low, uint32_t high, uint32_t key) {
    uint32_t mid=low+((high-low)>>1);
    if (target==mid) {
        return (mid+0xe3u)^key;
    }
    uint32_t saved=mid+key;
    uint32_t child;
    if (target<mid) {
        child=phase4_walk(target, low, mid-1u, key);
        return (child*3u)^saved^3u;
    }
    child=phase4_walk(target, mid+1u, high, key);
    return (child*3u)^saved^0xcu;
}

int solve_phase_4(uint64_t challenge, char*out, size_t cap) {
    uint32_t values[MAX_FIELDS]={0};
    size_t count=1;
    (void)challenge;
    uint32_t target=(uint32_t)challenge&0x7fu;
    uint32_t key=(uint32_t)(challenge>>7)^(uint32_t)(challenge>>32);
    values[0]=phase4_walk(target, 0u, 127u, key);
    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char*out, size_t cap) {
    uint32_t values[MAX_FIELDS]={0};
    size_t count=2;
    (void)challenge;
    static const struct {
        int next;
        uint32_t field8, field12;
    } nodes[12]={
        {9, 0xb20u, 0x5aaeu}, {5, 0x305u, 0xa436u},
        {1, 0x14eu, 0x8b40u}, {-1, 0x4bu, 0x804eu},
        {10, 0x280u, 0xe4ddu}, {8, 0x81bu, 0x50u},
        {3, 0x76au, 0xc595u}, {11, 0x90du, 0xde9eu},
        {7, 0x61eu, 0x4f31u}, {2, 0xa44u, 0xc340u},
        {0, 0x45bu, 0xc96au}, {6, 0x55bu, 0x8086u}
    };
    static const int heads[8]={4, 10, 0, 9, 2, 1, 5, 8};
    uint32_t acc=(uint32_t)(challenge>>6)^(uint32_t)(challenge>>32)^0x141du;
    uint32_t moves=(uint32_t)(challenge>>3)&3u;
    int p=heads[challenge&7u];
    for (uint32_t i=0; i<=moves;++i) {
        if (p<0||p>=12) return 1;
        acc=(acc*3u+nodes[p].field12)^nodes[p].field8;
        values[1]=nodes[p].field8;
        if (i<moves) p=nodes[p].next;
    }
    values[0]=acc;
    return write_answer(out, cap, values, count);
}

static uint32_t phase6_op(unsigned int op, uint32_t x) {
    switch (op) {
        case 0: return (x^0x6af1u)+(x<<7);
        case 1: return ((x<<3)|(x>>29))^0x419u;
        case 2: return ((x>>2)^x)+0x6684u;
        case 3: return (4u*x+0x90a9u)^(3u*x);
        case 4: return (x>>5)|(x<<27);
        case 5: return (x+0x76a0u)^(x>>7);
        default: return 0;
    }
}

int solve_phase_6(uint64_t challenge, char*out, size_t cap) {
    static const unsigned int ops[8]={5, 4, 3, 1, 3, 0, 4, 2};
    uint32_t values[MAX_FIELDS]={0};
    size_t count=1;
    (void)challenge;
    uint32_t x=(uint32_t)(challenge>>6)^(uint32_t)(challenge>>32);
    x=phase6_op(ops[challenge&7u], x);
    x=phase6_op(ops[(challenge>>3)&7u], x^0x1527u);
    values[0]=x+0x7b22u;
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char*out, size_t cap) {
    uint32_t a=(uint32_t)(challenge>>32)^(uint32_t)challenge;
    uint32_t b=(uint32_t)(context>>32)^(uint32_t)context;
    uint32_t k=(uint32_t)context&3u;
    uint32_t x=a^b;
    uint32_t mixed=((x>>5)^0x3adcu)+(x^0x4f64u);
    uint32_t second=mixed^((k*0x4f64u)^(uint32_t)(context>>16));
    uint32_t third=second^(0x3adcu+b+k+(a>>5));
    int size=snprintf(out, cap, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN%" PRIu32
                        " %" PRIu32 " %" PRIu32, k, second, third);
    return size<0||(size_t)size>=cap;
}
