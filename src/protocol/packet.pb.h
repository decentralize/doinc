// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: packet.proto

#ifndef PROTOBUF_packet_2eproto__INCLUDED
#define PROTOBUF_packet_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_packet_2eproto();
void protobuf_AssignDesc_packet_2eproto();
void protobuf_ShutdownFile_packet_2eproto();

class Packet;

// ===================================================================

class Packet : public ::google::protobuf::Message {
 public:
  Packet();
  virtual ~Packet();

  Packet(const Packet& from);

  inline Packet& operator=(const Packet& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Packet& default_instance();

  void Swap(Packet* other);

  // implements Message ----------------------------------------------

  Packet* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Packet& from);
  void MergeFrom(const Packet& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required int32 fn = 1;
  inline bool has_fn() const;
  inline void clear_fn();
  static const int kFnFieldNumber = 1;
  inline ::google::protobuf::int32 fn() const;
  inline void set_fn(::google::protobuf::int32 value);

  // required bytes data = 2;
  inline bool has_data() const;
  inline void clear_data();
  static const int kDataFieldNumber = 2;
  inline const ::std::string& data() const;
  inline void set_data(const ::std::string& value);
  inline void set_data(const char* value);
  inline void set_data(const void* value, size_t size);
  inline ::std::string* mutable_data();
  inline ::std::string* release_data();
  inline void set_allocated_data(::std::string* data);

  // required int64 crc = 3;
  inline bool has_crc() const;
  inline void clear_crc();
  static const int kCrcFieldNumber = 3;
  inline ::google::protobuf::int64 crc() const;
  inline void set_crc(::google::protobuf::int64 value);

  // @@protoc_insertion_point(class_scope:Packet)
 private:
  inline void set_has_fn();
  inline void clear_has_fn();
  inline void set_has_data();
  inline void clear_has_data();
  inline void set_has_crc();
  inline void clear_has_crc();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::std::string* data_;
  ::google::protobuf::int64 crc_;
  ::google::protobuf::int32 fn_;
  friend void  protobuf_AddDesc_packet_2eproto();
  friend void protobuf_AssignDesc_packet_2eproto();
  friend void protobuf_ShutdownFile_packet_2eproto();

  void InitAsDefaultInstance();
  static Packet* default_instance_;
};
// ===================================================================


// ===================================================================

// Packet

// required int32 fn = 1;
inline bool Packet::has_fn() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Packet::set_has_fn() {
  _has_bits_[0] |= 0x00000001u;
}
inline void Packet::clear_has_fn() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void Packet::clear_fn() {
  fn_ = 0;
  clear_has_fn();
}
inline ::google::protobuf::int32 Packet::fn() const {
  // @@protoc_insertion_point(field_get:Packet.fn)
  return fn_;
}
inline void Packet::set_fn(::google::protobuf::int32 value) {
  set_has_fn();
  fn_ = value;
  // @@protoc_insertion_point(field_set:Packet.fn)
}

// required bytes data = 2;
inline bool Packet::has_data() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void Packet::set_has_data() {
  _has_bits_[0] |= 0x00000002u;
}
inline void Packet::clear_has_data() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void Packet::clear_data() {
  if (data_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    data_->clear();
  }
  clear_has_data();
}
inline const ::std::string& Packet::data() const {
  // @@protoc_insertion_point(field_get:Packet.data)
  return *data_;
}
inline void Packet::set_data(const ::std::string& value) {
  set_has_data();
  if (data_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    data_ = new ::std::string;
  }
  data_->assign(value);
  // @@protoc_insertion_point(field_set:Packet.data)
}
inline void Packet::set_data(const char* value) {
  set_has_data();
  if (data_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    data_ = new ::std::string;
  }
  data_->assign(value);
  // @@protoc_insertion_point(field_set_char:Packet.data)
}
inline void Packet::set_data(const void* value, size_t size) {
  set_has_data();
  if (data_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    data_ = new ::std::string;
  }
  data_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:Packet.data)
}
inline ::std::string* Packet::mutable_data() {
  set_has_data();
  if (data_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    data_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:Packet.data)
  return data_;
}
inline ::std::string* Packet::release_data() {
  clear_has_data();
  if (data_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = data_;
    data_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void Packet::set_allocated_data(::std::string* data) {
  if (data_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete data_;
  }
  if (data) {
    set_has_data();
    data_ = data;
  } else {
    clear_has_data();
    data_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:Packet.data)
}

// required int64 crc = 3;
inline bool Packet::has_crc() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void Packet::set_has_crc() {
  _has_bits_[0] |= 0x00000004u;
}
inline void Packet::clear_has_crc() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void Packet::clear_crc() {
  crc_ = GOOGLE_LONGLONG(0);
  clear_has_crc();
}
inline ::google::protobuf::int64 Packet::crc() const {
  // @@protoc_insertion_point(field_get:Packet.crc)
  return crc_;
}
inline void Packet::set_crc(::google::protobuf::int64 value) {
  set_has_crc();
  crc_ = value;
  // @@protoc_insertion_point(field_set:Packet.crc)
}


// @@protoc_insertion_point(namespace_scope)

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_packet_2eproto__INCLUDED