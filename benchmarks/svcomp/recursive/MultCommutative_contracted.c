/* MultCommutative with ESBMC function contracts */
extern int __VERIFIER_nondet_int(void);
extern void abort(void);
void assume_abort_if_not(int cond) { if(!cond) abort(); }

int mult(int n, int m)
{
    __ESBMC_requires(n >= 0);
    __ESBMC_requires(m >= 0);
    __ESBMC_requires(n <= 10);
    __ESBMC_requires(m <= 10);
    __ESBMC_ensures(__ESBMC_return_value == n * m);

    if (m < 0) {
        return mult(n, -m);
    }
    if (m == 0) {
        return 0;
    }
    return n + mult(n, m - 1);
}

int main(void) {
    int m = __VERIFIER_nondet_int();
    assume_abort_if_not(m >= 0 && m <= 10);
    int n = __VERIFIER_nondet_int();
    assume_abort_if_not(n >= 0 && n <= 10);
    int res1 = mult(m, n);
    int res2 = mult(n, m);
    __ESBMC_assert(res1 == res2, "mult is commutative");
    return 0;
}
