/* Adapted from sv-benchmarks/c/recursive-simple/fibo_10-2.c
 * Property: fibo(10) == 55  (must hold, no error reachable)
 * Converted reach_error() -> __ESBMC_assert for ConVer pipeline
 */

int fibo(int n) {
    if (n < 1) {
        return 0;
    } else if (n == 1) {
        return 1;
    } else {
        return fibo(n-1) + fibo(n-2);
    }
}

int main(void) {
    int x = 10;
    int result = fibo(x);
    __ESBMC_assert(result == 55, "fibo(10) must equal 55");
    return 0;
}
