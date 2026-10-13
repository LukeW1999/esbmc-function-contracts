/* prefixsum_iter with ESBMC function contracts */
extern int __VERIFIER_nondet_int(void);
extern void abort(void);
void assume_abort_if_not(int cond) { if(!cond) abort(); }
void reach_error() { __ESBMC_assert(0, "reach_error"); }
void __VERIFIER_assert(int cond) { if(!cond) { reach_error(); abort(); } }

int is_pow2(unsigned int x)
{
    __ESBMC_ensures(__ESBMC_return_value == 0 | __ESBMC_return_value == 1);

    while (((x % 2) == 0) && x > 1)
        x /= 2;
    return (x == 1);
}

int upsweep(int *a, int n)
{
    __ESBMC_requires(a != (void*)0);
    __ESBMC_requires(n >= 1);
    __ESBMC_requires(n <= 8);
    __ESBMC_assigns(*a);
    __ESBMC_ensures(__ESBMC_return_value >= n);
    __ESBMC_ensures(__ESBMC_return_value <= 8);
    __ESBMC_ensures(__ESBMC_return_value >= 1);

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

void downsweep(int *a, int n, int space)
{
    __ESBMC_requires(a != (void*)0);
    __ESBMC_requires(n >= 1);
    __ESBMC_requires(n <= 8);
    __ESBMC_requires(space >= 1);
    __ESBMC_requires(space <= 8);
    __ESBMC_assigns(*a);

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

void check(int *a0, int *a, int n)
{
    __ESBMC_requires(a0 != (void*)0);
    __ESBMC_requires(a  != (void*)0);
    __ESBMC_requires(n >= 1);
    __ESBMC_requires(n <= 8);

    int i, sum = 0;
    for (i = 0; i < n; i++) {
        __VERIFIER_assert(sum == a[i]);
        sum += a0[i];
    }
}

int main(void) {
    int n = __VERIFIER_nondet_int();
    assume_abort_if_not(n >= 1 && n <= 8);
    assume_abort_if_not(is_pow2(n));

    int a[8], a0[8];
    int i;
    for (i = 0; i < n; i++)
        a0[i] = a[i];

    int space = upsweep(a, n);
    downsweep(a, n, space);
    check(a0, a, n);
    return 0;
}
