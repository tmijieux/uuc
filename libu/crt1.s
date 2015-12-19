	.globl _start
_start:                 
	xor    %ebp,%ebp
	mov    %rdx,%r9
	pop    %rsi
	mov    %rsp,%rdx
	and    $0xfffffffffffffff0,%rsp
	push   %rax
	push   %rsp
	mov    $__libc_csu_fini,%r8
	mov    $__libc_csu_init,%rcx
	mov    $__start.main,%rdi
	callq  __libc_start_main
	hlt

	        
