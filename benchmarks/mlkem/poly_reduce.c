/* mlkem-native poly_reduce / polyvec_reduce, adapted for ESBMC contracts.
 *
 * Source: pq-code-package/mlkem-native  (mlkem/src/poly.c, poly_k.c)
 *   - mlk_barrett_reduce            : Barrett reduction to centered rep
 *   - mlk_scalar_signed_to_unsigned : map (-Q,Q) -> [0,Q)   (constant-time
 *                                     ct_sel/ct_cmask simplified to a branch;
 *                                     semantically identical for verification)
 *   - mlk_poly_reduce               : reduce all 256 coeffs of a polynomial
 *   - mlk_polyvec_reduce            : reduce each of the MLKEM_K components
 *
 * CBMC contracts (requires/assigns/ensures, array_bound) re-expressed with the
 * __ESBMC_* intrinsics.  __loop__ invariants are dropped: ESBMC unrolls the
 * fixed 256 / MLKEM_K bounds, so the bounded run is complete for these sizes.
 */
#include <stdint.h>
#include <stddef.h>

#ifndef MLKEM_K
#define MLKEM_K 3
#endif
#ifndef MLKEM_N
#define MLKEM_N      256
#endif
#define MLKEM_Q      3329
#define MLKEM_Q_HALF 1665   /* (MLKEM_Q + 1)/2 */

typedef struct { int16_t coeffs[MLKEM_N]; } mlk_poly;
typedef struct { mlk_poly vec[MLKEM_K];   } mlk_polyvec;

static int16_t mlk_barrett_reduce(int16_t a)
{
  const int32_t magic = 20159; /* round(2^26 / MLKEM_Q) */
  const int32_t t = (magic * a + ((int32_t)1 << 25)) >> 26;
  return (int16_t)(a - t * MLKEM_Q);
}

/* Contracts are active only with -DCONTRACTS (enforce / replace runs).
 * The Direct harness compiles them out, so it verifies the real bodies with
 * no contract intrinsics that could vacuously short-circuit the property. */
#ifdef CONTRACTS
#define REQUIRES(x)   __ESBMC_requires(x)
#define ENSURES(x)    __ESBMC_ensures(x)
#define ASSIGNS(...)  __ESBMC_assigns(__VA_ARGS__)
#else
#define REQUIRES(x)
#define ENSURES(x)
#define ASSIGNS(...)
#endif

static int16_t mlk_scalar_signed_to_unsigned_q(int16_t c)
{
  REQUIRES(c > -MLKEM_Q && c < MLKEM_Q);
  ENSURES(__ESBMC_return_value >= 0 && __ESBMC_return_value < MLKEM_Q);
  return (c < 0) ? (int16_t)(c + MLKEM_Q) : c;
}

void mlk_poly_reduce(mlk_poly *r)
{
  int i;   /* quantifier variable for the ensures clause */
  REQUIRES(r != NULL);
  ASSIGNS(r->coeffs);
  ENSURES(__ESBMC_forall(
      &i, !(i >= 0 && i < MLKEM_N) ||
          (r->coeffs[i] >= 0 && r->coeffs[i] < MLKEM_Q)));

  unsigned li;
  for (li = 0; li < MLKEM_N; li++)
  {
    int16_t t = mlk_barrett_reduce(r->coeffs[li]);
    r->coeffs[li] = mlk_scalar_signed_to_unsigned_q(t);
  }
}

void mlk_polyvec_reduce(mlk_polyvec *r)
{
  int j;   /* quantifier variable for the ensures clause */
  REQUIRES(r != NULL);
  ASSIGNS(r->vec);
  ENSURES(__ESBMC_forall(
      &j, !(j >= 0 && j < MLKEM_K * MLKEM_N) ||
          (r->vec[j / MLKEM_N].coeffs[j % MLKEM_N] >= 0 &&
           r->vec[j / MLKEM_N].coeffs[j % MLKEM_N] < MLKEM_Q)));

  unsigned k;
  for (k = 0; k < MLKEM_K; k++)
    mlk_poly_reduce(&r->vec[k]);
}

#ifdef HARNESS_DIRECT
int main(void)
{
  mlk_polyvec v;   /* nondeterministic contents */
  mlk_polyvec_reduce(&v);
  unsigned j;
  __ESBMC_assert(__ESBMC_forall(
      &j, !(j >= 0 && j < MLKEM_K * MLKEM_N) ||
          (v.vec[j / MLKEM_N].coeffs[j % MLKEM_N] >= 0 &&
           v.vec[j / MLKEM_N].coeffs[j % MLKEM_N] < MLKEM_Q)),
      "all coeffs reduced into [0,Q)");
  return 0;
}
#endif
