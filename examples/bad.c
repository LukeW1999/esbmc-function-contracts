#include <assert.h>
#include <stdlib.h>

int g_processed;
int g_other;

int compute(int *arr, int n, int lo, int hi)
{
  __ESBMC_requires(__ESBMC_is_fresh(arr, n * sizeof(int))
                   && n > 0 && n <= 8
                   && lo >= 0 && lo <= hi && hi <= 100);
  __ESBMC_assigns(g_processed);
  __ESBMC_ensures(__ESBMC_return_value >= n * lo
                  && __ESBMC_return_value <= n * hi);
  int sum = 0;
  for (int i = 0; i < n; i++) {
    int v = arr[i];
    if (v < lo) v = lo;
    if (v > hi) v = hi;
    sum += v;
  }
  g_processed = n;
  g_other = 7;
  return sum;
}

int main(void)
{
  int a[4] = {1, 2, 3, 4};
  int s = compute(a, 4, 0, 100);
  assert(s >= 0 && s <= 400);
  return 0;
}
