// from x509-parser (contracted version)
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define X509_FILE_NUM 0
#define X509_FILE_LINE_NUM_ERR ((X509_FILE_NUM * 100000) + __LINE__)

int bufs_differ(const u8 *b1, const u8 *b2, u32 n)
{
	__ESBMC_requires(b1 != NULL);
	__ESBMC_requires(b2 != NULL);
	__ESBMC_ensures(__ESBMC_return_value == 0 || __ESBMC_return_value == 1);

	int ret = 0;
	u32 i = 0;

	for (i = 0; i < n; i++) {
		if(b1[i] != b2[i]) {
			ret = 1;
			break;
		}
	}

	return ret;
}

static const u8 null_encoded_val[] = { 0x05, 0x00 };

int parse_null(const u8 *buf, u32 len, u32 *parsed)
{
	__ESBMC_requires(buf != NULL);
	__ESBMC_requires(parsed != NULL);
	__ESBMC_requires(len > 0);
	__ESBMC_ensures(__ESBMC_return_value == 0 || __ESBMC_return_value < 0);
	__ESBMC_ensures(__ESBMC_return_value != 0 || *parsed == 2);

	int ret;

	if ((len == 0) || (buf == NULL) || (parsed == NULL)) {
		ret = -X509_FILE_LINE_NUM_ERR;
		goto out;
	}

	if (len != sizeof(null_encoded_val)) {
		ret = -X509_FILE_LINE_NUM_ERR;
		goto out;
	}

	ret = bufs_differ(buf, null_encoded_val, sizeof(null_encoded_val));
	if (ret) {
		ret = -X509_FILE_LINE_NUM_ERR;
		goto out;
	}

	ret = 0;
	*parsed = sizeof(null_encoded_val);

out:
	return ret;
}

int main() {
  const u8 b[] = { 0x05, 0x00 };
  u32 parsed;

  int ret = parse_null(b, 2, &parsed);
  assert(parsed == 2);
  assert(ret == 0);

  return 0;
}
