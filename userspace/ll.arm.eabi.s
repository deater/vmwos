#
#  linux_logo in ARM 32-bit EABI assembler 0.46
#
#  Originally by
#       Vince Weaver <vince _at_ deater.net>
#
#  assemble with     "as -o ll.arm.eabi.o ll.arm.eabi.s"
#  link with         "ld -o ll.arm.eabi ll.arm.eabi.o"

.include "logo.include"

@ Optimization
@	1186 -- v0.46 starting point
@	1185 -- move logo to beginning of data segment
@	1181 -- have ldm auto-set r3 to logo
@	1177 -- use pc-relative to set addresses
@	1173 -- use 0x8000 instead of 0xff00 for sentinel
@	1165 -- mask by shifting
@	1161 -- avoid unnecessary copy

# Syscalls:    New EABI way, syscall num is in r7, do a "swi 0"
#              for EABI you need kernel support and gcc > 4.0.0?

# ARM has 16 registers
#  (more visible in system mode: r0-r7 are unbanked, always the same.
#   r8-r14 change depending on system status)
# When writing user programs
# + r0-r12 are general purpose
# + r13 = stack pointer
# + r14 = link register
# + r15 = program counter
# reading r15 in general gives you current PC+8 (exceptions for STM and STR)
# 6 Status registers (only one visible in userspace)
# - NZCVQ (Negative, Zero, Carry, oVerflow, saturate)
# prefix most instructions to handle the condition codes:
# EQ, NE (equal/not equal)  CS, CC (carry set/clear)
# MI, PL (minus/plus)       VS, VC (overflow set/clear)
# HI, LS (unsigned higer/lowersame)
# GE, LT (greaterequal,less than)
# GT, LE (greater than, lessthanequal)
# AL (always)
#
# comment character is a @
# gas supports "nop"=mov r0,r0
#   ldr = load register
#   adr reg,label = load label into addr (via pc-relative add or sub)
#   adrl = like above, but always an 8-byte instr
# every instruction has a condition code
#
# THUMB instructions: see the separate ll.thumb.s
#
# ALU ops can also do a shift (no dedicated shift instr)
#   Shift goes last.  LSL, LSR (logical shift left/right)
#                     ASR (arithmatic shift right)
#		      ROR (rotate right)
#		      RRX (rotate right sign extend)
#   Carry out is the last value shifted out, or C if no value shifted
#

# load store addressing modes for load word/store word/ unsigned byte
#   [ r , #+/- 12bitoffset]            = load as expected
#   [ r , +/- reg         ]            = load as expected
#   [ r , +/- reg, shift #shift amt ]  = load as expected
#   [ r , #+/- 12bitoffset]!           = pre-index / load+off, then if CC then write back new index
#   [ r , +/- reg         ]!           = pre-index / load+off, then if CC then write back new index
#   [ r , +/- reg, shift #shift amt]!  = pre-index / load+off, then if CC then write back new index
#   [ r ], #+/- 12bitofset             = post-index / load, then if CC write back base+off
#   [ r ] , +/- reg                    = post-index / load, then if CC write back base+off
#   [ r ] , +/- reg, shift #shift amt  = post-index / load, then if CC write back base+off

# for halfword, signed bytes you only get an 8bit offset and some other caveats

# Constants are 8 bits, optionally shifted left by an even amount

# The PC is a gp register and can be written too by any ALU op

# multiply with accumulate option, mul two numbers together, add in third, store to 4th

# no support for unaligned memory access

# Addressing modes include pre/post incrememnt that can
#  save back the updated address to a register
# Also, multiple registers worth of data can be read/stored at once

# offsets into the results returned by the uname syscall
.equ U_SYSNAME,0
.equ U_NODENAME,65
.equ U_RELEASE,65*2
.equ U_VERSION,(65*3)
.equ U_MACHINE,(65*4)
.equ U_DOMAINNAME,65*5

# offset into the results returned by the sysinfo syscall
.equ S_TOTALRAM,16

# Sycscalls
.equ SYSCALL_EXIT,	1
.equ SYSCALL_READ,	3
.equ SYSCALL_WRITE,	4
.equ SYSCALL_OPEN,	5
.equ SYSCALL_CLOSE,	6
.equ SYSCALL_SYSINFO,	116
.equ SYSCALL_UNAME,	122

#
.equ STDIN,	0
.equ STDOUT,	1
.equ STDERR,	2

	.globl	_start
_start:

	#=========================
	# PRINT LOGO
	#=========================

# LZSS decompression algorithm implementation
# by Stephan Walter 2002, based on LZSS.C by Haruhiko Okumura 1989
# optimized some more by Vince Weaver

	@ r1 = out_addr (buffer we are printing to)
	@ r2 = N-F (R)
	@ r3 = logo data
	@ r8 = logo end
	@ r9 = text_buf_addr
	@ r11 = data_begin
	@ r12 = bss_begin

#	adr	r3,addresses
#	ldmia	r3!,{r1,r2,r8,r9,r11,r12}


@ UGH: no assembler support for PIC, have to do it by hand

	ldr	r11,[pc,#12]
	add	r11,r11,pc
	ldr	r12,[pc,#8]
	add	r12,r12,pc
	b	done_pic

data_offset:	.word (data_begin - _start - 12)
bss_offset:	.word (bss_begin - _start - 20)
out_offset:	.word (out_buffer-bss_begin)
logo_offset:	.word (logo-data_begin)
done_pic:

	ldr	r1,[pc,#-16]	@ out buffer
	add	r1,r12,r1

	ldr	r3,[pc,#-20]	@ logo
	add	r3,r11,r3

	add	r8,r11,#(logo_end-data_begin)
	add	r9,r12,#(text_buf-bss_begin)
	ldr	r2,=(N-F)

	str	r1,[r12,#(out_addr-bss_begin)]

@	push	{r0-r12}

@	mov	r0,#0
@	mov	r3,r1
@	bl	num_to_ascii

@	pop	{r0-r12}


decompression_loop:
	ldrb	r4,[r3],#+1		@ load a byte, increment pointer/

	orr 	r5,r4,#0x8000	@ set bit 15 in the byte we loaded

test_flags:
	cmp	r3,r8		@ have we reached the end?
	bge	done_logo  	@ if so, exit

	lsrs 	r5,#1		@ shift bottom bit into carry flag

discrete_char:
	ldrcsb	r4,[r3],#+1		@ load a byte, increment pointer
	movcs	r6,#1			@ we set r6 to one so byte
					@ will be output once

	bcs	store_byte		@ and store it


offset_length:
	ldrb	r0,[r3],#+1	@ load a byte, increment pointer
	ldrb	r7,[r3],#+1	@ load a byte, increment pointer
				@ we can't load halfword as no unaligned loads on arm

	orr	r7,r0,r7,LSL #8	@ merge back into 16 bits
				@ this has match_length and match_position

				@ no need to mask r7, as we do it
				@ by default in output_loop

	mov	r0,#(THRESHOLD+1)
	add	r6,r0,r7,LSR #(P_BITS)
				@ r6 = (r7 >> P_BITS) + THRESHOLD + 1
				@                       (=match_length)

output_loop:

	lsl	r7,#22			@ mask by shifting
	lsr	r7,#22

	ldrb 	r4,[r9,r7]		@ load byte from text_buf[]
	add	r7,r7,#1		@ advance pointer in text_buf

store_byte:
	strb	r4,[r1],#+1		@ store a byte, increment pointer
	strb	r4,[r9,r2]		@ store a byte to text_buf[r]
	add 	r2,r2,#1		@ r++

	lsl	r2,#22			@ mask by shifting
	lsr	r2,#22

	subs	r6,r6,#1		@ decement count
	bne 	output_loop		@ repeat until k>j

	cmp	r5,#0xff		@ are the top bits 0?
	bgt	test_flags		@ if not, re-load flags

	b	decompression_loop



# end of LZSS code

done_logo:

@	ldr	r1,=out_buffer		@ buffer we are printing to
	ldr	r1,[r12,#(out_addr-bss_begin)]

	bl	write_stdout		@ print the logo

	#==========================
	# PRINT VERSION
	#==========================
first_line:

	add	r0,r12,#(uname_info-bss_begin)	@ uname struct
	mov	r7,#SYSCALL_UNAME
	swi	0x0		 		@ do syscall

	add	r1,r12,#(uname_info-bss_begin)
						@ os-name from uname "Linux"

@	ldr	r10,out_addr			@ point r10 to out_buffer
	ldr	r10,[r12,#(out_addr-bss_begin)]


	bl	strcat				@ call strcat


	add	r1,r11,#(ver_string-data_begin) @ source is " Version "
	bl 	strcat			        @ call strcat

	add	r1,r12,#((uname_info-bss_begin)+U_RELEASE)
						@ version from uname, ie "2.6.20"
	bl	strcat				@ call strcat

	add	r1,r11,#(compiled_string-data_begin)
						@ source is ", Compiled "
	bl	strcat				@  call strcat

	@VMWOS	add	r1,r12,#((uname_info-bss_begin)+U_VERSION)
	add	r1,r12,#((uname_info-bss_begin))
	add	r1,r1,#U_VERSION
						@ compiled date
	bl	strcat				@ call strcat

	mov	r3,#0xa
	strb	r3,[r10],#+1		@ store a linefeed, increment pointer
	strb	r0,[r10],#+1		@ NUL terminate, increment pointer

	bl	center_and_print	@ center and print

	@===============================
	@ Middle-Line
	@===============================
middle_line:
	@=========
	@ Load /proc/cpuinfo into buffer
	@=========

@	ldr	r10,out_addr		@ point r10 to out_buffer
	ldr	r10,[r12,#(out_addr-bss_begin)]

	add	r0,r11,#(cpuinfo-data_begin)
					@ '/proc/cpuinfo'
	mov	r1,#0			@ 0 = O_RDONLY <bits/fcntl.h>
	mov	r7,#SYSCALL_OPEN
	swi	0x0
					@ syscall.  return in r0?
	mov	r5,r0			@ save our fd
	add	r1,r12,#(disk_buffer-bss_begin)
	mov	r2,#4096
				 	@ 4096 is maximum size of proc file ;)
	mov	r7,#SYSCALL_READ
	swi	0x0

	mov	r0,r5
	mov	r7,#SYSCALL_CLOSE
	swi	0x0
					@ close (to be correct)


	@=============
	@ Number of CPUs
	@=============
number_of_cpus:

	add	r1,r11,#(one-data_begin)
					# cheat.  Who has an SMP arm?
					# 2012 calling, my pandaboard is...
	bl	strcat

	@=========
	@ MHz
	@=========
print_mhz:

	@ the arm system I have does not report MHz

	@=========
	@ Chip Name
	@=========
chip_name:
	mov	r0,#'a'
	mov	r1,#'r'
	mov	r2,#'e'
	mov	r3,#'\n'
	bl	find_string
					@ find 'sor\t: ' and grab up to ' '

	add	r1,r11,#(processor-data_begin)
					@ print " Processor, "
	bl	strcat

	@========
	@ RAM
	@========

	add	r0,r12,#(sysinfo_buff-bss_begin)
	mov	r7,#SYSCALL_SYSINFO
	swi	0x0
					@ sysinfo() syscall

	ldr	r3,[r12,#((sysinfo_buff-bss_begin)+S_TOTALRAM)]
					@ size in bytes of RAM
	movs	r3,r3,lsr #20		@ divide by 1024*1024 to get M
	adc	r3,r3,#0		@ round

	mov	r0,#1
	bl 	num_to_ascii

	add	r1,r11,#(ram_comma-data_begin)
					@ print 'M RAM, '
	bl	strcat			@ call strcat


	@========
	@ Bogomips
	@========

	mov	r0,#'I'
	mov	r1,#'P'
	mov	r2,#'S'
	mov	r3,#'\n'
	bl	find_string

	add	r1,r11,#(bogo_total-data_begin)
	bl	strcat			@ print bogomips total

	bl	center_and_print	@ center and print

	#=================================
	# Print Host Name
	#=================================
last_line:
@	ldr	r10,out_addr		@ point r10 to out_buffer
	ldr	r10,[r12,#(out_addr-bss_begin)]


	add	r1,r12,#((uname_info-bss_begin)+U_NODENAME)
					@ host name from uname()
	bl	strcat			@ call strcat

	bl	center_and_print	@ center and print

	add	r1,r11,#(default_colors-data_begin)
					@ restore colors, print a few linefeeds
	bl	write_stdout


	@================================
	@ Exit
	@================================
exit:
	mov	r0,#0				@ result is zero
	mov	r7,#SYSCALL_EXIT
	swi	0x0				@ and exit


	@=================================
	@ FIND_STRING
	@=================================
	@ r0,r1,r2 = string to find
	@ r3 = char to end at
	@ r5 trashed
find_string:
	add	r7,r12,#(disk_buffer-bss_begin)
@	ldr	r7,=disk_buffer		@ look in cpuinfo buffer
find_loop:
	ldrb	r5,[r7],#+1		@ load a byte, increment pointer
	cmp	r5,r0			@ compare against first byte
	ldrb	r5,[r7]			@ load next byte
	cmpeq	r5,r1			@ if first byte matched, comp this one
	ldrb	r5,[r7,#+1]		@ load next byte
	cmpeq	r5,r2			@ if first two matched, comp this one
	beq	find_colon		@ if all 3 matched, we are found

	cmp	r5,#0			@ are we at EOF?
	beq	done			@ if so, done

	b	find_loop

find_colon:
	ldrb	r5,[r7],#+1		@ load a byte, increment pointer
	cmp	r5,#':'
	bne	find_colon		@ repeat till we find colon

	add	r7,r7,#1		@ skip the space

store_loop:
	ldrb	r5,[r7],#+1		@ load a byte, increment pointer
	strb	r5,[r10],#+1		@ store a byte, increment pointer
	cmp	r5,r3
	bne	store_loop

almost_done:
	mov	r0,#0
	strb	r0,[r10],#-1		@ replace last value with NUL

done:
	mov	pc,lr			@ return

	#================================
	# strcat
	#================================
	# value to cat in r1
	# output buffer in r10
	# r3 trashed
strcat:
	ldrb	r3,[r1],#+1		@ load a byte, increment pointer
	strb	r3,[r10],#+1		@ store a byte, increment pointer
	cmp	r3,#0			@ is it zero?
	bne	strcat			@ if not loop
	sub	r10,r10,#1		@ point to one less than null
	mov	pc,lr			@ return


	#==============================
	# center_and_print
	#==============================
	# string to center in at output_buffer

center_and_print:

	stmfd	SP!,{LR}		@ store return address on stack

	add	r1,r11,#(escape-data_begin)
					@ we want to output ^[[
	bl	write_stdout

str_loop2:
@	ldr	r2,out_addr		@ point r2 to out_buffer
	ldr	r2,[r12,#(out_addr-bss_begin)]

	sub	r2,r10,r2		@ get length by subtracting

	rsb	r2,r2,#81		@ reverse subtract!  r2=81-r2
					@ we use 81 to not count ending \n

	bne	done_center		@ if result negative, don't center

	lsrs	r3,r2,#1		@ divide by 2
	adc	r3,r3,#0		@ round?

	mov	r0,#0			@ print to stdout
	bl	num_to_ascii		@ print number of spaces

	add	r1,r11,#(C-data_begin)
					@ we want to output C
	bl	write_stdout

done_center:
@	ldr	r1,out_addr		@ point r1 to out_buffer
	ldr	r1,[r12,#(out_addr-bss_begin)]

	ldmfd	SP!,{LR}		@ restore return address from stack

	#================================
	# WRITE_STDOUT
	#================================
	# r1 has string
	# r0,r2,r3 trashed
write_stdout:
	mov	r2,#0				@ clear count

str_loop1:
	add	r2,r2,#1
	ldrb	r3,[r1,r2]
	cmp	r3,#0
	bne	str_loop1			@ repeat till zero

write_stdout_we_know_size:
	mov	r0,#STDOUT			@ print to stdout
	mov	r7,#SYSCALL_WRITE
	swi	0x0		 		@ run the syscall
	mov	pc,lr				@ return


	@#############################
	@ num_to_ascii
	@#############################
	@ r3 = value to print
	@ r0 = 0=stdout, 1=strcat

num_to_ascii:
	stmfd	SP!,{r10,LR}		@ store return address on stack
	add	r10,r12,#((ascii_buffer-bss_begin))
	add	r10,r10,#10
					@ point to end of our buffer

div_by_10:
	@================================================================
	@ Divide by 10 - because ARM has no hardware divide instruction
	@    the algorithm multiplies by 1/10 * 2^32
	@    then divides by 2^32 (by ignoring the low 32-bits of result)
	@================================================================
	@ r3=numerator
	@ r7=quotient    r8=remainder
	@ r5=trashed
divide_by_10:
	ldr	r4,=429496730			@ 1/10 * 2^32
	sub	r5,r3,r3,lsr #30
	umull	r8,r7,r4,r5			@ {r8,r7}=r4*r5

	mov	r4,#10				@ calculate remainder

						@ could use "mls" on
						@ armv6/armv7
	mul	r8,r7,r4
	sub	r8,r3,r8

	@ r7=Q, R8=R

	add	r8,r8,#0x30	@ convert to ascii
	strb	r8,[r10],#-1	@ store a byte, decrement pointer
	adds	r3,r7,#0	@ move Q in for next divide, update flags
	bne	div_by_10	@ if Q not zero, loop


write_out:
	add	r1,r10,#1	@ adjust pointer
	ldmfd	SP!,{r10,LR}	@ restore return address from stack

	cmp	r0,#0
	bne	strcat		@ if 1, strcat

	b write_stdout		@ else, fallthrough to stdout


literals:
# Put literal values here
.ltorg




#===========================================================================
#	section .data
#===========================================================================
.data
.include	"logo.lzss_new"
data_begin:
ver_string:	.ascii	" Version \0"
compiled_string:	.ascii	", Compiled \0"
processor:	.ascii	" Processor, \0"
ram_comma:	.ascii	"M RAM, \0"
bogo_total:	.ascii	" Bogomips Total\n\0"

default_colors:	.ascii "\033[0m\n\n\0"
escape:		.ascii "\033[\0"
C:		.ascii "C\0"

.ifdef FAKE_PROC
cpuinfo:	.ascii  "proc/cpui.arm\0"
.else
cpuinfo:	.ascii	"/proc/cpuinfo\0"
.endif

one:	.ascii	"One \0"





#============================================================================
#	section .bss
#============================================================================

# Note, vmwOS doesn't really support BSS yet, need to use an actual
# executable format

#.bss

@ UGH if not aligned and we have alignment disabled
@ the ARM chip will just do fun things like swap bytes when writing
@ fun to debug
.align 4
bss_begin:
out_addr:	.word	0

#.lcomm	ascii_buffer,10
ascii_buffer:
.rept 10
.byte 0
.endr

.align 4
#.lcomm	sysinfo_buff,(64)
sysinfo_buff:
.rept 64
.byte 0
.endr

.align 4
#.lcomm	uname_info,(65*6)
uname_info:
.rept 65*6
.byte 0
.endr

.align 4
#.lcomm	text_buf, (N+F-1)
text_buf:
.rept (N+F-1)
.byte 0
.endr

.align 4
#.lcomm	disk_buffer,4096	@ we cheat!!!!
disk_buffer:
.rept 4096
.byte 0
.endr

#.lcomm	out_buffer,16384
out_buffer:
.rept 16384
.byte 0
.endr

	# see /usr/src/linux/include/linux/kernel.h

