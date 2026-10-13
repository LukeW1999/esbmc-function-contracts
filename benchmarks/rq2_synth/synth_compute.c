/* Synthetic running example + RQ2 scalability benchmark.
 * compute() clamps & sums an array (loop bound N) AND updates a global
 * g_processed that it declares in its assigns clause -- so the example
 * genuinely exercises assigns/frame checking (addresses the running-example
 * critique). Parameterised by N (loop bound) and K (call count) via -D.
 *
 * Direct harness (-DHARNESS_DIRECT, contracts compiled out) verifies the real
 * bodies -- non-vacuous (validated both ways). Enforce/Replace use -DCONTRACTS.
 */
#include <stddef.h>

#ifndef N
#define N 10
#endif
#ifndef K
#define K 1
#endif

int g_processed;        /* global the function is permitted to modify */

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
  REQUIRES(arr != NULL && n > 0 && n <= N && lo >= 0 && lo <= hi && hi <= 100);
  ASSIGNS(g_processed);                       /* frame: only g_processed */
  ENSURES(__ESBMC_return_value >= n * lo && __ESBMC_return_value <= n * hi);

  int sum = 0;
  for (int i = 0; i < n; i++) {
    int v = arr[i];
    if (v < lo) v = lo;
    if (v > hi) v = hi;
    sum += v;
  }
  g_processed = n;                            /* the only global write */
  return sum;
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
    __ESBMC_assert(r >= len * lo && r <= len * hi, "sum in [n*lo, n*hi]");
  }
  return 0;
}
#endif
