#define CAP 64
/* Toy recursive descent over a nesting counter. The contract states the
   bound the code must own; the CAPPED build adds the guard that owns it. */
int descend(unsigned remaining, unsigned depth)
{
  __ESBMC_requires(depth <= CAP);
  __ESBMC_ensures(1);
  if (remaining == 0) return 0;
#ifdef CAPPED
  if (depth == CAP) return -1;
#endif
  return descend(remaining - 1, depth + 1);
}
int main(void) { return descend(7, 0) >= -1 ? 0 : 1; }
