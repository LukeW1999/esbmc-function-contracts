// from x509-parser (contracted version)
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

u64 time_components_to_comparable_u64(u16 na_year, u8 na_month, u8 na_day,
                                      u8 na_hour, u8 na_min, u8 na_sec)
{
	/* Valid calendar ranges ensure result fits in 55 bits:
	 * sec (0-59) * 1        < 2^6
	 * min (0-59) * 2^8      < 2^14
	 * hour(0-23) * 2^16     < 2^21
	 * day (0-31) * 2^24     < 2^29
	 * mon (0-12) * 2^32     < 2^36
	 * year(0-9999)* 2^40   < 2^54
	 * total                 < 2^55
	 */
	__ESBMC_requires(na_year <= 9999);
	__ESBMC_requires(na_month <= 12);
	__ESBMC_requires(na_day <= 31);
	__ESBMC_requires(na_hour <= 23);
	__ESBMC_requires(na_min <= 59);
	__ESBMC_requires(na_sec <= 59);
	__ESBMC_ensures(__ESBMC_return_value < (1ULL << 55));

	u64 res, tmp;

	res = ((u64)na_sec);
	tmp = ((u64)na_min) * (1ULL << 8);
	res += tmp;
	tmp = (((u64)na_hour) * (1ULL << 16));
	res += tmp;
	tmp = ((u64)na_day) * (1ULL << 24);
	res += tmp;
	tmp = ((u64)na_month) * (1ULL << 32);
	res += tmp;
	tmp = ((u64)na_year) * (1ULL << 40);
	res += tmp;

	return res;
}


int main(void)
{
  u16 na_year = 2023;
  u8 na_month = 12;
  u8 na_day = 20;
  u8 na_hour = 12;
  u8 na_min = 0;
  u8 na_sec = 0;
  u64 res;

  res = time_components_to_comparable_u64(na_year, na_month, na_day, na_hour, na_min, na_sec);
  assert(res < (1ULL << 55));
  return 0;
}
