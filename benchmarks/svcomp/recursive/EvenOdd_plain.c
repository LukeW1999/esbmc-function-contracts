/* EvenOdd01-1 adapted from sv-benchmarks/c/recursive/EvenOdd01-1.c
 * Bounds tightened: n <= 20 to make BMC feasible
 * Property: isOdd(n) == n % 2  (mutual recursion: isOdd <-> isEven)
 */
extern int __VERIFIER_nondet_int(void);
extern void abort(void);
void assume_abort_if_not(int cond) { if(!cond) abort(); }

int isOdd(int n);
int isEven(int n);

int isOdd(int n) {
    if (n == 0) { return 0; }
    else if (n == 1) { return 1; }
    else { return isEven(n - 1); }
}

int isEven(int n) {
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
