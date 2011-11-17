//===--- IRGenFunction.cpp - Swift Per-Function IR Generation -------------===//
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
//  This file implements basic setup and teardown for the class which
//  performs IR generation for function bodies.
//
//===----------------------------------------------------------------------===//

#include "llvm/Instructions.h"
#include "llvm/Support/SourceMgr.h"
#include "swift/Basic/SourceLoc.h"

#include "IRGenFunction.h"
#include "IRGenModule.h"

using namespace swift;
using namespace irgen;

IRGenFunction::IRGenFunction(IRGenModule &IGM, FuncExpr *FE, llvm::Function *Fn)
  : IGM(IGM), Builder(IGM.getLLVMContext()), CurFuncExpr(FE), CurFn(Fn) {
}

/// Create an alloca whose lifetime is the duration of the current
/// full-expression.
LValue IRGenFunction::createFullExprAlloca(llvm::Type *Ty, Alignment Align,
                                           const llvm::Twine &Name) {
  assert(AllocaIP && "alloca insertion point has not been initialized!");
  llvm::AllocaInst *Alloca = new llvm::AllocaInst(Ty, Name, AllocaIP);
  Alloca->setAlignment(Align.getValue());

  // TODO: lifetime intrinsics.

  return LValue::forAddress(Alloca, Align);
}

/// Call the llvm.memcpy intrinsic.  The arguments need not already
/// be of i8* type.
void IRGenFunction::emitMemCpy(llvm::Value *dest, llvm::Value *src,
                               Size size, Alignment align) {
  dest = Builder.CreateBitCast(dest, IGM.Int8PtrTy);
  src = Builder.CreateBitCast(src, IGM.Int8PtrTy);

  llvm::Value *args[] = {
    dest,
    src,
    llvm::ConstantInt::get(IGM.SizeTy, size.getValue()),
    llvm::ConstantInt::get(IGM.SizeTy, align.getValue()),
    Builder.getFalse() // volatile
  };

  Builder.CreateCall(IGM.getMemCpyFn(), args);
}                               

/// Create an alloca whose lifetime is the duration of the current
/// scope.
LValue IRGenFunction::createScopeAlloca(llvm::Type *Ty, Alignment Align,
                                        const llvm::Twine &Name) {
  assert(AllocaIP && "alloca insertion point has not been initialized!");
  llvm::AllocaInst *Alloca = new llvm::AllocaInst(Ty, Name, AllocaIP);
  Alloca->setAlignment(Align.getValue());

  // TODO: lifetime intrinsics.

  return LValue::forAddress(Alloca, Align);
}

void IRGenFunction::unimplemented(SourceLoc Loc, StringRef Message) {
  return IGM.unimplemented(Loc, Message);
}
