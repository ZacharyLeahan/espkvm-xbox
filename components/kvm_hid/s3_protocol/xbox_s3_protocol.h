/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "psa/crypto.h"

#define XBOX_S3_PORT 3334
#define XBOX_P4_PORT 3335
#define XBOX_S3_HELLO 1
#define XBOX_S3_STATE 2
/* Both ESP targets are little endian. Authenticate the entire fixed wire image,
 * including the receiver's boot nonce so old-boot input cannot be replayed. */
typedef struct {
    uint8_t magic[4], kind, flags, reserved[2];
    uint32_t sequence, session, receiver;
    uint8_t mac[6], padding[2], report[20], tag[32];
} xbox_s3_packet_t;
_Static_assert(sizeof(xbox_s3_packet_t)==80,"wire packet size");

static inline psa_status_t xbox_s3_key(const char *hex, psa_key_id_t *key)
{
    if(strlen(hex)!=64) return PSA_ERROR_INVALID_ARGUMENT;
    uint8_t bytes[32];
    for(unsigned i=0;i<32;i++) {
        unsigned value=0;
        for(unsigned j=0;j<2;j++) {
            char c=hex[2*i+j];
            int digit=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:-1;
            if(digit<0) return PSA_ERROR_INVALID_ARGUMENT;
            value=(value<<4)|(unsigned)digit;
        }
        bytes[i]=value;
    }
    psa_key_attributes_t a=PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_type(&a,PSA_KEY_TYPE_HMAC);
    psa_set_key_bits(&a,256);
    psa_set_key_usage_flags(&a,PSA_KEY_USAGE_SIGN_MESSAGE|PSA_KEY_USAGE_VERIFY_MESSAGE);
    psa_set_key_algorithm(&a,PSA_ALG_HMAC(PSA_ALG_SHA_256));
    psa_status_t result=psa_import_key(&a,bytes,sizeof(bytes),key);
    memset(bytes,0,sizeof(bytes));
    return result;
}
static inline bool xbox_s3_sign(psa_key_id_t key,xbox_s3_packet_t *p)
{
    memcpy(p->magic,"X3P1",4);
    size_t length=0;
    return psa_mac_compute(key,PSA_ALG_HMAC(PSA_ALG_SHA_256),(const uint8_t *)p,
        offsetof(xbox_s3_packet_t,tag),p->tag,sizeof(p->tag),&length)==PSA_SUCCESS && length==32;
}
static inline bool xbox_s3_verify(psa_key_id_t key,const xbox_s3_packet_t *p)
{
    return !memcmp(p->magic,"X3P1",4) &&
        psa_mac_verify(key,PSA_ALG_HMAC(PSA_ALG_SHA_256),(const uint8_t *)p,
        offsetof(xbox_s3_packet_t,tag),p->tag,sizeof(p->tag))==PSA_SUCCESS;
}
static inline bool xbox_s3_newer(uint32_t seq,uint32_t previous)
{ return (int32_t)(seq-previous)>0; }
