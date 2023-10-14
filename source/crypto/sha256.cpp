#include "../../include/crypto/sha256.h"
#include <cstdint>
#include <cstring>

void sha256::transform(const unsigned char *message, uint32_t block_nb) {
    uint32_t w[64];
    uint32_t wv[8];

    uint32_t t1;
    uint32_t t2;

    const uint8_t *sub_block;

    for (int i = 0; i < (int)block_nb; i++) {
        sub_block = message + (i << 6);

        for (int j = 0; j < 16; j++)
            SHA2_PACK32(&sub_block[j << 2], &w[j]);

        for (int j = 16; j < 64; j++)
            w[j] = SHA256_F4(w[j - 2]) + w[j - 7] + SHA256_F3(w[j - 15]) + w[j - 16];

        for (int j = 0; j < 8; j++)
            wv[j] = m_h[j];

        for (int j = 0; j < 64; j++) {
            t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6])
                + sha::sha_K[j] + w[j];
            
            t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);

            wv[7] = wv[6];
            wv[6] = wv[5];
            wv[5] = wv[4];
            wv[4] = wv[3] + t1;
            wv[3] = wv[2];
            wv[2] = wv[1];
            wv[1] = wv[0];
            wv[0] = t1 + t2;
        }

        for (int j = 0; j < 8; j++)
            m_h[j] += wv[j];
    }
}

void sha256::update(const uint8_t *message, uint32_t len) {
    uint32_t block_nb;

    uint32_t new_len;
    uint32_t rem_len;
    uint32_t tmp_len;

    const uint8_t *shifted_message;

    tmp_len = SHA224_256_BLOCK_SIZE - m_len;
    rem_len = len < tmp_len ? len : tmp_len;

    memcpy(&m_block[m_len], message, rem_len);

    if (m_len + len < SHA224_256_BLOCK_SIZE) {
        m_len += len;

        return;
    }

    new_len  = len - rem_len;
    block_nb = new_len / SHA224_256_BLOCK_SIZE;

    shifted_message = message + rem_len;

    transform(m_block, 1);
    transform(shifted_message, block_nb);

    rem_len = new_len % SHA224_256_BLOCK_SIZE;

    memcpy(m_block, &shifted_message[block_nb << 6], rem_len);

    m_len = rem_len;
    m_tot_len += (block_nb + 1) << 6;
}

void sha256::final(unsigned char *digest) {
    uint32_t block_nb;
    uint32_t pm_len;
    uint32_t len_b;

    block_nb = (1 + ((SHA224_256_BLOCK_SIZE - 9)
                        < (m_len % SHA224_256_BLOCK_SIZE)));

    len_b = (m_tot_len + m_len) << 3;
    pm_len = block_nb << 6;

    memset(m_block + m_len, 0, pm_len - m_len);

    m_block[m_len] = 0x80;

    SHA2_UNPACK32(len_b, m_block + pm_len - 4);

    transform(m_block, block_nb);

    for (int i = 0; i < 8; i++)
        SHA2_UNPACK32(m_h[i], &digest[i << 2]);
}

std::string sha256::hash(const std::string& value, const sha256_options& options) {
    uint8_t digest[DIGEST_SIZE];

    memset(digest, 0, DIGEST_SIZE);

    sha256 ctx = sha256();

    ctx.update((uint8_t *)value.c_str(), value.length());
    ctx.final(digest);

    char buf[2 * DIGEST_SIZE + 1];

    buf[2 * DIGEST_SIZE] = 0;

    for (int i = 0; i < DIGEST_SIZE; i++)
        sprintf(buf + i * 2, "%02x", digest[i]);

    return std::string(buf);
}