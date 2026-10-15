// Source: Costan, Gaubert, Goubault, Martel, Putot: "A Policy Iteration Algorithm", CAV 2005
// Plain version — BMC needs --unwind 5 (loop runs exactly 4 times)

#include "assert.h"

int main() {
    int i, j;
    i = 1;
    j = 10;
    while (j >= i) {
        i = i + 2;
        j = -1 + j;
    }
    __VERIFIER_assert(j == 6);
    return 0;
}
