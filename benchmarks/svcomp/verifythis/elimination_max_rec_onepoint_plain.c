/* elimination_max_rec_onepoint adapted: calloc->stack array, n bounded
 * Functions: check (recursive), main
 * Property: check returns index of maximum; a[i] <= a[result] for witness i
 * Key difference from elimination_max_rec: i is passed into check, so
 * check itself asserts a[i] <= a[result] at every recursion level.
 */
extern int __VERIFIER_nondet_int(void);
extern void abort(void);
void assume_abort_if_not(int cond) { if(!cond) abort(); }
void reach_error() { __ESBMC_assert(0, "reach_error"); }
void __VERIFIER_assert(int cond) { if(!cond) { reach_error(); abort(); } }

int check(int x, int y, int *a, int i, int n) {
    if (x >= y) return x;

    __VERIFIER_assert(0 <= x && y < n);
    assume_abort_if_not(0 <= i && i < n);

    if (a[x] <= a[y]) x++;
    else y--;

    int x1 = check(x, y, a, i, n);

    int ai = a[i];
    int ax = a[x1];
    __VERIFIER_assert(ai <= ax);

    return x1;
}

int main(void) {
    int n = __VERIFIER_nondet_int();
    assume_abort_if_not(n >= 1 && n <= 6);
    int i = __VERIFIER_nondet_int();
    assume_abort_if_not(0 <= i && i < n);
    int a[6];
    int x = check(0, n-1, a, i, n);
    int ai = a[i];
    int ax = a[x];
    __VERIFIER_assert(ai <= ax);
    return x;
}
