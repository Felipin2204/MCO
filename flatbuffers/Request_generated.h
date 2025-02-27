// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_REQUEST_H_
#define FLATBUFFERS_GENERATED_REQUEST_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 3,
             "Non-compatible flatbuffers version included");

struct Request;
struct RequestBuilder;
struct RequestT;

struct RequestT : public ::flatbuffers::NativeTable {
  typedef Request TableType;
  int32_t id = 0;
  std::vector<double> lambda{};
  std::vector<double> sum_weights{};
  std::vector<double> tmin{};
  double alpha = 0.0;
  std::vector<double> omega{};
  std::vector<double> capacity{};
  double d = 0.0;
  double nv = 0.0;
  double max_capacity = 0.0;
  bool regularized = false;
  double beta = 0.0;
};

struct Request FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef RequestT NativeTableType;
  typedef RequestBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ID = 4,
    VT_LAMBDA = 6,
    VT_SUM_WEIGHTS = 8,
    VT_TMIN = 10,
    VT_ALPHA = 12,
    VT_OMEGA = 14,
    VT_CAPACITY = 16,
    VT_D = 18,
    VT_NV = 20,
    VT_MAX_CAPACITY = 22,
    VT_REGULARIZED = 24,
    VT_BETA = 26
  };
  int32_t id() const {
    return GetField<int32_t>(VT_ID, 0);
  }
  const ::flatbuffers::Vector<double> *lambda() const {
    return GetPointer<const ::flatbuffers::Vector<double> *>(VT_LAMBDA);
  }
  const ::flatbuffers::Vector<double> *sum_weights() const {
    return GetPointer<const ::flatbuffers::Vector<double> *>(VT_SUM_WEIGHTS);
  }
  const ::flatbuffers::Vector<double> *tmin() const {
    return GetPointer<const ::flatbuffers::Vector<double> *>(VT_TMIN);
  }
  double alpha() const {
    return GetField<double>(VT_ALPHA, 0.0);
  }
  const ::flatbuffers::Vector<double> *omega() const {
    return GetPointer<const ::flatbuffers::Vector<double> *>(VT_OMEGA);
  }
  const ::flatbuffers::Vector<double> *capacity() const {
    return GetPointer<const ::flatbuffers::Vector<double> *>(VT_CAPACITY);
  }
  double d() const {
    return GetField<double>(VT_D, 0.0);
  }
  double nv() const {
    return GetField<double>(VT_NV, 0.0);
  }
  double max_capacity() const {
    return GetField<double>(VT_MAX_CAPACITY, 0.0);
  }
  bool regularized() const {
    return GetField<uint8_t>(VT_REGULARIZED, 0) != 0;
  }
  double beta() const {
    return GetField<double>(VT_BETA, 0.0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int32_t>(verifier, VT_ID, 4) &&
           VerifyOffset(verifier, VT_LAMBDA) &&
           verifier.VerifyVector(lambda()) &&
           VerifyOffset(verifier, VT_SUM_WEIGHTS) &&
           verifier.VerifyVector(sum_weights()) &&
           VerifyOffset(verifier, VT_TMIN) &&
           verifier.VerifyVector(tmin()) &&
           VerifyField<double>(verifier, VT_ALPHA, 8) &&
           VerifyOffset(verifier, VT_OMEGA) &&
           verifier.VerifyVector(omega()) &&
           VerifyOffset(verifier, VT_CAPACITY) &&
           verifier.VerifyVector(capacity()) &&
           VerifyField<double>(verifier, VT_D, 8) &&
           VerifyField<double>(verifier, VT_NV, 8) &&
           VerifyField<double>(verifier, VT_MAX_CAPACITY, 8) &&
           VerifyField<uint8_t>(verifier, VT_REGULARIZED, 1) &&
           VerifyField<double>(verifier, VT_BETA, 8) &&
           verifier.EndTable();
  }
  RequestT *UnPack(const ::flatbuffers::resolver_function_t *_resolver = nullptr) const;
  void UnPackTo(RequestT *_o, const ::flatbuffers::resolver_function_t *_resolver = nullptr) const;
  static ::flatbuffers::Offset<Request> Pack(::flatbuffers::FlatBufferBuilder &_fbb, const RequestT* _o, const ::flatbuffers::rehasher_function_t *_rehasher = nullptr);
};

struct RequestBuilder {
  typedef Request Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_id(int32_t id) {
    fbb_.AddElement<int32_t>(Request::VT_ID, id, 0);
  }
  void add_lambda(::flatbuffers::Offset<::flatbuffers::Vector<double>> lambda) {
    fbb_.AddOffset(Request::VT_LAMBDA, lambda);
  }
  void add_sum_weights(::flatbuffers::Offset<::flatbuffers::Vector<double>> sum_weights) {
    fbb_.AddOffset(Request::VT_SUM_WEIGHTS, sum_weights);
  }
  void add_tmin(::flatbuffers::Offset<::flatbuffers::Vector<double>> tmin) {
    fbb_.AddOffset(Request::VT_TMIN, tmin);
  }
  void add_alpha(double alpha) {
    fbb_.AddElement<double>(Request::VT_ALPHA, alpha, 0.0);
  }
  void add_omega(::flatbuffers::Offset<::flatbuffers::Vector<double>> omega) {
    fbb_.AddOffset(Request::VT_OMEGA, omega);
  }
  void add_capacity(::flatbuffers::Offset<::flatbuffers::Vector<double>> capacity) {
    fbb_.AddOffset(Request::VT_CAPACITY, capacity);
  }
  void add_d(double d) {
    fbb_.AddElement<double>(Request::VT_D, d, 0.0);
  }
  void add_nv(double nv) {
    fbb_.AddElement<double>(Request::VT_NV, nv, 0.0);
  }
  void add_max_capacity(double max_capacity) {
    fbb_.AddElement<double>(Request::VT_MAX_CAPACITY, max_capacity, 0.0);
  }
  void add_regularized(bool regularized) {
    fbb_.AddElement<uint8_t>(Request::VT_REGULARIZED, static_cast<uint8_t>(regularized), 0);
  }
  void add_beta(double beta) {
    fbb_.AddElement<double>(Request::VT_BETA, beta, 0.0);
  }
  explicit RequestBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Request> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Request>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Request> CreateRequest(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    int32_t id = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<double>> lambda = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<double>> sum_weights = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<double>> tmin = 0,
    double alpha = 0.0,
    ::flatbuffers::Offset<::flatbuffers::Vector<double>> omega = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<double>> capacity = 0,
    double d = 0.0,
    double nv = 0.0,
    double max_capacity = 0.0,
    bool regularized = false,
    double beta = 0.0) {
  RequestBuilder builder_(_fbb);
  builder_.add_beta(beta);
  builder_.add_max_capacity(max_capacity);
  builder_.add_nv(nv);
  builder_.add_d(d);
  builder_.add_alpha(alpha);
  builder_.add_capacity(capacity);
  builder_.add_omega(omega);
  builder_.add_tmin(tmin);
  builder_.add_sum_weights(sum_weights);
  builder_.add_lambda(lambda);
  builder_.add_id(id);
  builder_.add_regularized(regularized);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<Request> CreateRequestDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    int32_t id = 0,
    const std::vector<double> *lambda = nullptr,
    const std::vector<double> *sum_weights = nullptr,
    const std::vector<double> *tmin = nullptr,
    double alpha = 0.0,
    const std::vector<double> *omega = nullptr,
    const std::vector<double> *capacity = nullptr,
    double d = 0.0,
    double nv = 0.0,
    double max_capacity = 0.0,
    bool regularized = false,
    double beta = 0.0) {
  auto lambda__ = lambda ? _fbb.CreateVector<double>(*lambda) : 0;
  auto sum_weights__ = sum_weights ? _fbb.CreateVector<double>(*sum_weights) : 0;
  auto tmin__ = tmin ? _fbb.CreateVector<double>(*tmin) : 0;
  auto omega__ = omega ? _fbb.CreateVector<double>(*omega) : 0;
  auto capacity__ = capacity ? _fbb.CreateVector<double>(*capacity) : 0;
  return CreateRequest(
      _fbb,
      id,
      lambda__,
      sum_weights__,
      tmin__,
      alpha,
      omega__,
      capacity__,
      d,
      nv,
      max_capacity,
      regularized,
      beta);
}

::flatbuffers::Offset<Request> CreateRequest(::flatbuffers::FlatBufferBuilder &_fbb, const RequestT *_o, const ::flatbuffers::rehasher_function_t *_rehasher = nullptr);

inline RequestT *Request::UnPack(const ::flatbuffers::resolver_function_t *_resolver) const {
  auto _o = std::unique_ptr<RequestT>(new RequestT());
  UnPackTo(_o.get(), _resolver);
  return _o.release();
}

inline void Request::UnPackTo(RequestT *_o, const ::flatbuffers::resolver_function_t *_resolver) const {
  (void)_o;
  (void)_resolver;
  { auto _e = id(); _o->id = _e; }
  { auto _e = lambda(); if (_e) { _o->lambda.resize(_e->size()); for (::flatbuffers::uoffset_t _i = 0; _i < _e->size(); _i++) { _o->lambda[_i] = _e->Get(_i); } } else { _o->lambda.resize(0); } }
  { auto _e = sum_weights(); if (_e) { _o->sum_weights.resize(_e->size()); for (::flatbuffers::uoffset_t _i = 0; _i < _e->size(); _i++) { _o->sum_weights[_i] = _e->Get(_i); } } else { _o->sum_weights.resize(0); } }
  { auto _e = tmin(); if (_e) { _o->tmin.resize(_e->size()); for (::flatbuffers::uoffset_t _i = 0; _i < _e->size(); _i++) { _o->tmin[_i] = _e->Get(_i); } } else { _o->tmin.resize(0); } }
  { auto _e = alpha(); _o->alpha = _e; }
  { auto _e = omega(); if (_e) { _o->omega.resize(_e->size()); for (::flatbuffers::uoffset_t _i = 0; _i < _e->size(); _i++) { _o->omega[_i] = _e->Get(_i); } } else { _o->omega.resize(0); } }
  { auto _e = capacity(); if (_e) { _o->capacity.resize(_e->size()); for (::flatbuffers::uoffset_t _i = 0; _i < _e->size(); _i++) { _o->capacity[_i] = _e->Get(_i); } } else { _o->capacity.resize(0); } }
  { auto _e = d(); _o->d = _e; }
  { auto _e = nv(); _o->nv = _e; }
  { auto _e = max_capacity(); _o->max_capacity = _e; }
  { auto _e = regularized(); _o->regularized = _e; }
  { auto _e = beta(); _o->beta = _e; }
}

inline ::flatbuffers::Offset<Request> Request::Pack(::flatbuffers::FlatBufferBuilder &_fbb, const RequestT* _o, const ::flatbuffers::rehasher_function_t *_rehasher) {
  return CreateRequest(_fbb, _o, _rehasher);
}

inline ::flatbuffers::Offset<Request> CreateRequest(::flatbuffers::FlatBufferBuilder &_fbb, const RequestT *_o, const ::flatbuffers::rehasher_function_t *_rehasher) {
  (void)_rehasher;
  (void)_o;
  struct _VectorArgs { ::flatbuffers::FlatBufferBuilder *__fbb; const RequestT* __o; const ::flatbuffers::rehasher_function_t *__rehasher; } _va = { &_fbb, _o, _rehasher}; (void)_va;
  auto _id = _o->id;
  auto _lambda = _o->lambda.size() ? _fbb.CreateVector(_o->lambda) : 0;
  auto _sum_weights = _o->sum_weights.size() ? _fbb.CreateVector(_o->sum_weights) : 0;
  auto _tmin = _o->tmin.size() ? _fbb.CreateVector(_o->tmin) : 0;
  auto _alpha = _o->alpha;
  auto _omega = _o->omega.size() ? _fbb.CreateVector(_o->omega) : 0;
  auto _capacity = _o->capacity.size() ? _fbb.CreateVector(_o->capacity) : 0;
  auto _d = _o->d;
  auto _nv = _o->nv;
  auto _max_capacity = _o->max_capacity;
  auto _regularized = _o->regularized;
  auto _beta = _o->beta;
  return CreateRequest(
      _fbb,
      _id,
      _lambda,
      _sum_weights,
      _tmin,
      _alpha,
      _omega,
      _capacity,
      _d,
      _nv,
      _max_capacity,
      _regularized,
      _beta);
}

inline const Request *GetRequest(const void *buf) {
  return ::flatbuffers::GetRoot<Request>(buf);
}

inline const Request *GetSizePrefixedRequest(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<Request>(buf);
}

inline bool VerifyRequestBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<Request>(nullptr);
}

inline bool VerifySizePrefixedRequestBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<Request>(nullptr);
}

inline void FinishRequestBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<Request> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedRequestBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<Request> root) {
  fbb.FinishSizePrefixed(root);
}

inline std::unique_ptr<RequestT> UnPackRequest(
    const void *buf,
    const ::flatbuffers::resolver_function_t *res = nullptr) {
  return std::unique_ptr<RequestT>(GetRequest(buf)->UnPack(res));
}

inline std::unique_ptr<RequestT> UnPackSizePrefixedRequest(
    const void *buf,
    const ::flatbuffers::resolver_function_t *res = nullptr) {
  return std::unique_ptr<RequestT>(GetSizePrefixedRequest(buf)->UnPack(res));
}

#endif  // FLATBUFFERS_GENERATED_REQUEST_H_
