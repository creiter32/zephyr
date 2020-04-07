/*
 * Copyright (c) 2019-2020 Infineon Technologies AG
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IFX_OPTIGA_TRUST_M_H_
#define IFX_OPTIGA_TRUST_M_H_

#include <device.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include <drivers/crypto/optiga.h>

// TODO(chr): find the maximum APDU size value
#define OPTRUST_MAX_APDU_SIZE 1600

struct optrust_ctx {
	struct device *dev;
	u8_t *apdu_buf;
	size_t apdu_buf_len;
	struct optiga_apdu apdu;
};

/**
 * @brief Initialize a command context and bind it to a device
 *
 * @param ctx context to initialize
 * @param dev device to bind the context to
 * @param apdu_buf send and receive buffer for the APDU
 * @param apdu_buf_len length of apdu_buf
 */
int optrust_init(struct optrust_ctx *ctx, struct device *dev, u8_t *apdu_buf, size_t apdu_buf_len);

/**
 * @brief Deallocate all resources and unbind the command context
 * @param ctx context to free
 */
void optrust_deinit(struct optrust_ctx *ctx);

/**
 * @brief Request a session context for exclusive use
 * @param oid Returned OID of the assigned session context
 * @return 0 on success, error code otherwise
 */
int optrust_session_acquire(struct optrust_ctx *ctx, u16_t *oid);

/**
 * @brief Return a session context
 * @param oid OID of the session context to return
 * @return 0 on success, error code otherwise
 */
int optrust_session_release(struct optrust_ctx *ctx, u16_t oid);

/**
 * @brief Initialize the Trust M for shielded connection
 * @return 0 on success, error code otherwise
 */
int optrust_shielded_connection_init(struct optrust_ctx *ctx, const u8_t *psk, size_t psk_len);

/**
 * @brief Enable or disable shielded connection for following commands
 * @enable If true shielded connection is enabled, otherwise disabled
 * @return 0 on success, error code otherwise
 */
int optrust_shielded_connection_enable(struct optrust_ctx *ctx, bool enable);

/**
 * @brief Read data from a data object in the OPTIGA
 *
 * @param ctx Command context to use
 * @param oid Object ID to read from
 * @param offs Number of bytes to skip from the beginning of the data object
 * @param buf Output buffer for the read data
 * @param len Must be set to the length of buf and returns the number of data bytes read
 * @return 0 on success, error code otherwise
 */
int optrust_data_get(struct optrust_ctx *ctx, u16_t oid, size_t offs, u8_t *buf, size_t *len);

/**
 * @brief Write data to a data object in the OPTIGA
 *
 * @param ctx Command context to use
 * @param oid Object ID to write to
 * @param erase If true, erase data object before writing
 * @param offs Number of bytes to skip from the beginning of the data object
 * @param buf Data to write
 * @param len length of buf
 * @return 0 on success, error code otherwise
 */
int optrust_data_set(struct optrust_ctx *ctx, u16_t oid, bool erase, size_t offs, const u8_t *buf, size_t len);


#define OPTRUST_NIST_P256_PUB_KEY_LEN 64
#define OPTRUST_NIST_P384_PUB_KEY_LEN 96

enum OPTRUST_ALGORITHM {
    OPTRUST_ALGORITHM_NIST_P256 = 0x03,
    OPTRUST_ALGORITHM_NIST_P384 = 0x04,
    OPTRUST_ALGORITHM_SHA256	= 0xE2
};

enum OPTRUST_KEY_USAGE_FLAG {
    OPTRUST_KEY_USAGE_FLAG_AUTH	= 0x01,
    OPTRUST_KEY_USAGE_FLAG_ENC = 0x02,
    OPTRUST_KEY_USAGE_FLAG_HOST_FW_UPDATE = 0x04,
    OPTRUST_KEY_USAGE_FLAG_DEV_MGMT = 0x08,
    OPTRUST_KEY_USAGE_FLAG_SIGN = 0x10,
};

/**
 * @brief Generate an ECC key pair and export the public key
 *
 * @param ctx Command context to use
 * @param oid Object ID to store the private key
 * @param alg Type of key pair to generate
 * @param key_usage Combination of IFX_OPTIGA_TRUST_KEY_USAGE_FLAG, see Solution Reference Manual, Table 39 for their meaning
 * @param pub_key Output buffer for the public key
 * @param pub_key_len length of pub_key, contains the length of the public key
 * @return 0 on success, error code otherwise
 *
 * @note The size of the public key buffer must match the selected algorithm or be bigger.
 */
int optrust_ecc_gen_keys_oid(struct optrust_ctx *ctx, u16_t oid, enum OPTRUST_ALGORITHM alg,
                enum OPTRUST_KEY_USAGE_FLAG key_usage, u8_t *pub_key, size_t *pub_key_len);

#define OPTRUST_NIST_P256_SIGNATURE_LEN 64
#define OPTRUST_NIST_P384_SIGNATURE_LEN 96

/**
 * @brief Sign a digest using a private key in the OPTIGA
 *
 * @param ctx Command context to use
 * @param oid Object ID of the private key to use
 * @param digest Digest to sign
 * @param digest_len Length of digest
 * @param signature Output buffer for the signature
 * @param signature_len Length of signature buffer, contains length of signature afterwards.
 * @return 0 on success, error code otherwise
 */
int optrust_ecdsa_sign_oid(struct optrust_ctx *ctx, u16_t oid, const u8_t *digest, size_t digest_len, u8_t *signature, size_t signature_len);

/**
 * @brief Verify a signature using a public key in the OPTIGA
 *
 * @param ctx Command context to use
 * @param oid Object ID of the public key to use
 * @param digest Digest to verify the signature of
 * @param digest_len Length of digest
 * @param signature Signature to verify
 * @param signature_len Length of signature
 * @return 0 if the signature matches, error code otherwise
 */
int optrust_ecdsa_verify_oid(struct optrust_ctx *ctx, u16_t oid, const u8_t *digest, size_t digest_len, const u8_t *signature, size_t signature_len);

/*
 *  The following APIs are drafts for now
 */

/**
 * @brief Generate an ECDSA key pair and export private and public key
 *
 * @param ctx Command context to use
 * @param alg Type of key pair to generate
 * @param key_usage Combination of IFX_OPTIGA_TRUST_KEY_USAGE_FLAG, see Solution Reference Manual, Table 39 for their meaning
 * @param priv_key Output buffer for the private key
 * @param priv_key_len length of pub_key, contains the length of the private key
 * @param pub_key Output buffer for the public key
 * @param pub_key_len length of pub_key, contains the length of the public key
 * @return 0 on success, error code otherwise
 *
 * @note The size of the public and private key buffers must match the selected algorithm or be bigger.
 */
int optrust_ecc_gen_keys_ext(struct optrust_ctx *ctx,
                enum OPTRUST_ALGORITHM alg,
                enum OPTRUST_KEY_USAGE_FLAG key_usage,
				u8_t* priv_key, size_t * priv_key_len,
				u8_t *pub_key, size_t *pub_key_len);

/**
 * @brief Perform an ECDH operation on a shared secret to derive a key
 *
 * @param ctx Command context to use
 * @param shared_secret_oid OID of the shared secret to use for key derivation
 * @param deriv_data Shared secret derivation data
 * @param deriv_data_len Length of deriv_data
 * @param key Output buffer for the derived key
 * @param key_len Length of key
 * @return 0 on success, error code otherwise
 */
int optrust_ecdh_calc_ext(struct optrust_ctx *ctx, u16_t shared_secret_oid,
				const u8_t *deriv_data, size_t deriv_data_len,
				u8_t *key, size_t key_len);

/**
 * @brief Perform an ECDH operation on a shared secret to derive a key and store it in a session context
 *
 * @param ctx Command context to use
 * @param shared_secret_oid OID of the shared secret to use for key derivation
 * @param deriv_data Shared secret derivation data
 * @param deriv_data_len Length of deriv_data
 * @param key_len Length of key
 * @param key_oid OID to store the derived key
 * @return 0 on success, error code otherwise
 */
int optrust_ecdh_calc_oid(struct optrust_ctx *ctx, u16_t shared_secret_oid,
				const u8_t *deriv_data, size_t deriv_data_len,
				size_t key_len, u16_t key_oid);

/**
 * @brief Verify a signature using a public key provided by the host
 *
 * @param ctx Command context to use
 * @param alg Algorithm identifier of the public key
 * @param pub_key Output buffer for the public key
 * @param pub_key_len length of pub_key, contains the length of the public key
 * @param digest Digest to verify the signature of
 * @param digest_len Length of digest
 * @param signature Signature to verify
 * @param signature_len Length of signature
 * @return 0 if the signature matches, error code otherwise
 */
int optrust_ecdsa_verify_ext(struct optrust_ctx *ctx, enum OPTRUST_ALGORITHM alg,
				const u8_t *pub_key, size_t pub_key_len,
				const u8_t *digest, size_t digest_len,
				const u8_t *signature, size_t signature_len);

/**
 * @brief Read metadata from a data object
 * @param ctx Command context to use
 * @param oid OID of the data object
 * @param data Output buffer for the data object
 * @param data_len Length of the output buffer, contains length of metadata afterwards
 * @return 0 on success, error code otherwise
 */
int optrust_metadata_get(struct optrust_ctx *ctx, u16_t oid, u8_t *data, size_t *data_len);

/**
 * @brief Set metadata of a data object
 * @param ctx Command context to use
 * @param oid OID of the data object
 * @param data metadata to write
 * @param data_len length of data
 * @return 0 on success, error code otherwise
 */
int optrust_metadata_set(struct optrust_ctx *ctx, u16_t oid, const u8_t *data, size_t data_len);

#define OPTRUST_SHA256_DIGEST_LEN 32

/**
 * @brief Hash data passed by the host
 *
 * @param ctx Command context to use
 * @param data Data to hash
 * @param data_len Length of data
 * @param digest Output buffer for the computed digest
 * @param digest_len Length of digest, contains the length of the computed digest afterwards
 * @return 0 on success, error code otherwise
 */
int optrust_sha256_ext(struct optrust_ctx *ctx, const u8_t* data, size_t data_len,
                       u8_t *digest, size_t *digest_len);

/**
 * @brief Hash data from an OID
 *
 * @param ctx Command context to use
 * @param oid OID to read the data to has
 * @param offs Number of bytes to skip befor hashing data
 * @param len Number of bytes to hash
 * @param digest Computed digest
 * @param digest_len Length of digest, contains the length of the computed digest afterwards
 * @return 0 on success, error code otherwise
 */
int optrust_sha256_oid(struct optrust_ctx *ctx,
				u16_t oid, size_t offs, size_t len,
				u8_t *digest, size_t *digest_len);

enum OPTRUST_RNG_TYPE {
    OPTRUST_RNG_TYPE_TRNG	= 0x00,
    OPTRUST_RNG_TYPE_DRNG	= 0x01,
};

/**
 * @brief Generate random bytes
 * @param ctx Command context to use
 * @param type Type of the RNG to use
 * @param rnd Output buffer for the random bytes
 * @param rnd_len Size of the output buffer
 * @return 0 on success, error code otherwise
 */
int optrust_rng_gen_ext(struct optrust_ctx *ctx, enum OPTRUST_RNG_TYPE type, u8_t *rnd, size_t rnd_len);

#endif /* IFX_OPTIGA_TRUST_M_H_ */