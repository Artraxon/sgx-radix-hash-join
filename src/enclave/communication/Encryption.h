//
// Created by leonhard on 02.09.21.
//

#ifndef RADIX_HASH_JOIN_ENCRYPTION_H
#define RADIX_HASH_JOIN_ENCRYPTION_H
#define INITV_LEN 12
#define KEY_LEN 16


#include <cstdint>
#include <stdint.h>
#include <sgx_tcrypto.h>

namespace hpcjoin {
    namespace communication {

        class Encryption {
        protected:
            uint8_t initv[12];
            uint8_t key[16];
        public:
            Encryption(uint8_t initv[INITV_LEN], uint8_t key[KEY_LEN]);

            void encrypt(uint8_t* data, uint64_t len, uint8_t* out);
            void decrypt(uint8_t* encrypted, uint64_t len, uint8_t* decryptedOut);

            static uint64_t getEncryptedSize(uint64_t textLen);
            static uint64_t getDecryptedSize(uint64_t encryptedLen);
            static void setupEncryption(uint8_t initv[INITV_LEN], uint8_t key[KEY_LEN]);
        };

        extern Encryption* encryption;
    }
}


#endif //RADIX_HASH_JOIN_ENCRYPTION_H
