/* prefixsum_iter adapted: calloc->stack array, n bounded to power-of-2 <= 8
 * Functions: upsweep, downsweep, is_pow2, check
 * Property: prefix sum correctness (unreach-call, verdict=true)
 */
extern int __VERIFIER_nondet_int(void);
extern void abort(void);
void assume_abort_if_not(int cond) { if(!cond) abort(); }
void reach_error() { __ESBMC_assert(0, "reach_error"); }
void __VERIFIER_assert(int cond) { if(!cond) { reach_error(); abort(); } }

int upsweep(int *a, int n) {
    int space = 1;
    for (; space < n; space = space*2) {
        int left = space - 1;
        while (left < n) {
            int right = left + space;
            a[right] = a[left] + a[right];
            left = left + space*2;
        }
    }
    return space;
}

void downsweep(int *a, int n, int space) {
    a[n - 1] = 0;
    space = space/2;
    for (; space > 0; space = space/2) {
        int right = space*2 - 1;
        while (right < n) {
            int left = right - space;
            int temp = a[right];
            a[right] = a[left] + a[right];
            a[left] = temp;
            right = right + space*2;
        }
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
    assume_abort_if_not(n >= 1 && n <= 8);
    assume_abort_if_not(is_pow2(n));   /* n in {1,2,4,8} */

    int a[8], a0[8];
    int i;
    for (i = 0; i < n; i++)
        a0[i] = a[i];

    int space = upsweep(a, n);
    downsweep(a, n, space);
    check(a0, a, n);
    return 0;
}
