/*
 * ANSSI X.509 Parser - CBMC Contract Version
 * Translated from ESBMC contracts to CBMC contracts
 *
 * Goal: Test if CBMC can correctly propagate contracts through the call chain
 *       (i.e., whether replacing ALL functions with contracts works in CBMC)
 *
 * Call hierarchy:
 *   main()
 *     └─ parse_x509_cert_relaxed()    ← parent function (verification target)
 *          ├─ parse_id_len()           ← leaf function (parse TLV header)
 *          └─ parse_x509_cert()        ← leaf function (parse cert body)
 */

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>

typedef unsigned char u8;
typedef unsigned int u32;

#define X509_FILE_LINE_NUM_ERR 1000
#define ERROR_TRACE_APPEND(x) do { } while(0)

typedef struct { int dummy; } cert_parsing_ctx;

#define CLASS_UNIVERSAL 0
#define ASN1_TYPE_SEQUENCE 16

/*
 * parse_id_len with ENHANCED contracts (CBMC syntax)
 *
 * Key postconditions:
 * 1. On success (return == 0): hdr_len == 2, hdr_len + data_len >= 2
 * 2. hdr_len + data_len <= len on success
 */
int parse_id_len(const u8 *buf, u32 len, int exp_class, u32 exp_type,
                 u32 *hdr_len, u32 *data_len)
__CPROVER_requires(len > 0)
__CPROVER_requires(__CPROVER_is_fresh(buf, len))
__CPROVER_requires(hdr_len != ((void*)0))
__CPROVER_requires(data_len != ((void*)0))
__CPROVER_assigns(*hdr_len, *data_len)
/* ENHANCED POSTCONDITIONS - explicitly state what success means */
__CPROVER_ensures(__CPROVER_return_value != 0 || *hdr_len == 2)
__CPROVER_ensures(__CPROVER_return_value != 0 || *data_len <= 127)
__CPROVER_ensures(__CPROVER_return_value != 0 || *hdr_len + *data_len <= len)
__CPROVER_ensures(__CPROVER_return_value != 0 || *hdr_len + *data_len >= 2)
__CPROVER_ensures(__CPROVER_return_value == 0 || __CPROVER_return_value == -1)
{
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
 * parse_x509_cert with contracts (CBMC syntax)
 */
int parse_x509_cert(cert_parsing_ctx *ctx, const u8 *buf, u32 len)
__CPROVER_requires(ctx != ((void*)0))
__CPROVER_requires(len == 0 || buf != ((void*)0))
__CPROVER_assigns(ctx->dummy)
__CPROVER_ensures(__CPROVER_return_value == 0 || __CPROVER_return_value == -1)
{
    if (buf == ((void*)0) || len == 0) {
        return -1;
    }

    ctx->dummy = 1;
    return 0;
}

/*
 * parse_x509_cert_relaxed with ENHANCED contracts (CBMC syntax)
 *
 * This is the PARENT function that calls the leaf functions.
 * Key question: Can CBMC verify this when leaf functions are replaced by contracts?
 */
int parse_x509_cert_relaxed(cert_parsing_ctx *ctx, const u8 *buf, u32 len, u32 *eaten)
__CPROVER_requires(buf == ((void*)0) || len <= 128)
__CPROVER_requires(buf == ((void*)0) || __CPROVER_is_fresh(buf, len))
__CPROVER_requires(eaten != ((void*)0))
__CPROVER_requires(ctx != ((void*)0))
__CPROVER_assigns(*eaten, ctx->dummy)
/* ENHANCED POSTCONDITIONS matching ANSSI */
__CPROVER_ensures(__CPROVER_return_value != 0 || *eaten > 0)
__CPROVER_ensures(__CPROVER_return_value != 0 || *eaten <= len)
__CPROVER_ensures(__CPROVER_return_value <= 1)
{
    u32 hdr_len = 0, data_len = 0;
    int ret;

    if ((ctx == ((void*)0)) || (buf == ((void*)0)) || (len == 0) || (eaten == ((void*)0))) {
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

/* Isolation harnesses for the enforce scenarios (D1-D3).
 *
 * CBMC's --dfcc takes the name of a proof harness to use as the entry point,
 * and rejects the function under contract as its own harness
 * ("pair.first != harness_id"). ESBMC's --function generates an equivalent
 * harness internally, so these exist to put both tools in the same mode:
 * the function under test called once on nondeterministic arguments, rather
 * than reached through main. Without them the CBMC side would be verifying
 * the whole program with one function wrapped, which is a different
 * experiment.
 *
 * They are unused as of CBMC 6.8.0, which segfaults in goto-instrument
 * whenever --dfcc names a harness other than main. Reduced to seven lines:
 *
 *   int inc(int x)
 *   __CPROVER_requires(x >= 0 && x < 100)
 *   __CPROVER_ensures(__CPROVER_return_value == x + 1)
 *   { return x + 1; }
 *   void harness(void) { int a; __CPROVER_assume(a >= 0 && a < 100); (void)inc(a); }
 *   int main(void) { return inc(1) - 2; }
 *
 *   goto-instrument --dfcc harness --enforce-contract inc m.goto o.goto
 *     -> Segmentation fault (core dumped)
 *   goto-instrument --dfcc main    --enforce-contract inc m.goto o.goto
 *     -> VERIFICATION SUCCESSFUL
 *
 * So the enforce rows are measured with --dfcc main, which wraps the
 * function inside the whole program rather than in isolation. That is not
 * the same experiment as ESBMC's --function, and the table says so. Restore
 * these once the crash is fixed.
 */
void harness_parse_id_len(void)
{
    u8 buf[128];
    u32 len, hdr_len, data_len;
    int exp_class;
    u32 exp_type;
    __CPROVER_assume(len > 0 && len <= 128);
    (void)parse_id_len(buf, len, exp_class, exp_type, &hdr_len, &data_len);
}

void harness_parse_x509_cert(void)
{
    cert_parsing_ctx ctx;
    u8 buf[128];
    u32 len;
    __CPROVER_assume(len <= 128);
    (void)parse_x509_cert(&ctx, buf, len);
}

void harness_parse_x509_cert_relaxed(void)
{
    cert_parsing_ctx ctx;
    u8 buf[128];
    u32 len, eaten;
    __CPROVER_assume(len > 0 && len <= 128);
    (void)parse_x509_cert_relaxed(&ctx, buf, len, &eaten);
}

int main(void)
{
    cert_parsing_ctx ctx;
    u8 buffer[128];
    u32 len;
    u32 eaten;

    __CPROVER_assume(len > 0 && len <= 128);

    int ret = parse_x509_cert_relaxed(&ctx, buffer, len, &eaten);

    /* ANSSI contract verification */
    if (ret == 0) {
        assert(eaten > 0);    /* ANSSI ensures: eaten > 0 */
        assert(eaten <= len);  /* ANSSI ensures: eaten <= len */
    }

    assert(ret <= 1);          /* ANSSI ensures: result <= 1 */

    return 0;
}
