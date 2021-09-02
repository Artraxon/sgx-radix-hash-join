//
// Created by leonhard on 02.09.21.
//

#include "Encryption.h"
#include <algorithm>

namespace hpcjoin {
    namespace communication {

        Encryption::Encryption(uint8_t initv[INITV_LEN], uint8_t key[KEY_LEN]){
            std::copy_n(initv, INITV_LEN, this->initv);
            std::copy_n(key, KEY_LEN, this->key);
        }

        void Encryption::encrypt(uint8_t* data, uint64_t len, uint8_t* out){
                sgx_rijndael128GCM_encrypt(reinterpret_cast<sgx_aes_gcm_128bit_key_t const *>(key),
                                           data, len, out + 16, this->initv, INITV_LEN, nullptr, 0,
                                           reinterpret_cast<sgx_aes_gcm_128bit_tag_t *>(out));
        }
        void Encryption::decrypt(uint8_t* encrypted, uint64_t len, uint8_t* decryptedOut){
                sgx_rijndael128GCM_decrypt(reinterpret_cast<sgx_aes_gcm_128bit_key_t const *>(key), encrypted + 16, len, decryptedOut, this->initv, INITV_LEN, nullptr, 0,
                                       reinterpret_cast<sgx_aes_gcm_128bit_tag_t const *>(encrypted));
        }

        uint64_t Encryption::getEncryptedSize(uint64_t textLen){
            return textLen + 16;
        }

        uint64_t Encryption::getDecryptedSize(uint64_t encryptedLen){
            return encryptedLen - 16;
        }

        void Encryption::setupEncryption(uint8_t initv[INITV_LEN], uint8_t key[KEY_LEN]){
            if(encryption == nullptr){
                encryption = new Encryption(initv, key);
            }
        }

        Encryption* encryption = nullptr;

    }
}