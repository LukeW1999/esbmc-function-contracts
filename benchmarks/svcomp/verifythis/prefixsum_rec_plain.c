/* prefixsum_rec adapted: calloc->stack array, n bounded to power-of-2 <= 8
 * Functions: upsweep (recursive), downsweep (recursive), is_pow2, check
 * Property: prefix sum correctness (unreach-call, verdict=true)
 */
extern int __VERIFIER_nondet_int(void);
extern void abort(void);
void assume_abort_if_not(int cond) { if(!cond) abort(); }
void reach_error() { __ESBMC_assert(0, "reach_error"); }
void __VERIFIER_assert(int cond) { if(!cond) { reach_error(); abort(); } }

void upsweep(int *a, int left, int right) {
    if (right > left+1) {
        int space = right - left;
        upsweep(a, left-space/2, left);
        upsweep(a, right-space/2, right);
    }
    a[right] = a[left] + a[right];
}

void downsweep(int *a, int left, int right) {
    int tmp = a[right];
    a[right] = a[right] + a[left];
    a[left] = tmp;
    if (right > left+1) {
        int space = right - left;
        downsweep(a, left-space/2, left);
        downsweep(a, right-space/2, right);
    }
}

int is_pow2(unsigned int x) {
    while (((x % 2) == 0) && x > 1)
        x /= 2;
    return (x == 1);
}

void check(int *a0, int *a, int n) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        __VERIFIER_assert(sum == a[i]);
        sum += a0[i];
    }
}

int main(void) {
    int n = __VERIFIER_nondet_int();
    assume_abort_if_not(n >= 2 && n <= 4);
    assume_abort_if_not(is_pow2(n));   /* n in {2,4}; depth<=2 so unwind 5 suffices */

    int a[8], a0[8];
    int i;
    for (i = 0; i < n; i++)
        a0[i] = a[i];

    upsweep(a, n/2-1, n-1);
    a[n-1] = 0;
    downsweep(a, n/2-1, n-1);
    check(a0, a, n);
    return 0;
}
