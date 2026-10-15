// Source: Costan, Gaubert, Goubault, Martel, Putot: "A Policy Iteration Algorithm", CAV 2005
// Loop invariant version
// Invariant: i + 2*j == 21 (preserved each iteration: (i+2) + 2*(j-1) = i+2j = 21)
// At exit (j < i): i+2j=21, j < 21-2j => 3j < 21 => j <= 6
// Boolector's bit-vector arithmetic derives j == 6 exactly from the invariant + exit condition.

#include "assert.h"

int main() {
    int i, j;
    i = 1;
    j = 10;
    __ESBMC_loop_invariant(i + 2 * j == 21);
    while (j >= i) {
        i = i + 2;
        j = -1 + j;
    }
    __VERIFIER_assert(j == 6);
    return 0;
}
