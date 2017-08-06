.extern jit
.text
	.global jit_and_jump
	.global outer_jit_wrapper

# Since we don't touch the stack during execution we can use it to return at
# any point and just jump to the same finish code

# RDI is the virtual memory location
outer_jit_wrapper:
	sub $0x8, %rsp #The return address from the caller causes us to be between boundries
	push %r12
	push %r13
	push %r14
	push %r15
	mov $0xFF, %r10 # Stack pointer
	mov $0x20, %r11 #Set the always bit

jit_and_jump:
	push %r10
	push %r11

	call jit

	pop %r11
	pop %r10

	cmp $0, %rax
	je done
	jmp *%rax

done:
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	add $0x8, %rsp
	ret
