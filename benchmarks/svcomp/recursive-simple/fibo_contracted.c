/* fibo with ESBMC function contracts */

int fibo(int n) {
    __ESBMC_requires(n >= 0);
    __ESBMC_ensures(__ESBMC_return_value >= 0);
    /* ensures the Fibonacci identity */
    __ESBMC_ensures(n == 0 ? __ESBMC_return_value == 0 :
                    n == 1 ? __ESBMC_return_value == 1 : 1);

    if (n < 1) {
        return 0;
    } else if (n == 1) {
        return 1;
    } else {
        return fibo(n-1) + fibo(n-2);
    }
}

int main(void) {
    int x = 10;
    int result = fibo(x);
    __ESBMC_assert(result == 55, "fibo(10) must equal 55");
    return 0;
}
