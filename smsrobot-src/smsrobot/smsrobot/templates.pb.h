// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: templates.proto

#ifndef PROTOBUF_templates_2eproto__INCLUDED
#define PROTOBUF_templates_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3003000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3003000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
namespace pb {
class ttemplate;
class ttemplateDefaultTypeInternal;
extern ttemplateDefaultTypeInternal _ttemplate_default_instance_;
class ttemplate_param;
class ttemplate_paramDefaultTypeInternal;
extern ttemplate_paramDefaultTypeInternal _ttemplate_param_default_instance_;
class ttemplates;
class ttemplatesDefaultTypeInternal;
extern ttemplatesDefaultTypeInternal _ttemplates_default_instance_;
}  // namespace pb

namespace pb {

namespace protobuf_templates_2eproto {
// Internal implementation detail -- do not call these.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[];
  static const ::google::protobuf::uint32 offsets[];
  static void InitDefaultsImpl();
  static void Shutdown();
};
void AddDescriptors();
void InitDefaults();
}  // namespace protobuf_templates_2eproto

// ===================================================================

class ttemplate_param : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:pb.ttemplate.param) */ {
 public:
  ttemplate_param();
  virtual ~ttemplate_param();

  ttemplate_param(const ttemplate_param& from);

  inline ttemplate_param& operator=(const ttemplate_param& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ttemplate_param& default_instance();

  static inline const ttemplate_param* internal_default_instance() {
    return reinterpret_cast<const ttemplate_param*>(
               &_ttemplate_param_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(ttemplate_param* other);

  // implements Message ----------------------------------------------

  inline ttemplate_param* New() const PROTOBUF_FINAL { return New(NULL); }

  ttemplate_param* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const ttemplate_param& from);
  void MergeFrom(const ttemplate_param& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(ttemplate_param* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string field_id = 2;
  void clear_field_id();
  static const int kFieldIdFieldNumber = 2;
  const ::std::string& field_id() const;
  void set_field_id(const ::std::string& value);
  #if LANG_CXX11
  void set_field_id(::std::string&& value);
  #endif
  void set_field_id(const char* value);
  void set_field_id(const char* value, size_t size);
  ::std::string* mutable_field_id();
  ::std::string* release_field_id();
  void set_allocated_field_id(::std::string* field_id);

  // int32 index = 1;
  void clear_index();
  static const int kIndexFieldNumber = 1;
  ::google::protobuf::int32 index() const;
  void set_index(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:pb.ttemplate.param)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr field_id_;
  ::google::protobuf::int32 index_;
  mutable int _cached_size_;
  friend struct protobuf_templates_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class ttemplate : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:pb.ttemplate) */ {
 public:
  ttemplate();
  virtual ~ttemplate();

  ttemplate(const ttemplate& from);

  inline ttemplate& operator=(const ttemplate& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ttemplate& default_instance();

  static inline const ttemplate* internal_default_instance() {
    return reinterpret_cast<const ttemplate*>(
               &_ttemplate_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    1;

  void Swap(ttemplate* other);

  // implements Message ----------------------------------------------

  inline ttemplate* New() const PROTOBUF_FINAL { return New(NULL); }

  ttemplate* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const ttemplate& from);
  void MergeFrom(const ttemplate& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(ttemplate* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  typedef ttemplate_param param;

  // accessors -------------------------------------------------------

  // repeated .pb.ttemplate.param params = 3;
  int params_size() const;
  void clear_params();
  static const int kParamsFieldNumber = 3;
  const ::pb::ttemplate_param& params(int index) const;
  ::pb::ttemplate_param* mutable_params(int index);
  ::pb::ttemplate_param* add_params();
  ::google::protobuf::RepeatedPtrField< ::pb::ttemplate_param >*
      mutable_params();
  const ::google::protobuf::RepeatedPtrField< ::pb::ttemplate_param >&
      params() const;

  // string name = 1;
  void clear_name();
  static const int kNameFieldNumber = 1;
  const ::std::string& name() const;
  void set_name(const ::std::string& value);
  #if LANG_CXX11
  void set_name(::std::string&& value);
  #endif
  void set_name(const char* value);
  void set_name(const char* value, size_t size);
  ::std::string* mutable_name();
  ::std::string* release_name();
  void set_allocated_name(::std::string* name);

  // string message = 2;
  void clear_message();
  static const int kMessageFieldNumber = 2;
  const ::std::string& message() const;
  void set_message(const ::std::string& value);
  #if LANG_CXX11
  void set_message(::std::string&& value);
  #endif
  void set_message(const char* value);
  void set_message(const char* value, size_t size);
  ::std::string* mutable_message();
  ::std::string* release_message();
  void set_allocated_message(::std::string* message);

  // @@protoc_insertion_point(class_scope:pb.ttemplate)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::RepeatedPtrField< ::pb::ttemplate_param > params_;
  ::google::protobuf::internal::ArenaStringPtr name_;
  ::google::protobuf::internal::ArenaStringPtr message_;
  mutable int _cached_size_;
  friend struct protobuf_templates_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class ttemplates : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:pb.ttemplates) */ {
 public:
  ttemplates();
  virtual ~ttemplates();

  ttemplates(const ttemplates& from);

  inline ttemplates& operator=(const ttemplates& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ttemplates& default_instance();

  static inline const ttemplates* internal_default_instance() {
    return reinterpret_cast<const ttemplates*>(
               &_ttemplates_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    2;

  void Swap(ttemplates* other);

  // implements Message ----------------------------------------------

  inline ttemplates* New() const PROTOBUF_FINAL { return New(NULL); }

  ttemplates* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const ttemplates& from);
  void MergeFrom(const ttemplates& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(ttemplates* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated .pb.ttemplate templates = 1;
  int templates_size() const;
  void clear_templates();
  static const int kTemplatesFieldNumber = 1;
  const ::pb::ttemplate& templates(int index) const;
  ::pb::ttemplate* mutable_templates(int index);
  ::pb::ttemplate* add_templates();
  ::google::protobuf::RepeatedPtrField< ::pb::ttemplate >*
      mutable_templates();
  const ::google::protobuf::RepeatedPtrField< ::pb::ttemplate >&
      templates() const;

  // int64 timestamp = 2;
  void clear_timestamp();
  static const int kTimestampFieldNumber = 2;
  ::google::protobuf::int64 timestamp() const;
  void set_timestamp(::google::protobuf::int64 value);

  // @@protoc_insertion_point(class_scope:pb.ttemplates)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::RepeatedPtrField< ::pb::ttemplate > templates_;
  ::google::protobuf::int64 timestamp_;
  mutable int _cached_size_;
  friend struct protobuf_templates_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// ttemplate_param

// int32 index = 1;
inline void ttemplate_param::clear_index() {
  index_ = 0;
}
inline ::google::protobuf::int32 ttemplate_param::index() const {
  // @@protoc_insertion_point(field_get:pb.ttemplate.param.index)
  return index_;
}
inline void ttemplate_param::set_index(::google::protobuf::int32 value) {
  
  index_ = value;
  // @@protoc_insertion_point(field_set:pb.ttemplate.param.index)
}

// string field_id = 2;
inline void ttemplate_param::clear_field_id() {
  field_id_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& ttemplate_param::field_id() const {
  // @@protoc_insertion_point(field_get:pb.ttemplate.param.field_id)
  return field_id_.GetNoArena();
}
inline void ttemplate_param::set_field_id(const ::std::string& value) {
  
  field_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:pb.ttemplate.param.field_id)
}
#if LANG_CXX11
inline void ttemplate_param::set_field_id(::std::string&& value) {
  
  field_id_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:pb.ttemplate.param.field_id)
}
#endif
inline void ttemplate_param::set_field_id(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  field_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:pb.ttemplate.param.field_id)
}
inline void ttemplate_param::set_field_id(const char* value, size_t size) {
  
  field_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:pb.ttemplate.param.field_id)
}
inline ::std::string* ttemplate_param::mutable_field_id() {
  
  // @@protoc_insertion_point(field_mutable:pb.ttemplate.param.field_id)
  return field_id_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ttemplate_param::release_field_id() {
  // @@protoc_insertion_point(field_release:pb.ttemplate.param.field_id)
  
  return field_id_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ttemplate_param::set_allocated_field_id(::std::string* field_id) {
  if (field_id != NULL) {
    
  } else {
    
  }
  field_id_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), field_id);
  // @@protoc_insertion_point(field_set_allocated:pb.ttemplate.param.field_id)
}

// -------------------------------------------------------------------

// ttemplate

// string name = 1;
inline void ttemplate::clear_name() {
  name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& ttemplate::name() const {
  // @@protoc_insertion_point(field_get:pb.ttemplate.name)
  return name_.GetNoArena();
}
inline void ttemplate::set_name(const ::std::string& value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:pb.ttemplate.name)
}
#if LANG_CXX11
inline void ttemplate::set_name(::std::string&& value) {
  
  name_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:pb.ttemplate.name)
}
#endif
inline void ttemplate::set_name(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:pb.ttemplate.name)
}
inline void ttemplate::set_name(const char* value, size_t size) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:pb.ttemplate.name)
}
inline ::std::string* ttemplate::mutable_name() {
  
  // @@protoc_insertion_point(field_mutable:pb.ttemplate.name)
  return name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ttemplate::release_name() {
  // @@protoc_insertion_point(field_release:pb.ttemplate.name)
  
  return name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ttemplate::set_allocated_name(::std::string* name) {
  if (name != NULL) {
    
  } else {
    
  }
  name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), name);
  // @@protoc_insertion_point(field_set_allocated:pb.ttemplate.name)
}

// string message = 2;
inline void ttemplate::clear_message() {
  message_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& ttemplate::message() const {
  // @@protoc_insertion_point(field_get:pb.ttemplate.message)
  return message_.GetNoArena();
}
inline void ttemplate::set_message(const ::std::string& value) {
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:pb.ttemplate.message)
}
#if LANG_CXX11
inline void ttemplate::set_message(::std::string&& value) {
  
  message_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:pb.ttemplate.message)
}
#endif
inline void ttemplate::set_message(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:pb.ttemplate.message)
}
inline void ttemplate::set_message(const char* value, size_t size) {
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:pb.ttemplate.message)
}
inline ::std::string* ttemplate::mutable_message() {
  
  // @@protoc_insertion_point(field_mutable:pb.ttemplate.message)
  return message_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ttemplate::release_message() {
  // @@protoc_insertion_point(field_release:pb.ttemplate.message)
  
  return message_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ttemplate::set_allocated_message(::std::string* message) {
  if (message != NULL) {
    
  } else {
    
  }
  message_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), message);
  // @@protoc_insertion_point(field_set_allocated:pb.ttemplate.message)
}

// repeated .pb.ttemplate.param params = 3;
inline int ttemplate::params_size() const {
  return params_.size();
}
inline void ttemplate::clear_params() {
  params_.Clear();
}
inline const ::pb::ttemplate_param& ttemplate::params(int index) const {
  // @@protoc_insertion_point(field_get:pb.ttemplate.params)
  return params_.Get(index);
}
inline ::pb::ttemplate_param* ttemplate::mutable_params(int index) {
  // @@protoc_insertion_point(field_mutable:pb.ttemplate.params)
  return params_.Mutable(index);
}
inline ::pb::ttemplate_param* ttemplate::add_params() {
  // @@protoc_insertion_point(field_add:pb.ttemplate.params)
  return params_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::pb::ttemplate_param >*
ttemplate::mutable_params() {
  // @@protoc_insertion_point(field_mutable_list:pb.ttemplate.params)
  return &params_;
}
inline const ::google::protobuf::RepeatedPtrField< ::pb::ttemplate_param >&
ttemplate::params() const {
  // @@protoc_insertion_point(field_list:pb.ttemplate.params)
  return params_;
}

// -------------------------------------------------------------------

// ttemplates

// repeated .pb.ttemplate templates = 1;
inline int ttemplates::templates_size() const {
  return templates_.size();
}
inline void ttemplates::clear_templates() {
  templates_.Clear();
}
inline const ::pb::ttemplate& ttemplates::templates(int index) const {
  // @@protoc_insertion_point(field_get:pb.ttemplates.templates)
  return templates_.Get(index);
}
inline ::pb::ttemplate* ttemplates::mutable_templates(int index) {
  // @@protoc_insertion_point(field_mutable:pb.ttemplates.templates)
  return templates_.Mutable(index);
}
inline ::pb::ttemplate* ttemplates::add_templates() {
  // @@protoc_insertion_point(field_add:pb.ttemplates.templates)
  return templates_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::pb::ttemplate >*
ttemplates::mutable_templates() {
  // @@protoc_insertion_point(field_mutable_list:pb.ttemplates.templates)
  return &templates_;
}
inline const ::google::protobuf::RepeatedPtrField< ::pb::ttemplate >&
ttemplates::templates() const {
  // @@protoc_insertion_point(field_list:pb.ttemplates.templates)
  return templates_;
}

// int64 timestamp = 2;
inline void ttemplates::clear_timestamp() {
  timestamp_ = GOOGLE_LONGLONG(0);
}
inline ::google::protobuf::int64 ttemplates::timestamp() const {
  // @@protoc_insertion_point(field_get:pb.ttemplates.timestamp)
  return timestamp_;
}
inline void ttemplates::set_timestamp(::google::protobuf::int64 value) {
  
  timestamp_ = value;
  // @@protoc_insertion_point(field_set:pb.ttemplates.timestamp)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)


}  // namespace pb

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_templates_2eproto__INCLUDED
