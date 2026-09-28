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
#define MAX_UINT32 (0xffffffffUL)
#define ASN1_MAX_BUFFER_SIZE (MAX_UINT32)

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

int parse_algoid_params_none(const u8 *cert, u32 off, u32 len)
{
	__ESBMC_requires(cert != NULL);
	__ESBMC_ensures(__ESBMC_return_value == 0 || __ESBMC_return_value == -1 || __ESBMC_return_value < 0);

	const u8 *buf = cert + off;
	u32 parsed = 0;
	int ret;

	if (cert == NULL) {
		ret = -X509_FILE_LINE_NUM_ERR;
		goto out;
	}

	switch (len) {
	case 0:
		ret = 0;
		break;
	case 2:
		ret = parse_null(buf, len, &parsed);
		break;
	default:
		ret = -1;
		break;
	}

out:
	return ret;
}


int main() {
	u8 buf[2] = { 0x05, 0x00 };
	u32 len = 2;
	u32 off = 0;

	int ret = parse_algoid_params_none(buf, off, len);
	assert(ret == 0);

	return ret;
}
