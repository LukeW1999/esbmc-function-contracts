/* Soundness boundary test: temporary modification then restore
 *
 * foo() temporarily sets h = 99, then restores the value it found.
 * Assigns clause only declares g.
 *
 * ESBMC (black-box/observational): PASS — h == 0 at function exit
 * CBMC (white-box/DFCC): FAIL — h is written inside function body
 *
 * This demonstrates the semantic difference between:
 * - Observational compliance (our approach): only final state matters
 * - Strict write-set compliance (CBMC DFCC): ANY write is a violation
 */

int g = 0;
int h = 0;

void foo(void)
{
    __ESBMC_assigns(g);
    __ESBMC_ensures(g == 42);

    g = 42;
    int t = h;  /* temporary write to h, restored to the value found */
    h = 99;
    h = t;
}

int main(void)
{
    foo();
    return 0;
}
