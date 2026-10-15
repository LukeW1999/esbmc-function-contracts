/* lcp.c adapted from sv-benchmarks/c/verifythis/lcp.c
 * Contracts added for ESBMC function-level verification
 * lcp() called from check() - multiple call sites
 */

extern void abort(void);
void reach_error() { __ESBMC_assert(0, "reach_error"); }
void __VERIFIER_assert(int cond) { if(!cond) { reach_error(); abort(); } }

extern int __VERIFIER_nondet_int(void);
void assume_abort_if_not(int cond) { if(!cond) { abort(); } }

/* lcp: returns length of longest common prefix of a[x..] and a[y..] */
int lcp(int *a, int n, int x, int y)
{
    __ESBMC_requires(a != (void*)0);
    __ESBMC_requires(n >= 0);
    __ESBMC_requires(n <= 8);
    __ESBMC_requires(x >= 0 && x <= n);
    __ESBMC_requires(y >= 0 && y <= n);

    __ESBMC_ensures(__ESBMC_return_value >= 0);
    __ESBMC_ensures(x + __ESBMC_return_value <= n);
    __ESBMC_ensures(y + __ESBMC_return_value <= n);

    int l = 0;
    while (x+l < n && y+l < n && a[x+l] == a[y+l])
        l++;
    return l;
}

void check(int *a, int n, int x, int y, int l)
{
    __ESBMC_requires(a != (void*)0);
    __ESBMC_requires(n >= 0);
    __ESBMC_requires(n <= 8);
    __ESBMC_requires(l >= 0);
    __ESBMC_requires(x + l <= n);
    __ESBMC_requires(y + l <= n);

    int i = __VERIFIER_nondet_int();
    assume_abort_if_not(0 <= i && i < l);
    /* key: a[x+i] == a[y+i] for all i < l - expressed as scalar index check */
    __VERIFIER_assert(x+i < n && y+i < n);
}

int main(void)
{
    int n = __VERIFIER_nondet_int();
    assume_abort_if_not(n >= 2 && n <= 8);          /* bound for BMC */

    int a[8];                                         /* stack array, no calloc */
    int x = __VERIFIER_nondet_int();
    int y = __VERIFIER_nondet_int();
    assume_abort_if_not(x >= 0 && x < n);
    assume_abort_if_not(y >= 0 && y < n);

    int l = lcp(a, n, x, y);
    check(a, n, x, y, l);

    __ESBMC_assert(l >= 0, "lcp non-negative");
    __ESBMC_assert(x + l <= n, "lcp bounded by x");
    __ESBMC_assert(y + l <= n, "lcp bounded by y");
    return 0;
}
