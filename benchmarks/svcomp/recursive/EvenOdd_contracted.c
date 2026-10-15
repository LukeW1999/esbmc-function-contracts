/* EvenOdd01-1 with ESBMC function contracts */
extern int __VERIFIER_nondet_int(void);
extern void abort(void);
void assume_abort_if_not(int cond) { if(!cond) abort(); }

int isOdd(int n);
int isEven(int n);

int isOdd(int n) {
    __ESBMC_requires(n >= 0);
    __ESBMC_requires(n <= 20);
    __ESBMC_ensures(__ESBMC_return_value == n % 2);
    if (n == 0) { return 0; }
    else if (n == 1) { return 1; }
    else { return isEven(n - 1); }
}

int isEven(int n) {
    __ESBMC_requires(n >= 0);
    __ESBMC_requires(n <= 20);
    __ESBMC_ensures(__ESBMC_return_value == 1 - n % 2);
    if (n == 0) { return 1; }
    else if (n == 1) { return 0; }
    else { return isOdd(n - 1); }
}

int main(void) {
    int n = __VERIFIER_nondet_int();
    assume_abort_if_not(n >= 0 && n <= 20);
    int result = isOdd(n);
    __ESBMC_assert(result == n % 2, "isOdd is correct");
    return 0;
}
