/*
 * ANSSI Code with ENHANCED ESBMC Contracts (v2)
 * Compatible with ESBMC 8.0.0 (no __ESBMC_assigns)
 * 
 * Goal: Make compositional verification work by adding precise postconditions
 */

#include <stddef.h>

typedef unsigned char u8;
typedef unsigned int u32;

#define X509_FILE_LINE_NUM_ERR 1000
#define ERROR_TRACE_APPEND(x) do { } while(0)

typedef struct { int dummy; } cert_parsing_ctx;

#define CLASS_UNIVERSAL 0
#define ASN1_TYPE_SEQUENCE 16

/*
 * parse_id_len with ENHANCED contracts
 * 
 * Key postconditions:
 * 1. On success (return == 0): hdr_len == 2, hdr_len + data_len >= 2
 * 2. hdr_len + data_len <= len on success
 */
int parse_id_len(const u8 *buf, u32 len, int exp_class, u32 exp_type,
                 u32 *hdr_len, u32 *data_len)
{
    __ESBMC_requires(__ESBMC_is_fresh(buf, len));
    __ESBMC_requires(__ESBMC_is_fresh(hdr_len, sizeof(u32)));
    __ESBMC_requires(__ESBMC_is_fresh(data_len, sizeof(u32)));
    __ESBMC_requires(len > 0);
    
    __ESBMC_assigns(*hdr_len, *data_len);
    
    // ENHANCED POSTCONDITIONS - explicitly state what success means
    __ESBMC_ensures((__ESBMC_return_value != 0) || (*hdr_len == 2));
    __ESBMC_ensures((__ESBMC_return_value != 0) || (*data_len <= 127));
    __ESBMC_ensures((__ESBMC_return_value != 0) || (*hdr_len + *data_len <= len));
    __ESBMC_ensures((__ESBMC_return_value != 0) || (*hdr_len + *data_len >= 2));  // KEY guarantee
    __ESBMC_ensures((__ESBMC_return_value == 0) || (__ESBMC_return_value == -1));
    
    if (len < 2) {
        return -1;
    }
    
    if (buf[0] != 0x30) {
        return -1;
    }
    
    u8 len_byte = buf[1];
    
    if ((len_byte & 0x80) == 0) {
        *hdr_len = 2;
        *data_len = len_byte;
        
        if (*hdr_len + *data_len > len) {
            return -1;
        }
        
        return 0;
    }
    
    return -1;
}

/*
 * parse_x509_cert with contracts
 */
int parse_x509_cert(cert_parsing_ctx *ctx, const u8 *buf, u32 len)
{
    __ESBMC_requires(__ESBMC_is_fresh(ctx, sizeof(cert_parsing_ctx)));
    __ESBMC_requires(!(len > 0) || (buf != NULL));
    
    __ESBMC_assigns(ctx->dummy);
    __ESBMC_ensures(__ESBMC_return_value == 0 || __ESBMC_return_value == -1);
    
    if (buf == NULL || len == 0) {
        return -1;
    }
    
    ctx->dummy = 1;
    return 0;
}

/*
 * parse_x509_cert_relaxed with ENHANCED contracts
 */
int parse_x509_cert_relaxed(cert_parsing_ctx *ctx, const u8 *buf, u32 len, u32 *eaten)
{
    __ESBMC_requires(!((len > 0) && (buf != NULL)) || (len <= 128));
    __ESBMC_requires(__ESBMC_is_fresh(buf, len));
    __ESBMC_requires(__ESBMC_is_fresh(eaten, sizeof(u32)));
    __ESBMC_requires(len > 0);
    __ESBMC_requires(__ESBMC_is_fresh(ctx, sizeof(cert_parsing_ctx)));
    
    __ESBMC_assigns(*eaten, ctx->dummy);
    
    // ENHANCED POSTCONDITIONS matching ANSSI
    __ESBMC_ensures((__ESBMC_return_value != 0) || (*eaten > 0));
    __ESBMC_ensures((__ESBMC_return_value != 0) || (*eaten <= len));
    __ESBMC_ensures(__ESBMC_return_value <= 1);
    
    u32 hdr_len = 0, data_len = 0;
    int ret;

    if ((ctx == NULL) || (buf == NULL) || (len == 0) || (eaten == NULL)) {
        ret = -X509_FILE_LINE_NUM_ERR;
        ERROR_TRACE_APPEND(X509_FILE_LINE_NUM_ERR);
        goto out;
    }

    ret = parse_id_len(buf, len, CLASS_UNIVERSAL, ASN1_TYPE_SEQUENCE,
                       &hdr_len, &data_len);
    if (ret) {
        ret = 1;
        ERROR_TRACE_APPEND(X509_FILE_LINE_NUM_ERR);
        goto out;
    }

    *eaten = hdr_len + data_len;

    ret = parse_x509_cert(ctx, buf, hdr_len + data_len);
    if (ret) {
        ERROR_TRACE_APPEND(X509_FILE_LINE_NUM_ERR);
        goto out;
    }

    ret = 0;

out:
    return ret;
}

int main(void)
{
    cert_parsing_ctx ctx;
    u8 buffer[128];
    u32 len;
    u32 eaten;
    
    __ESBMC_assume(len > 0 && len <= 128);
    
    int ret = parse_x509_cert_relaxed(&ctx, buffer, len, &eaten);
    
    // ANSSI contract verification
    if (ret == 0) {
        __ESBMC_assert(eaten > 0, "ANSSI ensures: eaten > 0");
        __ESBMC_assert(eaten <= len, "ANSSI ensures: eaten <= len");
    }
    
    __ESBMC_assert(ret <= 1, "ANSSI ensures: result <= 1");
    
    return 0;
}
