/**
 * type.h
 */
#pragma once
#include "type/value.h"

namespace cmudb {
/* A varlen value is an abstract class representing all objects that have
 * variable length.
 * For simplicity, for valen_type we always set flag "inline" as true, which
 * means we store actual data along with its size rather than a pointer
 */
class VarlenType : public Type {
public:
  VarlenType(TypeId type);
  ~VarlenType();

  // Access the raw variable length data
  const char *GetData(const Value &val) const override;

  // Get the length of the variable length data
  uint32_t GetLength(const Value &val) const override;

  // Comparison functions
  CmpBool CompareEquals(const Value &left, const Value &right) const override;
  CmpBool CompareNotEquals(const Value &left,
                           const Value &right) const override;
  CmpBool CompareLessThan(const Value &left, const Value &right) const override;
  CmpBool CompareLessThanEquals(const Value &left,
                                const Value &right) const override;
  CmpBool CompareGreaterThan(const Value &left,
                             const Value &right) const override;
  CmpBool CompareGreaterThanEquals(const Value &left,
                                   const Value &right) const override;

  // Other mathematical functions
  virtual Value Min(const Value &left, const Value &right) const override;
  virtual Value Max(const Value &left, const Value &right) const override;

  Value CastAs(const Value &value, const TypeId type_id) const override;

  // Decimal types are always inlined
  bool IsInlined(const Value &) const override { return false; }

  // Debug
  std::string ToString(const Value &val) const override;

  // Serialize this value into the given storage space
  void SerializeTo(const Value &val, char *storage) const override;

  // Deserialize a value of the given type from the given storage space.
  Value DeserializeFrom(const char *storage) const override;

  // Create a copy of this value
  Value Copy(const Value &val) const override;
};
} // namespace cmudb
