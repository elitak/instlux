/* real-mode A20 gate control code for grub4dos.
 *
 *  Copyright (C) 2006, 2007 Tinybit <tinybit@tom.com>
 *  Copyright (C) 1991, 1992 Linus Torvalds
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* The code is based on linux/arch/i386/boot/setup.S, so it should be
 * distributed under GPLv2. The setup.S has this copyright notice:
 *
 *      setup.S         Copyright (C) 1991, 1992 Linus Torvalds
 *
 * But it seems many other people have improved the code in setup.S.
 *
 *                   ---- Tinybit <tinybit@tom.com>, August 20, 2007
 */

enable_disable_a20:

	###################################################################
	# Thanks to `A20 - a pain from the past' from url:
	# http://www.win.tue.nl/~aeb/linux/kbd/A20.html
	###################################################################

	###################################################################
	# Thanks to Chris Giese <geezer@execpc.com> for this code:
	# http://my.execpc.com/CE/AC/geezer/osd/boot/a20.asm
	# which is public domain.
	###################################################################

	###################################################################
	# input:	DL=0		disable a20
	#		DL=non-zero	enable a20
	#		DH=0		don't reset keyboard controller
	#		DH=non-zero	reset keyboard controller
	#		CX=loops to try when failure
	#
	# output:	ZF=0		failed
	#		ZF=1		completed ok. If ZF=CF=1, then
        #				the A20 status needn't change and
	#				was not touched.
	#		EAX modified
	#		CX modified
	###################################################################
#if 1
	pushfw				/* save old IF */

	/* oh, in case we are in real mode, disable interrupt since we will
	 * touch the int0 vector.
	 */

	cli

	/* disable CPU cache for the test to work reliably. */
	movl	%cr0, %eax
	pushl	%eax			/* save old cr0 */
//	andl	$0x00000011, %eax
	orl	$0x60000000, %eax	/* set CD and NW */
	movl	%eax, %cr0
	movl	%cr0, %eax
	testl	$0x60000000, %eax	/* check if we can use wbinvd. */
	jz	1f			/* CPU has no wbinvd instruction. */
	wbinvd
	andl	$0xDFFFFFFF, %eax	/* clear NW */
	movl	%eax, %cr0
1:
#endif
3:
	# First, see if the A20 status is already what we desired.

	call	a20_test_match
	/* ZF=1(means equal) for desired and we needn't do anything. */
	stc		/* ZF=CF=1 indicates nothing needs to be done. */
	jz	4f
	//jz	3f

	# Try ELAN before the BIOS call

#if defined(CONFIG_X86_ELAN) || 1

#if 0	// bad! this would hurt some PCs like the DELL machines
	testb	%dl, %dl
	setnz	%al
	shlb	$1, %al
	//movb	$0x02, %al			# alternate A20 gate
	outb	%al, $0x92			# this works on SC410/SC520

	call	a20_test_loop
	jz	3f

#else	// hopefully this wouldn't conflict with other PCs

	# Try another ELAN method: Use "inb 0xee" to control A20.
	# H. Peter Anvin and Christer Weinigel talked about this at(googled):
	# http://www.ussg.iu.edu/hypermail/linux/kernel/0201.0/0128.html

	# Alternate Gate A20 Control Register (Port 00EEh) A special 8-bit
	# read/write control register provides a fast and reliable way to
	# control the CPU A20 signal. A dummy read of this register returns
	# a value of FFh and forces the CPU A20 to propagate to the core
	# logic, while a dummy write to this register will cause the CPU A20
	# signal to be forced Low as long as no other A20 gate control
	# sources are forcing the CPU A20 signal to propagate.
#if 1
	testb	%dl, %dl
	jz	1f
	inb	$0xEE, %al
	jmp	2f
1:
	xorb	%al, %al
	outb	%al, $0xEE
2:
	call	a20_test_loop
	jz	3f
	//jnz	3f

#endif
#endif
#endif

	# Try the BIOS call(INT 0x15, AX=0x240x)

	testb	%dl, %dl
	setnz	%al
	movb	$0x24, %ah
	pushfl					# Be paranoid about flags
	pushal

.ifdef int13_handler
.ifdef ROM_int15
	/* we are inside asm.S */
	pushfw
	lcall	%cs:*(ROM_int15 - int13_handler)
.else
	int	$0x15
.endif
.else
	int	$0x15
.endif

	popal
	popfl

	call	a20_test_match
	jz	3f
	//jnz	3f

	# Try the keyboard controller

	call	empty_8042	# clear output buffer and wait until
				# input buffer is empty	

	call	a20_test_match	# Just in case the BIOS worked
	jz	3f		# but had a delayed reaction.

	pushfw

	cli

	# first, get the keyboard controller output status
	
	movb	$0xD0, %al	# 8042 command byte to read output port
	outb	%al, $0x64	# write command to port 64h
	call	delay
	
	pushw	%cx
	
	xorw	%cx, %cx	# try 65536 times
1:
	inb	$0x64, %al	# get 8042 status
	testb	$1, %al		# output buffer (data _from_ keyboard) full?
	loopz	1b		# no, loop

	popw	%cx

	# wait at least 7 microseconds for MCA type 1 controller
	
	call	delay
	call	delay
	call	delay
	call	delay
	call	delay
	call	delay
	call	delay
	
	# safe to read data
	
	inb	$0x60, %al	# read output port
	andb	$0xFD, %al	# clear bit 1
	testb	%dl, %dl
	setne	%ah
	shlb	$1, %ah
//	movb	$0xDD, %al	# added 2007-10-11
	orb	$1, %al		# added 2007-10-11
	orb	%al, %ah

	call	empty_8042
	
	movb	$0xD1, %al	# 8042 command byte to write output port
	outb	%al, $0x64	# write command to port 64h
	call	delay
	call	empty_8042
	
	movb	%ah, %al	# the value to write: 0xDF for a20 on,
				# and 0xDD for a20 off.
	outb	%al, $0x60

	popfw

#if 0
	call	empty_8042

	/* output a dummy command (USB keyboard hack) */
	movb	$0xff, %al
	outb	%al, $0x64
	call	delay
#endif

	# Wait until a20 really *is* enabled; it can take a fair amount of
	# time on certain systems; Toshiba Tecras are known to have this
	# problem.

	call	empty_8042
	
	call	a20_test_loop
	jz	3f

	# Try the shortcut HP Vectra method

	pushfw

	cli

	# 0xDD is for disable and 0xDF is for enable

	testb	%dl, %dl
	setne	%al
	shlb	$1, %al
	orb	$0xDD, %al
	outb	%al, $0x64

	popfw

	call	empty_8042

	call	a20_test_loop
	jz	3f

	# Final attempt: use "configuration port A"

	inb	$0x92, %al	# Configuration Port A
	testb	%dl, %dl
	jz	2f		# we want to disable a20
	testb	$0x02, %al
	jnz	1f		# chipset bug: do nothing if already set
	orb	$0x02, %al	# "fast A20" version
	andb	$0xFE, %al	# don't accidentally reset the cpu
	jmp	0f
2:
	testb	$0x02, %al
	jz	1f		# chipset bug: do nothing if already cleared
	andb	$0xFC, %al	# don't accidentally reset the cpu
0:
	outb	%al, $0x92
1:
	# Wait for configuration port A to take effect

	call	a20_test_loop
	jz	3f

	# A20 is still not responding.  Try frobbing it again.

	decw	%cx
	jnz	3b

	# right now CX=0, ZF=1

	decw	%cx		# just let ZF=0

	# failure, ZF=0 at this point

3:

#if 1
	pushfw
	
	cli
	
	call	empty_8042

#if 0	
	testb	%dh, %dh
	jz	1f
	
	# try to activate usb keyboard by disable and enable keyboard

	/* 8042 self test */
	movb	$0xAA, %al
	outb	%al, $0x64
	call	delay
	
	call	empty_8042

	/* disable keyboard */
	movb	$0xAD, %al
	outb	%al, $0x64
	call	delay
	
	call	empty_8042

	/* enable keyboard */
	movb	$0xAE, %al
	outb	%al, $0x64
	call	delay
	
	call	empty_8042

	/* write command byte */
	movb	$0x60, %al
	outb	%al, $0x64
	call	delay
	
	call	empty_8042

	movb	$0x45, %al	# enable IRQ1, turn on keyboard
	outb	%al, $0x60

	call	empty_8042

	movb	$0xF4, %al	# enable keyboard
	outb	%al, $0x60

	call	empty_8042

	movb	$0xFF, %al	# restart keyboard
	outb	%al, $0x60

	call	empty_8042

1:
#endif

	/* output a dummy command (USB keyboard hack) */
	/* it may also be a keyboard-reset command according to
	 *   http://www.phys.uu.nl/~0307467/docs/keyboa~2.txt
	 * where it says:
	 *   0FFh	Reset the keyboard and start internal diagnostics
	 */
	movb	$0xff, %al
	outb	%al, $0x64
	call	delay
	
	call	empty_8042
//1:
	popfw
#endif

4:

#if 1
	popl	%eax			/* restore cr0 */
	movl	%eax, %cr0

	lahf				/* Load Flags into AH Register. */
					/* AH = SF:ZF:xx:AF:xx:PF:xx:CF */
	
	popfw				/* restore IF */
	sahf				/* update ZF */
#endif
	ret

a20_test_loop:
	pushw	%cx
	xorw	%cx, %cx
1:
	call	a20_test_match
	jz	2f
	loop	1b
2:
	/* ZF=1(means equal) for match */
	popw	%cx
	ret


a20_test_match:
	call	a20_test
	sete	%al		/* save ZF to AL */
	testb	%dl, %dl
	sete	%ah		/* save ZF to AH */
	cmpb	%al, %ah
	/* ZF=1(means equal) for match */
	ret


# This routine tests whether or not A20 is enabled.  If so, it
# exits with zf = 0.
#

a20_test:
	pushl	%eax
	pushw	%ds
	pushw	%es
	xorw	%ax, %ax
	movw	%ax, %ds		/* DS=0 */
	decw	%ax
	movw	%ax, %es		/* ES=0xFFFF */

	movl	0, %eax
	pushl	%eax			/* save old int0 vector */

	cmpl	%es:0x10, %eax
	jne	1f			/* A20 is on */

	notl	0
	movl	0, %eax
	cmpl	%es:0x10, %eax
	notl	0			/* logical `NOT' won't touch flags */
1:
	/* ZF=0(means not equal) for A20 on, ZF=1(means equal) for A20 off. */

	popl	%eax			/* restore int0 vector */
	movl	%eax, 0
	popw	%es
	popw	%ds

	popl	%eax
	ret

# This routine checks that the keyboard command queue is empty
# (after emptying the output buffers)
#
# Some machines have delusions that the keyboard buffer is always full
# with no keyboard attached...
#
# If there is no keyboard controller, we will usually get 0xff
# to all the reads.  With each IO taking a microsecond and
# a timeout of 100,000 iterations, this can take about half a
# second ("delay" == outb to port 0x80). That should be ok,
# and should also be plenty of time for a real keyboard controller
# to empty.
#

empty_8042:
	pushl	%ecx
	//xorw	%cx, %cx	
	movl	$100000, %ecx	# use 1000 instead of 100000 iterations

3:
	//call	delay

	inb	$0x64, %al	# read 8042 status from port 64h
	testb	$1, %al		# is output buffer(data _from_ keyboard) full?
	jnz	1f		# yes, read it and discard
	testb	$2, %al		# is input buffer(data _to_ keyboard) empty?
	jnz	2f		# no, wait until time out
	jmp	3f		# both input buffer and output buffer are empty
				# success and return
1:
	//call	delay
	inb	$0x60, %al	# read output buffer and discard data/status from 8042
2:
	ADDR32 loop	3b

	# timed out and failure, return 
3:
	popl	%ecx
	ret

# Delay is needed after doing I/O
delay:
//	outb	%al, $0x80
	pushw	%cx
	movw	$0x0800, %cx	#; 0x2000
1:
	loop	1b
	popw	%cx
	ret

