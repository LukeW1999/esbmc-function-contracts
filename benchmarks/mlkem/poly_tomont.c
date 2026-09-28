/* mlkem-native mlk_poly_tomont, adapted for ESBMC contracts (companion to
 * poly_reduce.c). Maps each coefficient into the Montgomery domain via
 * montgomery_reduce(coeff * f), f = (2^16)^2 mod Q = 1353. Heavy per-coeff
 * arithmetic (multiply + reduce) like poly_reduce -> a second "modular wins"
 * kernel. Single pointer arg; assigns the whole poly; ensures |coeff| < Q. */
#include <stdint.h>
#include <stddef.h>
#ifndef MLKEM_K
#define MLKEM_K 3
#endif
#ifndef MLKEM_N
#define MLKEM_N 256
#endif
#define MLKEM_Q    3329
#define MLK_QINV  (-3327)   /* q^{-1} mod 2^16 */
#define MLK_TOMONT 1353     /* (2^16)^2 mod Q */

typedef struct { int16_t coeffs[MLKEM_N]; } mlk_poly;
typedef struct { mlk_poly vec[MLKEM_K];   } mlk_polyvec;

static int16_t mlk_montgomery_reduce(int32_t a)
{
  int16_t t = (int16_t)(a * MLK_QINV);
  return (int16_t)((a - (int32_t)t * MLKEM_Q) >> 16);
}

#ifdef CONTRACTS
#define REQUIRES(x)  __ESBMC_requires(x)
#define ENSURES(x)   __ESBMC_ensures(x)
#define ASSIGNS(...) __ESBMC_assigns(__VA_ARGS__)
#else
#define REQUIRES(x)
#define ENSURES(x)
#define ASSIGNS(...)
#endif

void mlk_poly_tomont(mlk_poly *r)
{
  int i;
  REQUIRES(r != NULL);
  ASSIGNS(r->coeffs);
  ENSURES(__ESBMC_forall(&i, !(i >= 0 && i < MLKEM_N) ||
            (r->coeffs[i] > -MLKEM_Q && r->coeffs[i] < MLKEM_Q)));
  unsigned li;
  for (li = 0; li < MLKEM_N; li++)
    r->coeffs[li] = mlk_montgomery_reduce((int32_t)r->coeffs[li] * MLK_TOMONT);
}

void mlk_polyvec_tomont(mlk_polyvec *r)
{
  int j;
  REQUIRES(r != NULL);
  ASSIGNS(r->vec);
  ENSURES(__ESBMC_forall(&j, !(j >= 0 && j < MLKEM_K * MLKEM_N) ||
            (r->vec[j/MLKEM_N].coeffs[j%MLKEM_N] > -MLKEM_Q &&
             r->vec[j/MLKEM_N].coeffs[j%MLKEM_N] < MLKEM_Q)));
  unsigned k;
  for (k = 0; k < MLKEM_K; k++)
    mlk_poly_tomont(&r->vec[k]);
}

#ifdef HARNESS_DIRECT
int main(void){
  mlk_polyvec v; mlk_polyvec_tomont(&v);
  unsigned j;
  __ESBMC_assert(__ESBMC_forall(&j, !(j >= 0 && j < MLKEM_K*MLKEM_N) ||
      (v.vec[j/MLKEM_N].coeffs[j%MLKEM_N] > -MLKEM_Q &&
       v.vec[j/MLKEM_N].coeffs[j%MLKEM_N] < MLKEM_Q)), "abs bound");
  return 0;
}
#endif
