#ifndef ACCMUT_STATISTICS_UTILS_H
#define ACCMUT_STATISTICS_UTILS_H

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <string>
#include <vector>

using namespace llvm;
using namespace std;

class ExecInstNums : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid
  static map<StringRef, int> funcNameID;
  static int curID;

  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  virtual bool runOnFunction(Function &F);

  ExecInstNums(/*Module *M*/);

private:
  Module *TheModule;
};

#endif