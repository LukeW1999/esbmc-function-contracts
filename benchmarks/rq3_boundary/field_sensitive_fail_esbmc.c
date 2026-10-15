/**
 * ESBMC Contract 字段敏感测试 - FAIL 用例
 * 
 * 这些测试应该 FAIL，用于验证 assigns 子句的正确性:
 * 1. assigns 声明了变量但 ensures 没有约束 -> havoc 后值不确定
 * 2. 验证字段敏感性: 不在 assigns 中的字段应保持不变
 * 
 * 运行命令:
 *   esbmc field_sensitive_fail_test.c --replace-call-with-contract "*"
 */

#include <assert.h>

/*============================================================================
 * FAIL 测试1: assigns 了但 ensures 没约束 -> 值被 havoc
 *============================================================================*/

int global_x = 0;
int global_y = 0;

void weak_ensures_contract(void) {
    __ESBMC_assigns(global_x, global_y);  // 声明修改 x 和 y
    __ESBMC_ensures(global_x == 42);      // 只约束了 x，没约束 y!
    
    global_x = 42;
    global_y = 100;  // 实现中设置为100，但 ensures 没说
}

/*============================================================================
 * FAIL 测试2: assigns 了标量但 ensures 是 trivial 的
 *============================================================================*/

int scalar_var = 0;

void trivial_ensures(void) {
    __ESBMC_assigns(scalar_var);
    __ESBMC_ensures(1);  // trivial ensures，不约束 scalar_var
    
    scalar_var = 999;
}

/*============================================================================
 * 主函数
 *============================================================================*/

int main() {
    // FAIL 测试1: global_y 被 havoc 但没被约束
    global_x = 10;
    global_y = 20;
    weak_ensures_contract();
    
    assert(global_x == 42);   // PASS: ensures 约束了
    assert(global_y == 100);  // FAIL: ensures 没约束，y 可能是任意值
    
    // FAIL 测试2: scalar_var 被 havoc 但没被约束
    scalar_var = 50;
    trivial_ensures();
    assert(scalar_var == 999);  // FAIL: ensures 没约束
    
    return 0;
}
