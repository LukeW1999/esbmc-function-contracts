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
    h = 99;  /* temporary write to h */
    h = 0;   /* restore h to original value */
}

int main(void) { foo(); }
