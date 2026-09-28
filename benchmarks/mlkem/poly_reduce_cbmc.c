/* CBMC/DFCC baseline: same mlkem poly_reduce / polyvec_reduce.
 * Loop invariants are intentionally omitted: this is the BMC-vs-BMC
 * comparison, so CBMC unrolls the fixed bounds via --unwind, exactly like
 * ESBMC. DFCC (operational assigns) is the contract mechanism. */
#include <stdint.h>
#include <stddef.h>

#ifndef MLKEM_K
#define MLKEM_K 3
#endif
#ifndef MLKEM_N
#define MLKEM_N 256
#endif
#define MLKEM_Q 3329

typedef struct { int16_t coeffs[MLKEM_N]; } mlk_poly;
typedef struct { mlk_poly vec[MLKEM_K];   } mlk_polyvec;

static int16_t mlk_barrett_reduce(int16_t a)
{
  const int32_t magic = 20159;
  const int32_t t = (magic * a + ((int32_t)1 << 25)) >> 26;
  return (int16_t)(a - t * MLKEM_Q);
}

static int16_t mlk_scalar_signed_to_unsigned_q(int16_t c)
{
  return (c < 0) ? (int16_t)(c + MLKEM_Q) : c;
}

void mlk_poly_reduce(mlk_poly *r)
#ifdef CONTRACTS
  __CPROVER_requires(r != NULL)
  __CPROVER_assigns(*r)
  __CPROVER_ensures(__CPROVER_forall {
      unsigned i; (i < MLKEM_N) ==> (r->coeffs[i] >= 0 && r->coeffs[i] < MLKEM_Q) })
#endif
{
  unsigned li;
  for (li = 0; li < MLKEM_N; li++)
  {
    int16_t t = mlk_barrett_reduce(r->coeffs[li]);
    r->coeffs[li] = mlk_scalar_signed_to_unsigned_q(t);
  }
}

void mlk_polyvec_reduce(mlk_polyvec *r)
#ifdef CONTRACTS
  __CPROVER_requires(r != NULL)
  __CPROVER_assigns(*r)
  __CPROVER_ensures(__CPROVER_forall {
      unsigned j; (j < MLKEM_K * MLKEM_N) ==>
        (r->vec[j / MLKEM_N].coeffs[j % MLKEM_N] >= 0 &&
         r->vec[j / MLKEM_N].coeffs[j % MLKEM_N] < MLKEM_Q) })
#endif
{
  unsigned k;
  for (k = 0; k < MLKEM_K; k++)
    mlk_poly_reduce(&r->vec[k]);
}

/* ---- harnesses (DFCC entry) ---- */
void harness_enforce_poly(void)   { mlk_poly a;    mlk_poly_reduce(&a); }
void harness_replace(void)        { mlk_polyvec v; mlk_polyvec_reduce(&v); }

#ifdef HARNESS_DIRECT
int main(void)
{
  mlk_polyvec v;
  mlk_polyvec_reduce(&v);
  unsigned j;
  for (j = 0; j < MLKEM_K * MLKEM_N; j++)
    __CPROVER_assert(v.vec[j / MLKEM_N].coeffs[j % MLKEM_N] >= 0 &&
                     v.vec[j / MLKEM_N].coeffs[j % MLKEM_N] < MLKEM_Q,
                     "all coeffs reduced into [0,Q)");
  return 0;
}
#endif
