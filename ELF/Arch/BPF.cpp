//===- BPF.cpp ------------------------------------------------------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "InputFiles.h"
#include "Symbols.h"
#include "Target.h"
#include "lld/Common/ErrorHandler.h"
#include "llvm/Object/ELF.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::support::endian;
using namespace llvm::ELF;
using namespace lld;
using namespace lld::elf;

namespace {
class BPF final : public TargetInfo {
public:
  BPF();
  RelExpr getRelExpr(RelType Type, const Symbol &S,
                     const uint8_t *Loc) const override;
  void relocateOne(uint8_t *Loc, RelType Type, uint64_t Val) const override;
};
} // namespace

BPF::BPF() { 
  NoneRel = R_BPF_NONE;
  RelativeRel = R_BPF_64_RELATIVE;
}

RelExpr BPF::getRelExpr(RelType Type, const Symbol &S,
                        const uint8_t *Loc) const {
  switch (Type) {
    case R_BPF_64_32:
      return R_PC;
    case R_BPF_64_64:
      return R_ABS;
    default:
      error(getErrorLocation(Loc) + "unrecognized reloc " + toString(Type));
      return R_INVALID;
  }
  return R_INVALID;
}

void BPF::relocateOne(uint8_t *Loc, RelType Type, uint64_t Val) const {
  switch (Type) {
    case R_BPF_64_32: {
      // Relocation of a symbol
      write32le(Loc + 4, ((Val - 8) / 8) & 0xFFFFFFFF);
      break;
    }
    case R_BPF_64_64: {  
      // Relocation of a lddw instruction
      // 64 bit address is divided into the imm of this and the following
      // instructions, lower 32 first.
      write32le(Loc + 4, Val & 0xFFFFFFFF);
      write32le(Loc + 8 + 4, Val >> 32);
      break;
    }
    default:
      error(getErrorLocation(Loc) + "unrecognized reloc " + toString(Type));
  }
}

TargetInfo *elf::getBPFTargetInfo() {
  static BPF Target;
  return &Target;
}
