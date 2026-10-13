/**
 * 变体程序 - 多次调用，调用后内容删除 (CBMC)
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
    // 多次调用
    int r1 = add_and_check(10, 20, 50);
    int r2 = add_and_check(30, 40, 100);
    int r3 = add_and_check(50, 60, 80);
    int r4 = add_and_check(5, 5, 20);
    int r5 = add_and_check(1, 2, 10);
    int r6 = add_and_check(99, 99, 50);
    int r7 = add_and_check(25, 25, 100);
    int r8 = add_and_check(10, 10, 30);
    int r9 = add_and_check(80, 80, 100);
    int r10 = add_and_check(15, 15, 50);
    
    // ===== 调用后的验证代码已删除 =====
    
    return 0;
}
