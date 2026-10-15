/**
 * 完整程序 - 单次调用 (CBMC)
 * 简化版本：无循环，避免loop contract复杂性
 */
#include <assert.h>

int add_and_check(int a, int b, int limit)
__CPROVER_requires(a >= 0 && a <= 100)
__CPROVER_requires(b >= 0 && b <= 100)
__CPROVER_requires(limit > 0)
__CPROVER_assigns()
__CPROVER_ensures(__CPROVER_return_value >= -1)
__CPROVER_ensures(a + b <= limit ==> __CPROVER_return_value == a + b)
__CPROVER_ensures(a + b > limit ==> __CPROVER_return_value == -1)
{
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
