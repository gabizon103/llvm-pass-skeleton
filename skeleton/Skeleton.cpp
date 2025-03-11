#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/IR/InstIterator.h"

using namespace llvm;

namespace {

struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
    // static char ID;
    // SkeletonPass() : FunctionPass(ID) {}

    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        for (auto& F : M) {
            errs() << "saw a function called " << F.getName() << "!\n";
            std::vector<Instruction *> worklist;
            for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
                worklist.push_back(&*I);
            }
            for (auto I : worklist) {
                if (auto* caller = dyn_cast<CallInst>(I)) {
                    Function* callee = caller->getCalledFunction();
                    if (callee->getInstructionCount() < 8 && isInlineViable(*callee).isSuccess()) {
                        // inline the function if it is short
                        InlineFunctionInfo ifi;
                        InlineFunction(*caller, ifi);
                    }
                }
            }
            
        }
        
        return PreservedAnalyses::none();
    };
};

}

// char SkeletonPass::ID = 0;
// static void registerSkeletonPass(const PassManagerBuilder &, legacy::PassManagerBase &PM) {
//     PM.add(new SkeletonPass());
// }

// static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, registerSkeletonPass);

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Skeleton pass",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(SkeletonPass());
                });
        }
    };
}
