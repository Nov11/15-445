/**
 * bigint_type.cpp
 */
#include <cassert>
#include <cmath>
#include <iostream>

#include "type/bigint_type.h"
namespace cmudb {
#define BIGINT_COMPARE_FUNC(OP)                                                \
  switch (right.GetTypeId()) {                                                 \
  case TypeId::TINYINT:                                                        \
    return GetCmpBool(left.value_.bigint OP right.GetAs<int8_t>());            \
  case TypeId::SMALLINT:                                                       \
    return GetCmpBool(left.value_.bigint OP right.GetAs<int16_t>());           \
  case TypeId::INTEGER:                                                        \
    return GetCmpBool(left.value_.bigint OP right.GetAs<int32_t>());           \
  case TypeId::BIGINT:                                                         \
    return GetCmpBool(left.value_.bigint OP right.GetAs<int64_t>());           \
  case TypeId::DECIMAL:                                                        \
    return GetCmpBool(left.value_.bigint OP right.GetAs<double>());            \
  case TypeId::VARCHAR: {                                                      \
    auto r_value = right.CastAs(TypeId::BIGINT);                               \
    return GetCmpBool(left.value_.bigint OP r_value.GetAs<int64_t>());         \
  }                                                                            \
  default:                                                                     \
    break;                                                                     \
  } // SWITCH

#define BIGINT_MODIFY_FUNC(METHOD, OP)                                         \
  switch (right.GetTypeId()) {                                                 \
  case TypeId::TINYINT:                                                        \
    return METHOD<int64_t, int8_t>(left, right);                               \
  case TypeId::SMALLINT:                                                       \
    return METHOD<int64_t, int16_t>(left, right);                              \
  case TypeId::INTEGER:                                                        \
    return METHOD<int64_t, int32_t>(left, right);                              \
  case TypeId::BIGINT:                                                         \
    return METHOD<int64_t, int64_t>(left, right);                              \
  case TypeId::DECIMAL:                                                        \
    return Value(TypeId::DECIMAL,                                              \
                 left.value_.bigint OP right.GetAs<double>());                 \
  case TypeId::VARCHAR: {                                                      \
    auto r_value = right.CastAs(TypeId::BIGINT);                               \
    return METHOD<int64_t, int64_t>(left, r_value);                            \
  }                                                                            \
  default:                                                                     \
    break;                                                                     \
  } // SWITCH

BigintType::BigintType() : IntegerParentType(BIGINT) {}

bool BigintType::IsZero(const Value &val) const {
  return (val.value_.bigint == 0);
}

Value BigintType::Add(const Value &left, const Value &right) const {
  assert(left.CheckInteger());
  assert(left.CheckComparable(right));
  if (left.IsNull() || right.IsNull())
    return left.OperateNull(right);

  BIGINT_MODIFY_FUNC(AddValue, +);

  throw Exception("type error");
}

Value BigintType::Subtract(const Value &left, const Value &right) const {
  assert(left.CheckInteger());
  assert(left.CheckComparable(right));
  if (left.IsNull() || right.IsNull())
    return left.OperateNull(right);

  BIGINT_MODIFY_FUNC(SubtractValue, -);

  throw Exception("type error");
}

Value BigintType::Multiply(const Value &left, const Value &right) const {
  assert(left.CheckInteger());
  assert(left.CheckComparable(right));
  if (left.IsNull() || right.IsNull())
    return left.OperateNull(right);

  BIGINT_MODIFY_FUNC(MultiplyValue, *);

  throw Exception("type error");
}

Value BigintType::Divide(const Value &left, const Value &right) const {
  assert(left.CheckInteger());
  assert(left.CheckComparable(right));
  if (left.IsNull() || right.IsNull())
    return left.OperateNull(right);

  if (right.IsZero()) {
    throw Exception(EXCEPTION_TYPE_DIVIDE_BY_ZERO,
                    "Division by zero on right-hand side");
  }

  BIGINT_MODIFY_FUNC(DivideValue, /);
  throw Exception("type error");
}

Value BigintType::Modulo(const Value &left, const Value &right) const {
  assert(left.CheckInteger());
  assert(left.CheckComparable(right));
  if (left.IsNull() || right.IsNull())
    return left.OperateNull(right);

  if (right.IsZero()) {
    throw Exception(EXCEPTION_TYPE_DIVIDE_BY_ZERO,
                    "Division by zero on right-hand side");
  }

  switch (right.GetTypeId()) {
  case TypeId::TINYINT:
    return ModuloValue<int64_t, int8_t>(left, right);
  case TypeId::SMALLINT:
    return ModuloValue<int64_t, int16_t>(left, right);
  case TypeId::INTEGER:
    return ModuloValue<int64_t, int32_t>(left, right);
  case TypeId::BIGINT:
    return ModuloValue<int64_t, int64_t>(left, right);
  case TypeId::DECIMAL:
    return Value(TypeId::DECIMAL,
                 ValMod(left.value_.bigint, right.GetAs<double>()));
  case TypeId::VARCHAR: {
    auto r_value = right.CastAs(TypeId::BIGINT);
    return ModuloValue<int64_t, int64_t>(left, r_value);
  }
  default:
    break;
  }
  throw Exception("type error");
}

Value BigintType::Sqrt(const Value &val) const {
  assert(val.CheckInteger());
  if (val.IsNull())
    return Value(TypeId::DECIMAL, PELOTON_DECIMAL_NULL);

  if (val.value_.bigint < 0) {
    throw Exception(EXCEPTION_TYPE_DECIMAL,
                    "Cannot take square root of a negative number.");
  }
  return Value(TypeId::DECIMAL, std::sqrt(val.value_.bigint));
}

Value BigintType::OperateNull(const Value &left __attribute__((unused)),
                              const Value &right) const {

  switch (right.GetTypeId()) {
  case TypeId::TINYINT:
  case TypeId::SMALLINT:
  case TypeId::INTEGER:
  case TypeId::BIGINT:
    return Value(TypeId::BIGINT, (int64_t)PELOTON_INT64_NULL);
  case TypeId::DECIMAL:
    return Value(TypeId::DECIMAL, (double)PELOTON_DECIMAL_NULL);
  default:
    break;
  }
  throw Exception("type error");
}

CmpBool BigintType::CompareEquals(const Value &left, const Value &right) const {
  assert(left.CheckInteger());
  assert(left.CheckComparable(right));

  if (left.IsNull() || right.IsNull())
    return CMP_NULL;

  BIGINT_COMPARE_FUNC(==);

  throw Exception("type error");
}

CmpBool BigintType::CompareNotEquals(const Value &left,
                                     const Value &right) const {
  assert(left.CheckInteger());
  assert(left.CheckComparable(right));
  if (left.IsNull() || right.IsNull())
    return CMP_NULL;

  BIGINT_COMPARE_FUNC(!=);

  throw Exception("type error");
}

CmpBool BigintType::CompareLessThan(const Value &left,
                                    const Value &right) const {
  assert(left.CheckInteger());
  assert(left.CheckComparable(right));
  if (left.IsNull() || right.IsNull())
    return CMP_NULL;

  BIGINT_COMPARE_FUNC(<);

  throw Exception("type error");
}

CmpBool BigintType::CompareLessThanEquals(const Value &left,
                                          const Value &right) const {
  assert(left.CheckInteger());
  assert(left.CheckComparable(right));
  if (left.IsNull() || right.IsNull())
    return CMP_NULL;

  BIGINT_COMPARE_FUNC(<=);

  throw Exception("type error");
}

CmpBool BigintType::CompareGreaterThan(const Value &left,
                                       const Value &right) const {
  assert(left.CheckInteger());
  assert(left.CheckComparable(right));
  if (left.IsNull() || right.IsNull())
    return CMP_NULL;

  BIGINT_COMPARE_FUNC(>);

  throw Exception("type error");
}

CmpBool BigintType::CompareGreaterThanEquals(const Value &left,
                                             const Value &right) const {
  assert(left.CheckInteger());
  assert(left.CheckComparable(right));
  if (left.IsNull() || right.IsNull())
    return CMP_NULL;

  BIGINT_COMPARE_FUNC(>=);
  throw Exception("type error");
}

std::string BigintType::ToString(const Value &val) const {
  assert(val.CheckInteger());

  if (val.IsNull())
    return "bigint_null";
  return std::to_string(val.value_.bigint);
}

void BigintType::SerializeTo(const Value &val, char *storage) const {

  *reinterpret_cast<int64_t *>(storage) = val.value_.bigint;
}

// Deserialize a value of the given type from the given storage space.
Value BigintType::DeserializeFrom(const char *storage) const {
  int64_t val = *reinterpret_cast<const int64_t *>(storage);
  return Value(type_id_, val);
}

Value BigintType::Copy(const Value &val) const {
  return Value(TypeId::BIGINT, val.value_.bigint);
}

Value BigintType::CastAs(const Value &val, const TypeId type_id) const {

  switch (type_id) {
  case TypeId::TINYINT: {
    if (val.IsNull())
      return Value(type_id, PELOTON_INT8_NULL);
    if (val.GetAs<int64_t>() > PELOTON_INT8_MAX ||
        val.GetAs<int64_t>() < PELOTON_INT8_MIN)
      throw Exception(EXCEPTION_TYPE_OUT_OF_RANGE,
                      "Numeric value out of range.");
    return Value(type_id, (int8_t)val.GetAs<int64_t>());
  }
  case TypeId::SMALLINT: {
    if (val.IsNull())
      return Value(type_id, PELOTON_INT16_NULL);
    if (val.GetAs<int64_t>() > PELOTON_INT16_MAX ||
        val.GetAs<int64_t>() < PELOTON_INT16_MIN)
      throw Exception(EXCEPTION_TYPE_OUT_OF_RANGE,
                      "Numeric value out of range.");
    return Value(type_id, (int16_t)val.GetAs<int64_t>());
  }
  case TypeId::INTEGER: {
    if (val.IsNull())
      return Value(type_id, PELOTON_INT32_NULL);
    if (val.GetAs<int64_t>() > PELOTON_INT32_MAX ||
        val.GetAs<int64_t>() < PELOTON_INT32_MIN)
      throw Exception(EXCEPTION_TYPE_OUT_OF_RANGE,
                      "Numeric value out of range.");
    return Value(type_id, (int32_t)val.GetAs<int64_t>());
  }

  case TypeId::BIGINT: {
    if (val.IsNull())
      return Value(type_id, PELOTON_INT64_NULL);
    return Copy(val);
  }

  case TypeId::DECIMAL: {
    if (val.IsNull())
      return Value(type_id, PELOTON_DECIMAL_NULL);
    return Value(type_id, (double)val.GetAs<int64_t>());
  }

  case TypeId::VARCHAR: {
    if (val.IsNull())
      return Value(TypeId::VARCHAR, nullptr, 0, false);
    return Value(TypeId::VARCHAR, val.ToString());
  }
  default:
    break;
  }
  throw Exception("bigint is not coercable to " +
                  Type::TypeIdToString(type_id));
}
} // namespace cmudb
