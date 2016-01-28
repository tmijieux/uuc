#include "llvm/IR/Verifier.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <cctype>
#include <cstdio>
#include <map>
#include <string>
#include <vector>


int main(int argc, char *argv[])
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::IRBuilder<> Builder(context);
    // générateur de représentation intermédiaire
    
    llvm::Module *module; 
 
    module = new llvm::Module("program", context);
    // un module c'est comme un fichier .c en C

    // je crée une instruction du genre %0 = fadd 11., 3.
    llvm::Value *v = Builder.CreateFAdd( 
	llvm::ConstantFP::get(context, llvm::APFloat(11.)),
	llvm::ConstantFP::get(context, llvm::APFloat(3.))
    ); 


    std::vector<llvm::Type*> Doubles(2, llvm::Type::getDoubleTy(context));
    // un vecteur de type (double, double)
 
    llvm::FunctionType *FT = llvm::FunctionType::get(
	llvm::Type::getDoubleTy(context),
	Doubles, false
    );

    llvm::Function *F = llvm::Function::Create(
	FT,
	llvm::Function::ExternalLinkage,
	"main",
	module
    );
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(
	context,
	"entry",
	F
    );

    Builder.SetInsertPoint(BB);
    // a partir de la les valeurs créé par le Builder seront i
    // insérés dans le Block de Base BB


    Builder.CreateRet(v);
    // termine le block par une instruction return
    // (tous les block doivent se terminer par un return ou par un branchement)

    module->dump();

    return 0;
}
