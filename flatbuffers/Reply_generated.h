// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_REPLY_H_
#define FLATBUFFERS_GENERATED_REPLY_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 3,
             "Non-compatible flatbuffers version included");

struct Reply;
struct ReplyBuilder;
struct ReplyT;

struct ReplyT : public ::flatbuffers::NativeTable {
  typedef Reply TableType;
  std::vector<double> r{};
  int32_t status = 0;
};

struct Reply FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ReplyT NativeTableType;
  typedef ReplyBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_R = 4,
    VT_STATUS = 6
  };
  const ::flatbuffers::Vector<double> *r() const {
    return GetPointer<const ::flatbuffers::Vector<double> *>(VT_R);
  }
  int32_t status() const {
    return GetField<int32_t>(VT_STATUS, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_R) &&
           verifier.VerifyVector(r()) &&
           VerifyField<int32_t>(verifier, VT_STATUS, 4) &&
           verifier.EndTable();
  }
  ReplyT *UnPack(const ::flatbuffers::resolver_function_t *_resolver = nullptr) const;
  void UnPackTo(ReplyT *_o, const ::flatbuffers::resolver_function_t *_resolver = nullptr) const;
  static ::flatbuffers::Offset<Reply> Pack(::flatbuffers::FlatBufferBuilder &_fbb, const ReplyT* _o, const ::flatbuffers::rehasher_function_t *_rehasher = nullptr);
};

struct ReplyBuilder {
  typedef Reply Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_r(::flatbuffers::Offset<::flatbuffers::Vector<double>> r) {
    fbb_.AddOffset(Reply::VT_R, r);
  }
  void add_status(int32_t status) {
    fbb_.AddElement<int32_t>(Reply::VT_STATUS, status, 0);
  }
  explicit ReplyBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Reply> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Reply>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Reply> CreateReply(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::Vector<double>> r = 0,
    int32_t status = 0) {
  ReplyBuilder builder_(_fbb);
  builder_.add_status(status);
  builder_.add_r(r);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<Reply> CreateReplyDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<double> *r = nullptr,
    int32_t status = 0) {
  auto r__ = r ? _fbb.CreateVector<double>(*r) : 0;
  return CreateReply(
      _fbb,
      r__,
      status);
}

::flatbuffers::Offset<Reply> CreateReply(::flatbuffers::FlatBufferBuilder &_fbb, const ReplyT *_o, const ::flatbuffers::rehasher_function_t *_rehasher = nullptr);

inline ReplyT *Reply::UnPack(const ::flatbuffers::resolver_function_t *_resolver) const {
  auto _o = std::unique_ptr<ReplyT>(new ReplyT());
  UnPackTo(_o.get(), _resolver);
  return _o.release();
}

inline void Reply::UnPackTo(ReplyT *_o, const ::flatbuffers::resolver_function_t *_resolver) const {
  (void)_o;
  (void)_resolver;
  { auto _e = r(); if (_e) { _o->r.resize(_e->size()); for (::flatbuffers::uoffset_t _i = 0; _i < _e->size(); _i++) { _o->r[_i] = _e->Get(_i); } } else { _o->r.resize(0); } }
  { auto _e = status(); _o->status = _e; }
}

inline ::flatbuffers::Offset<Reply> Reply::Pack(::flatbuffers::FlatBufferBuilder &_fbb, const ReplyT* _o, const ::flatbuffers::rehasher_function_t *_rehasher) {
  return CreateReply(_fbb, _o, _rehasher);
}

inline ::flatbuffers::Offset<Reply> CreateReply(::flatbuffers::FlatBufferBuilder &_fbb, const ReplyT *_o, const ::flatbuffers::rehasher_function_t *_rehasher) {
  (void)_rehasher;
  (void)_o;
  struct _VectorArgs { ::flatbuffers::FlatBufferBuilder *__fbb; const ReplyT* __o; const ::flatbuffers::rehasher_function_t *__rehasher; } _va = { &_fbb, _o, _rehasher}; (void)_va;
  auto _r = _o->r.size() ? _fbb.CreateVector(_o->r) : 0;
  auto _status = _o->status;
  return CreateReply(
      _fbb,
      _r,
      _status);
}

inline const Reply *GetReply(const void *buf) {
  return ::flatbuffers::GetRoot<Reply>(buf);
}

inline const Reply *GetSizePrefixedReply(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<Reply>(buf);
}

inline bool VerifyReplyBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<Reply>(nullptr);
}

inline bool VerifySizePrefixedReplyBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<Reply>(nullptr);
}

inline void FinishReplyBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<Reply> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedReplyBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<Reply> root) {
  fbb.FinishSizePrefixed(root);
}

inline std::unique_ptr<ReplyT> UnPackReply(
    const void *buf,
    const ::flatbuffers::resolver_function_t *res = nullptr) {
  return std::unique_ptr<ReplyT>(GetReply(buf)->UnPack(res));
}

inline std::unique_ptr<ReplyT> UnPackSizePrefixedReply(
    const void *buf,
    const ::flatbuffers::resolver_function_t *res = nullptr) {
  return std::unique_ptr<ReplyT>(GetSizePrefixedReply(buf)->UnPack(res));
}

#endif  // FLATBUFFERS_GENERATED_REPLY_H_
