/* sum adapted from sv-benchmarks/c/recursive-simple/sum_10x0-1.c
 * Generic nondet inputs, n bounded to [0,10]
 * Property: sum(n, m) == n + m  (unreach-call, verdict=true)
 */
extern int __VERIFIER_nondet_int(void);
extern void abort(void);
void assume_abort_if_not(int cond) { if(!cond) abort(); }

int sum(int n, int m) {
    if (n <= 0) {
        return m + n;
    } else {
        return sum(n - 1, m + 1);
    }
}

int main(void) {
    int a = __VERIFIER_nondet_int();
    int b = __VERIFIER_nondet_int();
    assume_abort_if_not(a >= 0 && a <= 10);
    int result = sum(a, b);
    __ESBMC_assert(result == a + b, "sum(a,b) == a+b");
    return 0;
}
