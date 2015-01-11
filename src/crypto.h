#ifndef __CRYPTO_H_
#define __CRYPTO_H_

#include <string>

#include <rsa.h>
using CryptoPP::RSA;

#include <files.h>
using CryptoPP::FileSink;
using CryptoPP::FileSource;

#include <cryptlib.h>
using CryptoPP::BufferedTransformation;
using CryptoPP::ByteQueue;
using CryptoPP::StringSink;
using CryptoPP::StringSource;

#include <base64.h>
using CryptoPP::Base64Encoder;

#include <secblock.h>
using CryptoPP::SecByteBlock;

#include <osrng.h>
using CryptoPP::AutoSeededRandomPool;

#include <sha.h>
using CryptoPP::SHA;
using CryptoPP::SHA1;

#include <pssr.h>
using CryptoPP::RSASS;
using CryptoPP::PSS;

namespace crypto {

void SaveToFile(const std::string& filename, const BufferedTransformation& bt);
void SavePrivateKey(const std::string& filename, const RSA::PrivateKey& key);
void SavePublicKey(const std::string& filename, const RSA::PublicKey& key);
void LoadFromFile(const std::string& filename, BufferedTransformation& bt);
void LoadPrivateKeyFile(const std::string& filename, RSA::PrivateKey& key);
void LoadPublicKeyFile(const std::string& filename, RSA::PublicKey& key);
//void LoadFromString(const std::string& source, BufferedTransformation& bt);
//void LoadPrivateKey(const std::string& b64encoded, RSA::PrivateKey& key);
//void LoadPublicKey(const std::string& b64encoded, RSA::PublicKey& key);
bool Validate(RSA::PrivateKey& key);
bool Validate(RSA::PublicKey& key);
void GenerateKeyPair(RSA::PrivateKey& priv_key, RSA::PublicKey& pub_key);
std::string HashedKey(RSA::PublicKey pub_key);
SecByteBlock Sign(RSA::PrivateKey& priv_key, std::string message);
bool Verify(RSA::PublicKey& pub_key, std::string message, SecByteBlock signature);

}

#endif
