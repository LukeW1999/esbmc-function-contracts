// from x509-parser (contracted version)
// Original ACSL used \forall/\exists quantifiers. The \forall property
// (ret==0 => all bytes <= 0x7f) is encoded via a nondet index checked by BMC.
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define X509_FILE_NUM 1
#define X509_FILE_LINE_NUM_ERR ((X509_FILE_NUM * 100000))

int check_ia5_string(const u8 *buf, u32 len)
{
	__ESBMC_requires(buf != NULL);
	__ESBMC_requires(len > 0);
	__ESBMC_ensures(__ESBMC_return_value == 0 || __ESBMC_return_value < 0);

	int ret;
	u32 i;

	if ((buf == NULL) || (len == 0)) {
		ret = -X509_FILE_LINE_NUM_ERR;
		goto out;
	}

	for (i = 0; i < len; i++) {
		if (buf[i] > 0x7f) {
			ret = -X509_FILE_LINE_NUM_ERR;
			goto out;
		}
	}

	ret = 0;

out:
	return ret;
}


int main() {
	u8 buf[5];
	u32 len = 5;

	int ret = check_ia5_string(buf, len);
	assert(ret == 0 || ret < 0);

	/* \forall i < len: ret==0 ==> buf[i] <= 0x7f
	 * Encoded via nondet index: BMC checks all possible idx values. */
	u32 idx;
	__ESBMC_assume(idx < len);
	if (ret == 0) assert(buf[idx] <= 0x7f);

	return 0;
}
