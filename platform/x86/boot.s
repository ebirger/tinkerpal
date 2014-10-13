# Multiboot magic tricks
.set ALIGN, 1<<0
.set MEMINFO, 1<<1
.set FLAGS, ALIGN | MEMINFO
.set MAGIC, 0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

# Multiboot header
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Entry point
.section .text
.global _start
.type _start, @function
_start:
	# Setup stack
	movl $_stack_top, %esp

        # XXX: zero BSS

	# Go to C entry point
	call kernel_main

	# We should never get here
	cli
	hlt
.hang:
	jmp .hang
