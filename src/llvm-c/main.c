#include <stdio.h>
#include <stdlib.h>

#include <llvm-c/Core.h>

int main(int argc, char *argv[])
{
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMTypeRef float_type, fun_type;
    LLVMValueRef a, b, c, fun;
    LLVMBasicBlockRef bb;
    LLVMTypeRef args[2];
    
    module = LLVMModuleCreateWithName("prog");
    builder = LLVMCreateBuilder();
    float_type = LLVMFloatType();
    a = LLVMConstReal(float_type, 3.);
    b = LLVMConstReal(float_type, 11.);
    c = LLVMBuildFAdd(builder, a, b, "tmpadd");

    args[0] = float_type;
    args[1] = float_type;
   
    fun_type =  LLVMFunctionType(float_type, args, 0, 0);
    fun = LLVMAddFunction(module, "main", fun_type);
    bb = LLVMAppendBasicBlock(fun, "entry");
    LLVMPositionBuilderAtEnd(builder, bb);
    LLVMBuildRet(builder, c);
    
    //LLVMDumpModule(module);
    printf("%s", LLVMPrintModuleToString(module));

    LLVMDisposeModule(module);
    return 0;
}
