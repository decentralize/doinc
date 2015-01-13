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

typedef RSA::PublicKey PublicKey;
typedef RSA::PrivateKey PrivateKey;
typedef SecByteBlock Signature;

void SaveToFile(const std::string& filename, const BufferedTransformation& bt);
void SavePrivateKey(const std::string& filename, const PrivateKey& key);
void SavePublicKey(const std::string& filename, const PublicKey& key);
void LoadFromFile(const std::string& filename, BufferedTransformation& bt);
void LoadPrivateKeyFile(const std::string& filename, PrivateKey& key);
void LoadPublicKeyFile(const std::string& filename, PublicKey& key);

std::string StringifyPublicKey(PublicKey& key);
void LoadFromString(const std::string& source, BufferedTransformation& bt);
void LoadPrivateKey(const std::string& str, PrivateKey& key);
void LoadPublicKey(const std::string& str, PublicKey& key);

bool Validate(PrivateKey& key);
bool Validate(PublicKey& key);

void GenerateKeyPair(PrivateKey& priv_key, PublicKey& pub_key);

std::string HashedKey(PublicKey pub_key);

Signature Sign(PrivateKey& priv_key, std::string message);
bool Verify(PublicKey& pub_key, std::string message, Signature signature);

}

#endif
