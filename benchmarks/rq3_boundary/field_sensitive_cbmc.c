/**
 * CBMC Contract 字段敏感测试
 * 
 * 测试目标:
 * 1. 结构体字段精确 assigns (字段敏感)
 * 2. 数组元素精确 assigns
 * 3. __CPROVER_old 引用旧值
 * 4. 多字段 assigns
 * 
 * 运行命令:
 *   goto-cc field_sensitive_test.c -o test.goto
 *   goto-instrument --replace-call-with-contract set_x test.goto t1.goto
 *   goto-instrument --replace-call-with-contract modify_array_element t1.goto t2.goto
 *   goto-instrument --replace-call-with-contract update_multiple_fields t2.goto t3.goto
 *   goto-instrument --replace-call-with-contract modify_global_and_ptr t3.goto t4.goto
 *   goto-instrument --replace-call-with-contract increment_counter t4.goto final.goto
 *   cbmc final.goto
 */

#include <assert.h>
#include <stddef.h>

/*============================================================================
 * 测试1: 结构体单字段精确修改 (字段敏感)
 *============================================================================*/

typedef struct {
    int x;
    int y;
    int z;
} Point;

/**
 * 只修改结构体的 x 字段，y 和 z 保持不变
 */
void set_x(Point *p, int val)
__CPROVER_requires(p != ((void*)0))
__CPROVER_assigns(p->x)
__CPROVER_ensures(p->x == val)
__CPROVER_ensures(p->y == __CPROVER_old(p->y))
__CPROVER_ensures(p->z == __CPROVER_old(p->z))
{
    p->x = val;
}

/*============================================================================
 * 测试2: 数组元素精确修改 (元素敏感)
 *============================================================================*/

int global_array[5];

/**
 * 只修改数组的第 i 个元素，其他元素保持不变
 */
void modify_array_element(int i, int val)
__CPROVER_requires(i >= 0 && i < 5)
__CPROVER_assigns(global_array[i])
__CPROVER_ensures(global_array[i] == val)
{
    global_array[i] = val;
}

/*============================================================================
 * 测试3: 多字段精确修改
 *============================================================================*/

typedef struct {
    int a;
    int b;
    int c;
    int d;
} MultiField;

/**
 * 只修改 a 和 c 字段，b 和 d 保持不变
 */
void update_multiple_fields(MultiField *m, int new_a, int new_c)
__CPROVER_requires(m != ((void*)0))
__CPROVER_assigns(m->a, m->c)
__CPROVER_ensures(m->a == new_a)
__CPROVER_ensures(m->c == new_c)
__CPROVER_ensures(m->b == __CPROVER_old(m->b))
__CPROVER_ensures(m->d == __CPROVER_old(m->d))
{
    m->a = new_a;
    m->c = new_c;
}

/*============================================================================
 * 测试4: 全局变量 + 指针参数混合 assigns
 *============================================================================*/

int global_x = 0;
int global_y = 0;

void modify_global_and_ptr(int *ptr)
__CPROVER_requires(ptr != ((void*)0))
__CPROVER_assigns(global_x, *ptr)
__CPROVER_ensures(global_x == 42)
__CPROVER_ensures(*ptr == 100)
{
    global_x = 42;
    *ptr = 100;
}

/*============================================================================
 * 测试5: 带条件的后置条件 + 复杂 __CPROVER_old
 *============================================================================*/

typedef struct {
    int count;
    int max_count;
} Counter;

/**
 * 增加计数器，但有上限
 */
void increment_counter(Counter *c)
__CPROVER_requires(c != ((void*)0))
__CPROVER_requires(c->count >= 0)
__CPROVER_requires(c->max_count > 0)
__CPROVER_assigns(c->count)
__CPROVER_ensures(__CPROVER_old(c->count) >= c->max_count || 
                  c->count == __CPROVER_old(c->count) + 1)
__CPROVER_ensures(__CPROVER_old(c->count) < c->max_count || 
                  c->count == __CPROVER_old(c->count))
__CPROVER_ensures(c->max_count == __CPROVER_old(c->max_count))
{
    if (c->count < c->max_count) {
        c->count++;
    }
}

/*============================================================================
 * 主函数: 测试所有场景
 *============================================================================*/

int main() {
    // 测试1: 结构体单字段 (字段敏感关键测试!)
    Point pt = {1, 2, 3};
    set_x(&pt, 99);
    assert(pt.x == 99);  // 已修改
    assert(pt.y == 2);   // 未改变 - 字段敏感验证
    assert(pt.z == 3);   // 未改变 - 字段敏感验证
    
    // 测试2: 数组元素 (元素敏感关键测试!)
    global_array[0] = 10;
    global_array[1] = 20;
    global_array[2] = 30;
    modify_array_element(1, 999);
    assert(global_array[0] == 10);   // 未改变 - 元素敏感验证
    assert(global_array[1] == 999);  // 已修改
    assert(global_array[2] == 30);   // 未改变 - 元素敏感验证
    
    // 测试3: 多字段
    MultiField mf = {1, 2, 3, 4};
    update_multiple_fields(&mf, 100, 300);
    assert(mf.a == 100); // 已修改
    assert(mf.b == 2);   // 未改变 - 字段敏感验证
    assert(mf.c == 300); // 已修改
    assert(mf.d == 4);   // 未改变 - 字段敏感验证
    
    // 测试4: 全局变量 + 指针
    int local = 0;
    global_x = 0;
    global_y = 200;
    modify_global_and_ptr(&local);
    assert(global_x == 42);
    assert(local == 100);
    assert(global_y == 200);  // 未改变 - 不在 assigns 中
    
    // 测试5: 条件后置条件
    Counter cnt = {50, 100};
    increment_counter(&cnt);
    assert(cnt.count == 51);      // 已增加
    assert(cnt.max_count == 100); // 未改变
    
    return 0;
}
