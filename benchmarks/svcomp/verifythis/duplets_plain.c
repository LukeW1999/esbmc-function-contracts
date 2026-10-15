/* duplets adapted: calloc->stack array, n bounded
 * Functions: mkdup, finddup, main
 * Property: finddup always finds the duplicate planted by mkdup (unreach-call, verdict=true)
 */
extern int __VERIFIER_nondet_int(void);
extern void abort(void);
void assume_abort_if_not(int cond) { if(!cond) abort(); }
void reach_error() { __ESBMC_assert(0, "reach_error"); }
void __VERIFIER_assert(int cond) { if(!cond) { reach_error(); abort(); } }

void mkdup(int *a, int n) {
    int i = __VERIFIER_nondet_int();
    int j = __VERIFIER_nondet_int();
    assume_abort_if_not(0 <= i && i < n);
    assume_abort_if_not(0 <= j && j < n);
    assume_abort_if_not(i != j);
    int x = __VERIFIER_nondet_int();
    a[i] = x;
    a[j] = x;
}

int finddup(int *a, int n, int *_i, int *_j) {
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = i+1; j < n; j++) {
            if (a[i] == a[j]) {
                *_i = i;
                *_j = j;
                return 1;
            }
        }
    }
    return 0;
}

int main(void) {
    int n = __VERIFIER_nondet_int();
    assume_abort_if_not(n >= 2 && n <= 8);
    int a[8];
    mkdup(a, n);
    int i, j;
    int r = finddup(a, n, &i, &j);
    __VERIFIER_assert(r);
    __VERIFIER_assert(0 <= i && i < n);
    __VERIFIER_assert(0 <= j && j < n);
    __VERIFIER_assert(i != j);
    __VERIFIER_assert(a[i] == a[j]);
    return 0;
}
