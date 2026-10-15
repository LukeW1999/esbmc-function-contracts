/* elimination_max_rec adapted: calloc->stack array, n bounded
 * Functions: check (recursive), main
 * Property: check returns index of maximum element
 */
extern int __VERIFIER_nondet_int(void);
extern void abort(void);
void assume_abort_if_not(int cond) { if(!cond) abort(); }
void reach_error() { __ESBMC_assert(0, "reach_error"); }
void __VERIFIER_assert(int cond) { if(!cond) { reach_error(); abort(); } }

int check(int x, int y, int *a, int n) {
    if (x >= y) return x;
    __VERIFIER_assert(0 <= x && y < n);
    int x0 = x, y0 = y;
    if (a[x] <= a[y]) x++;
    else y--;
    int x1 = check(x, y, a, n);
    int i = __VERIFIER_nondet_int();
    assume_abort_if_not(x0 <= i && i <= y0);
    __VERIFIER_assert(a[i] <= a[x1]);
    return x1;
}

int main(void) {
    int n = __VERIFIER_nondet_int();
    assume_abort_if_not(n >= 1 && n <= 6);   /* bound recursion depth */
    int a[6];
    int x = check(0, n-1, a, n);
    __ESBMC_assert(x >= 0 && x < n, "result index in bounds");
    return x;
}
