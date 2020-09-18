/*
 *  linux/kernel/traps.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * 'Traps.c' handles hardware traps and faults after we have saved some
 * state in 'asm.s'. Currently mostly a debugging-aid, will be extended
 * to mainly kill the offending process (probably by giving it a signal,
 * but possibly by killing it outright if necessary).
 */
 /*
  * �ڳ��� asm.s �б�����һЩ״̬�󣬱�������������Ӳ������͹��ϡ�Ŀǰ��Ҫ���ڵ���
  * Ŀ�ģ� �Ժ���չ����ɱ���𻵵Ľ��̣���Ҫ��ͨ������һ���źţ��������ҪҲ��ֱ��
  * ɱ���� 
  */
#include <string.h>				// �ַ���ͷ�ļ�����Ҫ������һЩ�й��ڴ���ַ���������Ƕ�뺯��
#include <linux/head.h>		 	// �����˶��������ļ򵥽ṹ���ͼ���ѡ�������
#include <linux/sched.h>		// ���ȳ���ͷ�ļ�������������ṹ task_struct����ʼ����
								// 0�����ݣ�����һЩ�й��������������úͻ�ȡ��Ƕ��ʽ��ຯ�������
#include <linux/kernel.h>		// �ں�ͷ�ļ�������һЩ�ں˳��ú�����ԭ�Ͷ���
#include <asm/system.h>			// ϵͳͷ�ļ������������û��޸�������/�ж��ŵȵ�Ƕ��ʽ����
#include <asm/segment.h>		// �β���ͷ�ļ����������йضμĴ���������Ƕ��ʽ��ຯ��
#include <asm/io.h>				// ����/���ͷ�ļ�������Ӳ���˿�����/����������

// ȡ�� seg �е�ַaddr ����һ���ֽ�
// ���� seg-��ѡ�����addr-����ָ����ַ
// �����%0-eax��_res��;���룺%1-eax��seg����%2-�ڴ��ַ��*��addr������
#define get_seg_byte(seg,addr) ({ \
register char __res; \
__asm__("push %%fs;mov %%ax,%%fs;movb %%fs:%2,%%al;pop %%fs" \
	:"=a" (__res):"0" (seg),"m" (*(addr))); \
__res;})

// ȡ seg �е�ַaddr ����һ�����֣�4�ֽڣ�
// ������seg-��ѡ�����addr-����ָ����ַ
#define get_seg_long(seg,addr) ({ \
register unsigned long __res; \
__asm__("push %%fs;mov %%ax,%%fs;movl %%fs:%2,%%eax;pop %%fs" \
	:"=a" (__res):"0" (seg),"m" (*(addr))); \
__res;})

// ȡfs�μĴ�����ֵ����ѡ�����
#define _fs() ({ \
register unsigned short __res; \
__asm__("mov %%fs,%%ax":"=a" (__res):); \
__res;})

int do_exit(long code);					// �����˳�������kernel/exit.c��

void page_exception(void);				// ҳ�쳣��ʵ����page_fault��mm/page.s��

// ���涨����һЩ�жϳ���ԭ�ͣ������ں���trap_init������������Ӧ�ж�����������
// ��Щ�����Ĵ����ڣ�kernel/asm.s �� system_call.s).
void divide_error(void);				// int0 (kernel/asm.s)
void debug(void);						// int1 (kernel/asm.s)
void nmi(void);							// int2 (kernel/asm.s)
void int3(void);						// int3 (kernel/asm.s)
void overflow(void);					// int4 (kernel/asm.s)
void bounds(void);						// int5 (kernel/asm.s)
void invalid_op(void);					// int6 (kernel/asm.s)
void device_not_available(void);		// int7 (kernel/system_call.s)
void double_fault(void);				// int8 (kernel/asm.s)
void coprocessor_segment_overrun(void);	// int9 (kernel/asm.s)
void invalid_TSS(void);					// int10 (kernel/asm.s)
void segment_not_present(void);			// int11 (kernel/asm.s)
void stack_segment(void);				// int12 (kernel/asm.s)
void general_protection(void);			// int13 (kernel/asm.s)
void page_fault(void);					// int14 (mm/page.s)
void coprocessor_error(void);			// int16 (kernel/system_call.s)
void reserved(void);					// int15 (kernel/asm.s)
void parallel_interrupt(void);			// int39 (kernel/system_call.s)
void irq13(void);						// int45 Э�������жϴ���(kernel/asm.s)

// ���ӳ���������ӡ�����жϵ����ơ�����š����ó����EIP��EFLAGS��ESP��fs�μĴ���ֵ��
// �εĻ��ʡ��εĳ��ȡ����̺š�����š�10�ֽ�ָ���롣�����ջ���û����ݶΣ��򻹴�ӡ16
// �ֽڵĶ�ջ����
static void die(char * str,long esp_ptr,long nr)
{
	long * esp = (long *) esp_ptr;
	int i;

	printk("%s: %04x\n\r",str,nr&0xffff);
	printk("EIP:\t%04x:%p\nEFLAGS:\t%p\nESP:\t%04x:%p\n",
		esp[1],esp[0],esp[2],esp[4],esp[3]);
	printk("fs: %04x\n",_fs());
	printk("base: %p, limit: %p\n",get_base(current->ldt[1]),get_limit(0x17));
	if (esp[4] == 0x17) {
		printk("Stack: ");
		for (i=0;i<4;i++)
			printk("%p ",get_seg_long(0x17,i+(long *)esp[3]));
		printk("\n");
	}
	str(i);
	printk("Pid: %d, process nr: %d\n\r",current->pid,0xffff & i);
	for(i=0;i<10;i++)
		printk("%02x ",0xff & get_seg_byte(esp[1],(i+(char *)esp[0])));
	printk("\n\r");
	do_exit(11);		/* play segment exception */
}

// ������Щ��do_��ͷ�ĺ���ʽasm.s�ж�Ӧ�жϴ��������õ�C����
void do_double_fault(long esp, long error_code)
{
	die("double fault",esp,error_code);
}

void do_general_protection(long esp, long error_code)
{
	die("general protection",esp,error_code);
}

void do_divide_error(long esp, long error_code)
{
	die("divide error",esp,error_code);
}

// �����ǽ����жϺ�˳��ѹ���ջ�ļĴ���ֵ���μ�asm.s
void do_int3(long * esp, long error_code,
		long fs,long es,long ds,
		long ebp,long esi,long edi,
		long edx,long ecx,long ebx,long eax)
{
	int tr;

	__asm__("str %%ax":"=a" (tr):"0" (0));
	printk("eax\t\tebx\t\tecx\t\tedx\n\r%8x\t%8x\t%8x\t%8x\n\r",
		eax,ebx,ecx,edx);
	printk("esi\t\tedi\t\tebp\t\tesp\n\r%8x\t%8x\t%8x\t%8x\n\r",
		esi,edi,ebp,(long) esp);
	printk("\n\rds\tes\tfs\ttr\n\r%4x\t%4x\t%4x\t%4x\n\r",
		ds,es,fs,tr);
	printk("EIP: %8x   CS: %4x  EFLAGS: %8x\n\r",esp[0],esp[1],esp[2]);
}

void do_nmi(long esp, long error_code)
{
	die("nmi",esp,error_code);
}

void do_debug(long esp, long error_code)
{
	die("debug",esp,error_code);
}

void do_overflow(long esp, long error_code)
{
	die("overflow",esp,error_code);
}

void do_bounds(long esp, long error_code)
{
	die("bounds",esp,error_code);
}

void do_invalid_op(long esp, long error_code)
{
	die("invalid operand",esp,error_code);
}

void do_device_not_available(long esp, long error_code)
{
	die("device not available",esp,error_code);
}

void do_coprocessor_segment_overrun(long esp, long error_code)
{
	die("coprocessor segment overrun",esp,error_code);
}

void do_invalid_TSS(long esp,long error_code)
{
	die("invalid TSS",esp,error_code);
}

void do_segment_not_present(long esp,long error_code)
{
	die("segment not present",esp,error_code);
}

void do_stack_segment(long esp,long error_code)
{
	die("stack segment",esp,error_code);
}

void do_coprocessor_error(long esp, long error_code)
{
	if (last_task_used_math != current)
		return;
	die("coprocessor error",esp,error_code);
}

void do_reserved(long esp, long error_code)
{
	die("reserved (15,17-47) error",esp,error_code);
}

// �������쳣�����壩�жϳ����ʼ���ӳ����������ǵ��жϵ����ţ��ж���������
// set_trap_gate() �� set_system_gate������ʹ�����ж���������IDT�е������ţ�trap gate��
// ����֮�����Ҫ��������ǰ�����õ���Ȩ��Ϊ0��������3.��˶ϵ������ж� int3 ������ж�
// ouvrflow �ͱ߽�����ж�bounds�������κγ��������
void trap_init(void)
{
	int i;

	set_trap_gate(0,&divide_error);
	set_trap_gate(1,&debug);
	set_trap_gate(2,&nmi);
	set_system_gate(3,&int3);	/* int3-5 can be called from all */ // int3 - int 5 ���Ա����г���ִ��
	set_system_gate(4,&overflow);
	set_system_gate(5,&bounds);
	set_trap_gate(6,&invalid_op);
	set_trap_gate(7,&device_not_available);
	set_trap_gate(8,&double_fault);
	set_trap_gate(9,&coprocessor_segment_overrun);
	set_trap_gate(10,&invalid_TSS);
	set_trap_gate(11,&segment_not_present);
	set_trap_gate(12,&stack_segment);
	set_trap_gate(13,&general_protection);
	set_trap_gate(14,&page_fault);
	set_trap_gate(15,&reserved);
	set_trap_gate(16,&coprocessor_error);
	// ��int17 - int48 ���������Ⱦ�����Ϊreserved���Ժ��Ӳ���ĳ�ʼ�������������Լ���������
	for (i=17;i<48;i++)
		set_trap_gate(i,&reserved);
	// ����Э�������ж� 0x2d(45) ��������������������������ж��������ò��п��ж���������
	set_trap_gate(45,&irq13);
	outb_p(inb_p(0x21)&0xfb,0x21);					// ���� 8259A ��оƬ�� IRQ2 �ж����� 
	outb(inb_p(0xA1)&0xdf,0xA1);					// ���� 8259A ��оƬ�� IRQ13 �ж�����
	set_trap_gate(39,&parallel_interrupt);			// ���ò��п� 1 ���ж�0x27 ������������
}
