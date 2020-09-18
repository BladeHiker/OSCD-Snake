/*
 *  linux/kernel/asm.s
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * asm.s contains the low-level code for most hardware faults.
 * page_exception is handled by the mm, so that isn't here. This
 * file also handles (hopefully) fpu-exceptions due to TS-bit, as
 * the fpu must be properly saved/resored. This hasn't been tested.
 */
/*
 * asm.s �����а����󲿷ֵ�Ӳ�����ϣ����������ĵײ�δ��롣ҳ�쳣�����ڴ�������
 * mm ����ģ����Բ�������˳��򻹴���ϣ��������������TS-λ����ɵ�fpu �쳣��
 * ��Ϊfpu ������ȷ�ؽ��б���/�ָ�������Щ��û�в��Թ���
 */
 
# �������ļ���Ҫ�漰��Intel �������ж�int0--int16 �Ĵ���int17-int31 �������ʹ�ã���
# ������һЩȫ�ֺ���������������ԭ����traps.c ��˵����
.globl _divide_error,   _debug,   _nmi,    _int3,       _overflow,_bounds,_invalid_op
.globl _double_fault,   _coprocessor_segment_overrun
.globl _invalid_TSS,    _segment_not_present,           _stack_segment
.globl _general_protection,  _coprocessor_error,_irq13, _reserved

# ������γ������޳�������
# int0-- ����������������� ���ͣ����󣻴���ţ���
# ��ִ�� DIV �� IDIV ָ��ʱ���������� 0��CPU �ͻ��������쳣���� EAX ����AX , AL)���ɲ���
# һ���Ϸ��������Ľ��ʱҲ���������쳣����š�_do_divide_error��ʵ������ C ���Ժ���
# ��do_divide_error()�������������ģ���ж�Ӧ�ĺ�����������do_divide_error����trap.c��ʵ��

_divide_error:
	pushl $_do_divide_error		# ���Ȱѽ�Ҫ���õĺ�����ַ��ջ����γ���ĳ����Ϊ0��
no_error_code:					# �������޳���Ŵ������ڴ�
	xchgl %eax,(%esp)			# _do_divide_error �ĵ�ַ -> eax��eax ��������ջ��
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %edi
	pushl %esi
	pushl %ebp
	push %ds					# 16 λ�ĶμĴ�����ջ��ҲҪռ��4 ���ֽڡ�
	push %es
	push %fs
	pushl $0	# "error code"  # ����������ջ��
	lea 44(%esp),%edx			# ȡԭ���÷��ص�ַ����ջָ��λ�ã���ѹ���ջ��
	pushl %edx
	movl $0x10,%edx				# ��ʼ���μĴ������ں˴������ݶ�ѡ�����
	mov %dx,%ds
	mov %dx,%es
	mov %dx,%fs
	call *%eax					# ����C ����do_divide_error()��
	addl $8,%esp				# �൱��ִ������pop�����������������ջ������ C ����������
								# �ö�ջָ������ָ��Ĵ���fs ��ջ����
	pop %fs
	pop %es
	pop %ds
	popl %ebp
	popl %esi
	popl %edi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax					# ����ԭ��eax �е����ݡ�	
	iret

# int1 -- debug �����ж���ڵ㡣�������ͬ�ϡ�
_debug:
	pushl $_do_int3		# _do_debug
	jmp no_error_code

# int2 -- �������жϵ�����ڵ㡣
_nmi:
	pushl $_do_nmi
	jmp no_error_code

# int3 -- �ϵ�ָ�������жϵ���ڵ�
_int3:
	pushl $_do_int3
	jmp no_error_code

# int4 -- ����������ж���ڵ㡣
_overflow:
	pushl $_do_overflow
	jmp no_error_code

# int5 -- �߽�������ж���ڵ㡣
_bounds:
	pushl $_do_bounds
	jmp no_error_code

# int6 -- ��Ч����ָ������ж���ڵ㡣
_invalid_op:
	pushl $_do_invalid_op
	jmp no_error_code

# int9 -- Э�������γ��������ж���ڵ㡣
_coprocessor_segment_overrun:
	pushl $_do_coprocessor_segment_overrun
	jmp no_error_code

# int15 �C ������
_reserved:
	pushl $_do_reserved
	jmp no_error_code

# int45 -- ( = 0x20 + 13 ) ��ѧЭ��������Coprocessor���������жϡ�
# ��Э������ִ����һ������ʱ�ͻᷢ��IRQ13 �ж��źţ���֪ͨCPU ������ɡ�
_irq13:
	pushl %eax
	xorb %al,%al				# 80387 ��ִ�м���ʱ��CPU ��ȴ����������ɡ�
	outb %al,$0xF0				# ͨ��д0xF0 �˿ڣ����жϽ�����CPU ��BUSY �����źţ�������
								# ����80387 �Ĵ�������չ��������PEREQ���ò�����Ҫ��Ϊ��ȷ��
								# �ڼ���ִ��80387 ���κ�ָ��֮ǰ����Ӧ���жϡ�
	movb $0x20,%al
	outb %al,$0x20				# ��8259 ���жϿ���оƬ����EOI���жϽ������źš�
	jmp 1f						# ��������תָ������ʱ���á�
1:	jmp 1f
1:	outb %al,$0xA0				# ����8259 ���жϿ���оƬ����EOI���жϽ������źš�
	popl %eax
	jmp _coprocessor_error		# _coprocessor_error ԭ���ڱ��ļ��У������Ѿ��ŵ�system_call.s��

# �����ж��ڵ���ʱ�����жϷ��ص�ַ֮�󽫳����ѹ���ջ����˷���ʱҲ��Ҫ������ŵ�����


_double_fault:
	pushl $_do_double_fault		# C ������ַ��ջ��
error_code:
	xchgl %eax,4(%esp)		# error code <-> %eax # error code <-> %eax��eax ԭ����ֵ�������ڶ�ջ�ϡ�
	xchgl %ebx,(%esp)		# &function <-> %ebx  # &function <-> %ebx��ebx ԭ����ֵ�������ڶ�ջ�ϡ�
	pushl %ecx
	pushl %edx
	pushl %edi
	pushl %esi
	pushl %ebp
	push %ds
	push %es
	push %fs
	pushl %eax				# error code  # �������ջ��
	lea 44(%esp),%eax		# offset	  # ���򷵻ص�ַ����ջָ��λ��ֵ��ջ��
	pushl %eax
	movl $0x10,%eax			# ���ں����ݶ�ѡ�����
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	call *%ebx				# ������Ӧ��C ���������������ջ��
	addl $8,%esp			# ��ջָ������ָ��ջ�з���fs ���ݵ�λ�á�
	pop %fs
	pop %es
	pop %ds
	popl %ebp
	popl %esi
	popl %edi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	iret

# int10 -- ��Ч������״̬��(TSS)��
_invalid_TSS:
	pushl $_do_invalid_TSS
	jmp error_code

# int11 -- �β����ڡ�
_segment_not_present:
	pushl $_do_segment_not_present
	jmp error_code

# int12 -- ��ջ�δ���
_stack_segment:
	pushl $_do_stack_segment
	jmp error_code

# int13 -- һ�㱣���Գ���
_general_protection:
	pushl $_do_general_protection
	jmp error_code


# int7 -- �豸������(_device_not_available)��(kernel/system_call.s)
# int14 -- ҳ����(_page_fault)��(mm/page.s)
# int16 -- Э����������(_coprocessor_error)��(kernel/system_call.s)
# ʱ���ж�int 0x20 (_timer_interrupt)��(kernel/system_call.s)
# ϵͳ����int 0x80 (_system_call)�ڣ�kernel/system_call.s��
