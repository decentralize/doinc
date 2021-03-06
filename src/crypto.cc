#include "crypto.h"

#include <boost/algorithm/string.hpp>
using boost::algorithm::trim;

namespace crypto {

void SaveToFile(const std::string& filename, const BufferedTransformation& bt) {
  FileSink file(filename.c_str());

  bt.CopyTo(file);
  file.MessageEnd();
}

void SavePrivateKey(const std::string& filename, const PrivateKey& key) {
  ByteQueue queue;
  key.Save(queue);

  SaveToFile(filename, queue);
}

void SavePublicKey(const std::string& filename, const PublicKey& key) {
  ByteQueue queue;
  key.Save(queue);

  SaveToFile(filename, queue);
}

void LoadFromFile(const std::string& filename, BufferedTransformation& bt) {
  FileSource file(filename.c_str(), true);

  file.TransferTo(bt);
  bt.MessageEnd();
}

void LoadPrivateKeyFile(const std::string& filename, PrivateKey& key) {
  ByteQueue queue;

  LoadFromFile(filename, queue);
  key.Load(queue);
}

void LoadPublicKeyFile(const std::string& filename, PublicKey& key) {
  ByteQueue queue;

  LoadFromFile(filename, queue);
  key.Load(queue);
}

std::string StringifyPublicKey(PublicKey& key) {
  std::string str;
  StringSink sink(str);
  key.Save(sink);
  return str;
}

void LoadFromString(const std::string& source, BufferedTransformation& bt) {
  StringSource strs(source, true, NULL);

  strs.TransferTo(bt);
  bt.MessageEnd();
}

void LoadPrivateKey(const std::string& source, PrivateKey& key) {
  ByteQueue queue;

  LoadFromString(source, queue);
  key.Load(queue);
}

void LoadPublicKey(const std::string& source, PublicKey& key) {
  ByteQueue queue;

  LoadFromString(source, queue);
  key.Load(queue);
}

bool Validate(PrivateKey& key) {
  AutoSeededRandomPool rnd_pool;
  return key.Validate(rnd_pool, 3);
}

bool Validate(PublicKey& key) {
  AutoSeededRandomPool rnd_pool;
  return key.Validate(rnd_pool, 3);
}

void GenerateKeyPair(RSA::PrivateKey& priv_key, RSA::PublicKey& pub_key) {
  AutoSeededRandomPool rnd_pool;
  priv_key.GenerateRandomWithKeySize(rnd_pool, 3072);
  pub_key.AssignFrom(priv_key);
}

std::string HashedKey(RSA::PublicKey pub_key) {
  // Hash pubkey for distribution
  std::string toHash;
  StringSink sink(toHash);
  pub_key.Save(sink);
  byte thumbprint[SHA::DIGESTSIZE];
  SHA().CalculateDigest(thumbprint, (const byte*) toHash.data(), toHash.length());

  // Create string version of the thumbprint
  std::string thumbprint_string;
  StringSource source(thumbprint, SHA::DIGESTSIZE, true,
      new Base64Encoder(new StringSink(thumbprint_string)));

  trim(thumbprint_string);

  return thumbprint_string;
}


Signature Sign(RSA::PrivateKey& priv_key, std::string message) {
  AutoSeededRandomPool rnd_pool;
  RSASS<PSS, SHA1>::Signer signer(priv_key);

  size_t sig_length = signer.MaxSignatureLength();
  Signature signature(sig_length);

  sig_length = signer.SignMessage(
      rnd_pool,
      (const byte*) message.data(),
      message.length(),
      signature);
  signature.resize(sig_length);

  return signature;
}

bool Verify(RSA::PublicKey& pub_key, std::string message, Signature signature) {
  RSASS<PSS, SHA1>::Verifier verifier(pub_key);

  return verifier.VerifyMessage(
      (const byte*)message.data(),
      message.length(),
      signature,
      signature.size());
}

}
