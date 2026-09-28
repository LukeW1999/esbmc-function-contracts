/* mlkem-native mlk_poly_add, adapted for ESBMC contracts (companion to
 * poly_reduce.c). Two-input additive kernel: r[i] += b[i] over 256 coeffs.
 * Inputs bounded to [-BND,BND] so the int16 sum cannot overflow; the contract
 * exercises a forall precondition + assigns(whole poly) + forall postcondition.
 * Direct harness calls poly_add at K independent sites (contracts compiled out).
 */
#include <stdint.h>
#include <stddef.h>
#ifndef MLKEM_K
#define MLKEM_K 3
#endif
#ifndef MLKEM_N
#define MLKEM_N 256
#endif
#define BND 100

typedef struct { int16_t coeffs[MLKEM_N]; } mlk_poly;

#ifdef CONTRACTS
#define REQUIRES(x)  __ESBMC_requires(x)
#define ENSURES(x)   __ESBMC_ensures(x)
#define ASSIGNS(...) __ESBMC_assigns(__VA_ARGS__)
#else
#define REQUIRES(x)
#define ENSURES(x)
#define ASSIGNS(...)
#endif

void mlk_poly_add(mlk_poly *r, const mlk_poly *b)
{
  int i;
  REQUIRES(r != NULL && b != NULL);
  REQUIRES(__ESBMC_forall(&i, !(i >= 0 && i < MLKEM_N) ||
            (r->coeffs[i] >= -BND && r->coeffs[i] <= BND &&
             b->coeffs[i] >= -BND && b->coeffs[i] <= BND)));
  ASSIGNS(r->coeffs);
  ENSURES(__ESBMC_forall(&i, !(i >= 0 && i < MLKEM_N) ||
            (r->coeffs[i] >= -2*BND && r->coeffs[i] <= 2*BND)));
  unsigned li;
  for (li = 0; li < MLKEM_N; li++)
    r->coeffs[li] = (int16_t)(r->coeffs[li] + b->coeffs[li]);
}

#ifdef HARNESS
int main(void)
{
  mlk_poly r[MLKEM_K], b[MLKEM_K];
  int j;
  /* bound all inputs so no overflow and the postcondition holds */
  for (int k = 0; k < MLKEM_K; k++)
    __ESBMC_assume(__ESBMC_forall(&j, !(j >= 0 && j < MLKEM_N) ||
        (r[k].coeffs[j] >= -BND && r[k].coeffs[j] <= BND &&
         b[k].coeffs[j] >= -BND && b[k].coeffs[j] <= BND)));
  for (int k = 0; k < MLKEM_K; k++) {
    mlk_poly_add(&r[k], &b[k]);
    __ESBMC_assert(__ESBMC_forall(&j, !(j >= 0 && j < MLKEM_N) ||
        (r[k].coeffs[j] >= -2*BND && r[k].coeffs[j] <= 2*BND)),
        "sum bounded");
  }
  return 0;
}
#endif
