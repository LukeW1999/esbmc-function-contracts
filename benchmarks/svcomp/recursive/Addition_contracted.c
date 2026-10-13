/* Addition01-2 with ESBMC function contracts */
extern int __VERIFIER_nondet_int(void);
extern void abort(void);
void assume_abort_if_not(int cond) { if(!cond) abort(); }

int addition(int m, int n) {
    __ESBMC_requires(n >= 0);
    __ESBMC_requires(n <= 10);
    __ESBMC_ensures(__ESBMC_return_value == m + n);
    if (n == 0) { return m; }
    if (n > 0) { return addition(m + 1, n - 1); }
    if (n < 0) { return addition(m - 1, n + 1); }
}

int main(void) {
    int m = __VERIFIER_nondet_int();
    assume_abort_if_not(m >= 0 && m <= 20);
    int n = __VERIFIER_nondet_int();
    assume_abort_if_not(n >= 0 && n <= 10);
    int result = addition(m, n);
    __ESBMC_assert(result == m + n, "addition is correct");
    return 0;
}
