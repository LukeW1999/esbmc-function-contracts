/**
 * 完整程序 - 多次调用 (ESBMC)
 */
#include <assert.h>

int add_and_check(int a, int b, int limit) {
    __ESBMC_requires(a >= 0 && a <= 100);
    __ESBMC_requires(b >= 0 && b <= 100);
    __ESBMC_requires(limit > 0);
    
    __ESBMC_ensures(__ESBMC_return_value >= -1);
    __ESBMC_ensures(a + b <= limit || __ESBMC_return_value == -1);
    __ESBMC_ensures(a + b > limit || __ESBMC_return_value == a + b);
    
    int sum = a + b;
    if (sum <= limit) {
        return sum;
    } else {
        return -1;
    }
}

int main() {
    // 多次调用
    int r1 = add_and_check(10, 20, 50);
    int r2 = add_and_check(30, 40, 100);
    int r3 = add_and_check(50, 60, 80);   // 超限，返回-1
    int r4 = add_and_check(5, 5, 20);
    int r5 = add_and_check(1, 2, 10);
    int r6 = add_and_check(99, 99, 50);   // 超限，返回-1
    int r7 = add_and_check(25, 25, 100);
    int r8 = add_and_check(10, 10, 30);
    int r9 = add_and_check(80, 80, 100);  // 超限，返回-1
    int r10 = add_and_check(15, 15, 50);
    
    // ===== 调用后的验证代码 =====
    assert(r1 == 30);
    assert(r2 == 70);
    assert(r3 == -1);
    assert(r4 == 10);
    assert(r5 == 3);
    assert(r6 == -1);
    assert(r7 == 50);
    assert(r8 == 20);
    assert(r9 == -1);
    assert(r10 == 30);
    
    return 0;
}
