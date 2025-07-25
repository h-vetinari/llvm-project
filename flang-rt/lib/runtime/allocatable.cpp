//===-- lib/runtime/allocatable.cpp -----------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "flang/Runtime/allocatable.h"
#include "flang-rt/runtime/assign-impl.h"
#include "flang-rt/runtime/derived.h"
#include "flang-rt/runtime/descriptor.h"
#include "flang-rt/runtime/stat.h"
#include "flang-rt/runtime/terminator.h"
#include "flang-rt/runtime/type-info.h"
#include "flang/Common/ISO_Fortran_binding_wrapper.h"
#include "flang/Runtime/assign.h"

namespace Fortran::runtime {
extern "C" {
RT_EXT_API_GROUP_BEGIN

void RTDEF(AllocatableInitIntrinsic)(Descriptor &descriptor,
    TypeCategory category, int kind, int rank, int corank) {
  INTERNAL_CHECK(corank == 0);
  descriptor.Establish(TypeCode{category, kind},
      Descriptor::BytesFor(category, kind), nullptr, rank, nullptr,
      CFI_attribute_allocatable);
}

void RTDEF(AllocatableInitCharacter)(Descriptor &descriptor,
    SubscriptValue length, int kind, int rank, int corank) {
  INTERNAL_CHECK(corank == 0);
  descriptor.Establish(
      kind, length, nullptr, rank, nullptr, CFI_attribute_allocatable);
}

void RTDEF(AllocatableInitDerived)(Descriptor &descriptor,
    const typeInfo::DerivedType &derivedType, int rank, int corank) {
  INTERNAL_CHECK(corank == 0);
  descriptor.Establish(
      derivedType, nullptr, rank, nullptr, CFI_attribute_allocatable);
}

void RTDEF(AllocatableInitIntrinsicForAllocate)(Descriptor &descriptor,
    TypeCategory category, int kind, int rank, int corank) {
  if (!descriptor.IsAllocated()) {
    RTNAME(AllocatableInitIntrinsic)(descriptor, category, kind, rank, corank);
  }
}

void RTDEF(AllocatableInitCharacterForAllocate)(Descriptor &descriptor,
    SubscriptValue length, int kind, int rank, int corank) {
  if (!descriptor.IsAllocated()) {
    RTNAME(AllocatableInitCharacter)(descriptor, length, kind, rank, corank);
  }
}

void RTDEF(AllocatableInitDerivedForAllocate)(Descriptor &descriptor,
    const typeInfo::DerivedType &derivedType, int rank, int corank) {
  if (!descriptor.IsAllocated()) {
    RTNAME(AllocatableInitDerived)(descriptor, derivedType, rank, corank);
  }
}

std::int32_t RTDEF(MoveAlloc)(Descriptor &to, Descriptor &from,
    const typeInfo::DerivedType *derivedType, bool hasStat,
    const Descriptor *errMsg, const char *sourceFile, int sourceLine) {
  Terminator terminator{sourceFile, sourceLine};

  // If to and from are the same allocatable they must not be allocated
  // and nothing should be done.
  if (from.raw().base_addr == to.raw().base_addr && from.IsAllocated()) {
    return ReturnError(
        terminator, StatMoveAllocSameAllocatable, errMsg, hasStat);
  }

  if (to.IsAllocated()) {
    int stat{
        to.Destroy(/*finalize=*/true, /*destroyPointers=*/false, &terminator)};
    if (stat != StatOk) {
      return ReturnError(terminator, stat, errMsg, hasStat);
    }
  }

  // If from isn't allocated, the standard defines that nothing should be done.
  if (from.IsAllocated()) {
    to = from;
    from.raw().base_addr = nullptr;

    // Carry over the dynamic type.
    if (auto *toAddendum{to.Addendum()}) {
      if (const auto *fromAddendum{from.Addendum()}) {
        if (const auto *derived{fromAddendum->derivedType()}) {
          toAddendum->set_derivedType(derived);
        }
      }
    }

    // Reset from dynamic type if needed.
    if (auto *fromAddendum{from.Addendum()}) {
      if (derivedType) {
        fromAddendum->set_derivedType(derivedType);
      }
    }
  }

  return StatOk;
}

void RTDEF(AllocatableSetBounds)(Descriptor &descriptor, int zeroBasedDim,
    SubscriptValue lower, SubscriptValue upper) {
  INTERNAL_CHECK(zeroBasedDim >= 0 && zeroBasedDim < descriptor.rank());
  if (descriptor.IsAllocatable() && !descriptor.IsAllocated()) {
    descriptor.GetDimension(zeroBasedDim).SetBounds(lower, upper);
    // The byte strides are computed when the object is allocated.
  }
}

void RTDEF(AllocatableSetDerivedLength)(
    Descriptor &descriptor, int which, SubscriptValue x) {
  if (descriptor.IsAllocatable() && !descriptor.IsAllocated()) {
    DescriptorAddendum *addendum{descriptor.Addendum()};
    INTERNAL_CHECK(addendum != nullptr);
    addendum->SetLenParameterValue(which, x);
  }
}

void RTDEF(AllocatableApplyMold)(
    Descriptor &descriptor, const Descriptor &mold, int rank) {
  if (descriptor.IsAllocatable() && !descriptor.IsAllocated()) {
    descriptor.ApplyMold(mold, rank);
  }
}

int RTDEF(AllocatableAllocate)(Descriptor &descriptor,
    std::int64_t *asyncObject, bool hasStat, const Descriptor *errMsg,
    const char *sourceFile, int sourceLine) {
  Terminator terminator{sourceFile, sourceLine};
  if (!descriptor.IsAllocatable()) {
    return ReturnError(terminator, StatInvalidDescriptor, errMsg, hasStat);
  } else if (descriptor.IsAllocated()) {
    return ReturnError(terminator, StatBaseNotNull, errMsg, hasStat);
  } else {
    int stat{ReturnError(
        terminator, descriptor.Allocate(asyncObject), errMsg, hasStat)};
    if (stat == StatOk) {
      if (const DescriptorAddendum * addendum{descriptor.Addendum()}) {
        if (const auto *derived{addendum->derivedType()}) {
          if (!derived->noInitializationNeeded()) {
            stat =
                Initialize(descriptor, *derived, terminator, hasStat, errMsg);
          }
        }
      }
    }
    return stat;
  }
}

int RTDEF(AllocatableAllocateSource)(Descriptor &alloc,
    const Descriptor &source, bool hasStat, const Descriptor *errMsg,
    const char *sourceFile, int sourceLine) {
  int stat{RTNAME(AllocatableAllocate)(
      alloc, /*asyncObject=*/nullptr, hasStat, errMsg, sourceFile, sourceLine)};
  if (stat == StatOk) {
    Terminator terminator{sourceFile, sourceLine};
    if (alloc.rank() != source.rank() && source.rank() != 0) {
      terminator.Crash("ALLOCATE object has rank %d while SOURCE= has rank %d",
          alloc.rank(), source.rank());
    }
    if (int rank{source.rank()}; rank > 0) {
      SubscriptValue allocExtent[maxRank], sourceExtent[maxRank];
      alloc.GetShape(allocExtent);
      source.GetShape(sourceExtent);
      for (int j{0}; j < rank; ++j) {
        if (allocExtent[j] != sourceExtent[j]) {
          if (!hasStat) {
            terminator.Crash("ALLOCATE object has extent %jd on dimension %d, "
                             "but SOURCE= has extent %jd",
                static_cast<std::intmax_t>(allocExtent[j]), j + 1,
                static_cast<std::intmax_t>(sourceExtent[j]));
          }
          return StatInvalidExtent;
        }
      }
    }
    DoFromSourceAssign(alloc, source, terminator);
  }
  return stat;
}

int RTDEF(AllocatableDeallocate)(Descriptor &descriptor, bool hasStat,
    const Descriptor *errMsg, const char *sourceFile, int sourceLine) {
  Terminator terminator{sourceFile, sourceLine};
  if (!descriptor.IsAllocatable()) {
    return ReturnError(terminator, StatInvalidDescriptor, errMsg, hasStat);
  } else if (!descriptor.IsAllocated()) {
    return ReturnError(terminator, StatBaseNull, errMsg, hasStat);
  } else {
    return ReturnError(terminator,
        descriptor.Destroy(
            /*finalize=*/true, /*destroyPointers=*/false, &terminator),
        errMsg, hasStat);
  }
}

int RTDEF(AllocatableDeallocatePolymorphic)(Descriptor &descriptor,
    const typeInfo::DerivedType *derivedType, bool hasStat,
    const Descriptor *errMsg, const char *sourceFile, int sourceLine) {
  int stat{RTNAME(AllocatableDeallocate)(
      descriptor, hasStat, errMsg, sourceFile, sourceLine)};
  if (stat == StatOk) {
    if (DescriptorAddendum * addendum{descriptor.Addendum()}) {
      addendum->set_derivedType(derivedType);
      descriptor.raw().type = derivedType ? CFI_type_struct : CFI_type_other;
    } else {
      // Unlimited polymorphic descriptors initialized with
      // AllocatableInitIntrinsic do not have an addendum. Make sure the
      // derivedType is null in that case.
      INTERNAL_CHECK(!derivedType);
      descriptor.raw().type = CFI_type_other;
    }
  }
  return stat;
}

void RTDEF(AllocatableDeallocateNoFinal)(
    Descriptor &descriptor, const char *sourceFile, int sourceLine) {
  Terminator terminator{sourceFile, sourceLine};
  if (!descriptor.IsAllocatable()) {
    ReturnError(terminator, StatInvalidDescriptor);
  } else if (!descriptor.IsAllocated()) {
    ReturnError(terminator, StatBaseNull);
  } else {
    ReturnError(terminator,
        descriptor.Destroy(
            /*finalize=*/false, /*destroyPointers=*/false, &terminator));
  }
}

// TODO: AllocatableCheckLengthParameter

RT_EXT_API_GROUP_END
}
} // namespace Fortran::runtime
