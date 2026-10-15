/* sum with ESBMC function contracts */
extern int __VERIFIER_nondet_int(void);
extern void abort(void);
void assume_abort_if_not(int cond) { if(!cond) abort(); }

int sum(int n, int m)
{
    __ESBMC_requires(n >= 0);
    __ESBMC_requires(n <= 10);
    __ESBMC_ensures(__ESBMC_return_value == n + m);

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
