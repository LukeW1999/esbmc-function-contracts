/* Counterexample-agreement / seeded-mutation harness.
 * Corroborates Proposition 1 (relative soundness) on the real ESBMC
 * implementation: for every seeded property violation that monolithic (Direct)
 * BMC catches, the modular pipeline (Enforce + Replace) must catch it too
 * (no false negatives). Mutants selected via -DMUT; contracts via -DCONTRACTS;
 * harness via -DHARNESS.
 */
#include <stddef.h>
#ifndef N
#define N 8
#endif
#ifndef K
#define K 2
#endif
#ifndef MUT
#define MUT 0
#endif

int g_processed, g_other;

#ifdef CONTRACTS
#define REQUIRES(x)  __ESBMC_requires(x)
#define ENSURES(x)   __ESBMC_ensures(x)
#define ASSIGNS(...) __ESBMC_assigns(__VA_ARGS__)
#else
#define REQUIRES(x)
#define ENSURES(x)
#define ASSIGNS(...)
#endif

int compute(int *arr, int n, int lo, int hi)
{
  REQUIRES(__ESBMC_is_fresh(arr, n * sizeof(int)) && n > 0 && n <= N && lo >= 0 && lo <= hi && hi <= 100);
  ASSIGNS(g_processed);
#if MUT==4
  ENSURES(__ESBMC_return_value >= n * lo);              /* M4: WEAK (no upper) */
#else
  ENSURES(__ESBMC_return_value >= n * lo && __ESBMC_return_value <= n * hi);
#endif
  int sum = 0;
  for (int i = 0; i < n; i++) {
    int v = arr[i];
    if (v < lo) v = lo;
    if (v > hi) v = hi;
    sum += v;
  }
  g_processed = n;
#if MUT==3
  g_other = 1;                 /* M3: write OUTSIDE assigns clause */
#endif
#if MUT==5
  g_other = 99; g_other = 0;   /* M5: temporary write, restored before exit */
#endif
#if MUT==1
  return sum + n;              /* M1: can exceed n*hi (upper-bound bug) */
#endif
#if MUT==2
  return sum - 1;              /* M2: can fall below n*lo (lower-bound bug) */
#endif
  return sum;                  /* M0 clean / M3 / M4 / M5 */
}

#ifdef HARNESS
int main(void)
{
  int buf[K][N];
  int len, lo, hi;
  if (!(len > 0 && len <= N)) return 0;
  if (!(lo >= 0 && lo <= 50)) return 0;
  if (!(hi >= 51 && hi <= 100)) return 0;
  for (int k = 0; k < K; k++) {
    int r = compute(buf[k], len, lo, hi);
    __ESBMC_assert(r >= len * lo && r <= len * hi, "postcondition bound");
  }
  return 0;
}
#endif
