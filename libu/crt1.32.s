	.globl _start
_start:
	xor    %ebp,%ebp
	pop    %esi
	mov    %esp,%ecx
	and    $0xfffffff0,%esp
	push   %eax
	push   %esp
	push   %edx
	push   $__libc_csu_fini
	push   $__libc_csu_init
	push   %ecx
	push   %esi
	push   $__start.main
	call   __libc_start_main
	hlt
