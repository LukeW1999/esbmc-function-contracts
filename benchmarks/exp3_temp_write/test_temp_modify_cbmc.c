/* Soundness boundary test: temporary modification then restore
 *
 * CBMC DFCC should FAIL because h is written even though it's restored.
 */

int g = 0;
int h = 0;

void foo(void)
__CPROVER_assigns(g)
__CPROVER_ensures(g == 42)
{
    g = 42;
    int t = h;  /* temporary write to h, restored to the value found */
    h = 99;
    h = t;
}

int main(void) { foo(); }
