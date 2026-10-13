/**
 * 无Contract版本 - 单次调用 (CBMC)
 */
#include <assert.h>

int add_and_check(int a, int b, int limit) {
    int sum = a + b;
    if (sum <= limit) {
        return sum;
    } else {
        return -1;
    }
}

int main() {
    int result = add_and_check(10, 20, 50);
    
    assert(result >= -1);
    assert(result == 30);
    
    return 0;
}
