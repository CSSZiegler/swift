//===--- ErrorObject.h - Cocoa-interoperable recoverable error object -----===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// This implements the object representation of the standard ErrorType protocol
// type, which represents recoverable errors in the language. This
// implementation is designed to interoperate efficiently with Cocoa libraries
// by:
// - allowing for NSError and CFError objects to "toll-free bridge" to
//   ErrorType existentials, which allows for cheap Cocoa to Swift interop
// - allowing a native Swift error to lazily "become" an NSError when
//   passed into Cocoa, allowing for cheap Swift to Cocoa interop
//
//===----------------------------------------------------------------------===//

#ifndef __SWIFT_RUNTIME_ERROROBJECT_H__
#define __SWIFT_RUNTIME_ERROROBJECT_H__

#include "swift/Runtime/Metadata.h"
#include "swift/Runtime/HeapObject.h"
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>

namespace swift {

/// A mockery of the physical layout of NSError and CFError.
struct OpaqueNSError {
  CFRuntimeBase base;
  CFIndex code;
  CFStringRef domain;
  CFDictionaryRef userInfo;
};

/// The layout of the Swift ErrorType box.
struct SwiftError : OpaqueNSError {
  // By inheriting OpaqueNSError, the SwiftError structure reserves enough
  // space within itself to lazily emplace an NSError instance, and gets
  // Core Foundation's refcounting scheme.

  /// The type of Swift error value contained in the box.
  const Metadata *type;
  /// The ErrorType witness table.
  const WitnessTable *errorConformance;
  
  /// Get a pointer to the value, which is tail-allocated after
  /// the fixed header.
  const OpaqueValue *getValue() const {
    // If the box is a bridged NSError, then the box's address is itself the
    // value.
    if (isPureNSError())
      return reinterpret_cast<const OpaqueValue *>(this);
  
    auto baseAddr = reinterpret_cast<uintptr_t>(this + 1);
    // Round up to the value's alignment.
    unsigned alignMask = type->getValueWitnesses()->getAlignmentMask();
    baseAddr = (baseAddr + alignMask) & ~alignMask;
    return reinterpret_cast<const OpaqueValue *>(baseAddr);
  }
  OpaqueValue *getValue() {
    return const_cast<OpaqueValue*>(
             const_cast<const SwiftError *>(this)->getValue());
  }
  
  // True if the object is really an NSError or CFError instance.
  // The type and errorConformance fields don't exist in an NSError.
  bool isPureNSError() const {
    // TODO
    return false;
  }
  
  /// Get the type of the contained value.
  const Metadata *getType() const;
  /// Get the ErrorType protocol witness table for the contained type.
  const WitnessTable *getErrorConformance() const;
  
  // Don't copy or move, please.
  SwiftError(const SwiftError &) = delete;
  SwiftError(SwiftError &&) = delete;
  SwiftError &operator=(const SwiftError &) = delete;
  SwiftError &operator=(SwiftError &&) = delete;
};
  
/// Allocate a catchable error object.
extern "C" BoxPair::Return swift_allocError(const Metadata *type,
                                         const WitnessTable *errorConformance);
  
/// Deallocate an error object whose contained object has already been
/// destroyed.
extern "C" void swift_deallocError(SwiftError *error, const Metadata *type);

struct ErrorValueResult {
  const OpaqueValue *value;
  const Metadata *type;
  const WitnessTable *errorConformance;
};

/// Extract a pointer to the value, the type metadata, and the ErrorType
/// protocol witness from an error object.
///
/// The "scratch" pointer should point to an uninitialized word-sized
/// temporary buffer. The implementation may write a reference to itself to
/// that buffer if the error object is a toll-free-bridged NSError instead of
/// a native Swift error, in which case the object itself is the "boxed" value.
extern "C" void swift_getErrorValue(const SwiftError *errorObject,
                                    void **scratch,
                                    ErrorValueResult *out);

} // namespace swift

#endif