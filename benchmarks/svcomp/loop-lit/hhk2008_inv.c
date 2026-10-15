// Source: Henzinger, Hottelier, Kovacs: "Valigator", LPAR 2008
// Loop invariant version — proves for ALL b <= 1000000 without unrolling

#include "assert.h"

extern int __VERIFIER_nondet_int(void);

int main() {
    int a = __VERIFIER_nondet_int();
    int b = __VERIFIER_nondet_int();
    int res, cnt;
    if (!(a <= 1000000)) return 0;
    if (!(0 <= b && b <= 1000000)) return 0;
    res = a;
    cnt = b;
    // Invariant: res + cnt == a + b (preserved each iteration; at exit cnt==0 => res==a+b)
    __ESBMC_loop_invariant(res + cnt == a + b);
    while (cnt > 0) {
        cnt = cnt - 1;
        res = res + 1;
    }
    __VERIFIER_assert(res == a + b);
    return 0;
}
