/**
 * 完整程序 - 单次调用 (ESBMC)
 * 简化版本：无循环，与CBMC使用相同例子
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
    // 单次调用
    int result = add_and_check(10, 20, 50);
    
    // ===== 调用后的验证代码 =====
    assert(result >= -1);
    assert(result == 30);
    
    return 0;
}
