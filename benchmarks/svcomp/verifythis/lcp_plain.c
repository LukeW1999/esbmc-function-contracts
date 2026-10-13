/* lcp baseline: no contracts, bounded n, stack array */
extern int __VERIFIER_nondet_int(void);
extern void abort(void);
void assume_abort_if_not(int cond) { if(!cond) abort(); }
void reach_error() { __ESBMC_assert(0, "reach_error"); }
void __VERIFIER_assert(int cond) { if(!cond) { reach_error(); abort(); } }

int lcp(int *a, int n, int x, int y) {
    int l = 0;
    while (x+l < n && y+l < n && a[x+l] == a[y+l])
        l++;
    return l;
}

void check(int *a, int n, int x, int y, int l) {
    int i = __VERIFIER_nondet_int();
    assume_abort_if_not(0 <= i && i < l);
    __VERIFIER_assert(x+i < n && y+i < n);
}

int main(void) {
    int n = __VERIFIER_nondet_int();
    assume_abort_if_not(n >= 2 && n <= 8);
    int a[8];
    int x = __VERIFIER_nondet_int();
    int y = __VERIFIER_nondet_int();
    assume_abort_if_not(x >= 0 && x < n);
    assume_abort_if_not(y >= 0 && y < n);
    int l = lcp(a, n, x, y);
    check(a, n, x, y, l);
    __ESBMC_assert(l >= 0,      "lcp non-negative");
    __ESBMC_assert(x + l <= n,  "lcp bounded by x");
    __ESBMC_assert(y + l <= n,  "lcp bounded by y");
    return 0;
}
