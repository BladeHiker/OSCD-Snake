/*
 *  linux/kernel/keyboard.S
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 *	Thanks to Alfred Leung for US keyboard patches
 *		Wolfgang Thiel for German keyboard patches
 *		Marc Corsini for the French keyboard
 */
/*
 * ��лAlfred Leung ������US ���̲�������
 * Wolfgang Thiel �����˵�����̲�������
 * Marc Corsini �����˷��ļ��̲�������
 */

#include <linux/config.h>				// �ں�����ͷ�ļ�������������Ժ�Ӳ�����ͣ�HD_TYPE����ѡ�

.text
.globl _keyboard_interrupt

/*
 * these are for the keyboard read functions
 */
/*
 * ������Щ�����ڼ��̶�������
 */
 
 // size �Ǽ��̻������ĳ��ȣ��ֽ�������
size	= 1024		/* must be a power of two ! And MUST be the same
			   			as in tty_io.c !!!! */
			   		/* ��ֵ������2 �Ĵη���������tty_io.c �е�ֵƥ��!!!! */
			   		
// ������Щ�ǻ�����нṹ�е�ƫ���� */
head = 4 				# ��������ͷָ���ֶ�ƫ�ơ�
tail = 8 				# ��������βָ���ֶ�ƫ�ơ�
proc_list = 12 			# �ȴ��û�����еĽ����ֶ�ƫ�ơ�
buf = 16 				# �������ֶ�ƫ�ơ�

// mode �Ǽ���������İ���״̬��־��
// ��ʾ��Сдת����(caps)��������(alt)�����Ƽ�(ctrl)�ͻ�����(shift)��״̬��
// λ7 caps �����£�
// λ6 caps ����״̬(Ӧ����leds �еĶ�Ӧ��־λһ��)��
// λ5 ��alt �����£�
// λ4 ��alt �����£�
// λ3 ��ctrl �����£�
// λ2 ��ctrl �����£�
// λ1 ��shift �����£�
// λ0 ��shift �����¡�
mode: .byte 0 		/* caps, alt, ctrl and shift mode */
// ����������(num-lock)����Сдת����(caps-lock)�͹���������(scroll-lock)��LED �����״̬��
// λ7-3 ȫ0 ���ã�
// λ2 caps-lock��
// λ1 num-lock(��ʼ��1��Ҳ����������������(num-lock)�����Ϊ��)��
// λ0 scroll-lock��
leds: .byte 2 		/* num-lock, caps, scroll-lock mode (nom-lock on) */
// ��ɨ������0xe0 ��0xe1 ʱ���øñ�־����ʾ��󻹸�����1 ����2 ���ַ�ɨ���롣
// λ1 =1 �յ�0xe1 ��־��
// λ0 =1 �յ�0xe0 ��־��
e0:	.byte 0

/*
 *  con_int is the real interrupt routine that reads the
 *  keyboard scan-code and converts it into the appropriate
 *  ascii character(s).
 */
/*
 * con_int ��ʵ�ʵ��жϴ����ӳ������ڶ�����ɨ���벢����ת��
 * ����Ӧ��ascii �ַ���
 */
 //// �����жϴ���������ڵ㡣
_keyboard_interrupt:
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	movl $0x10,%eax					# ��ds��es �μĴ�����Ϊ�ں����ݶΡ�
	mov %ax,%ds
	mov %ax,%es
	xor %al,%al		/* %eax is scan code */ /* eax ��ɨ����*/
	inb $0x60,%al
	cmpb $0xe0,%al
	je set_e0
	cmpb $0xe1,%al
	je set_e1
	call *key_table(,%eax,4)		# ���ü���������ker_table + eax * 4
	movb $0,e0						# ��λe0 ��־��
	// ������δ��������ʹ�� 8255A �� PC ��׼���̵�·����Ӳ����λ�������˿� 0x61 ��
	// 8255A ����� B �ĵ�ַ��������˿ڵĵ� 7 λ(PB7)���ڽ�ֹ�������Լ������ݵĴ�����
	// ��γ������ڶ��յ���ɨ��������Ӧ�𡣷��������Ƚ�ֹ���̣�Ȼ�����������������̹�����

e0_e1: inb $0x61,%al 				# ȡPPI �˿� B ״̬����λ7 ��������/��ֹ(0/1)���̡�
	jmp 1f							# ��ʱ
1:	jmp 1f
1:	orb $0x80,%al					# al λ7 ��λ(��ֹ���̹���)
	jmp 1f							#  ��ʱ
1:	jmp 1f
1:	outb %al,$0x61					# ʹPPI PB7 λ��λ��
	jmp 1f							#  ��ʱ	
1:	jmp 1f
1:	andb $0x7F,%al					# al λ7 ��λ��
	outb %al,$0x61					# ʹPPI PB7 λ��λ���������̹�������
	movb $0x20,%al					# ��8259 �ж�оƬ����EOI(�жϽ���)�źš�
	outb %al,$0x20
	pushl $0						# ����̨tty ��=0����Ϊ������ջ��
	call _do_tty_interrupt			# ���յ������ݸ��Ƴɹ淶ģʽ���ݲ�����ڹ淶�ַ���������С�
	addl $4,%esp					# ������ջ�Ĳ��������������ļĴ��������жϷ��ء�
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	iret
set_e0:	movb $1,e0					# �յ�ɨ��ǰ����0xe0 ʱ������e0 ��־��λ0����
	jmp e0_e1
set_e1:	movb $2,e0					# �յ�ɨ��ǰ����0xe1 ʱ������e1 ��־��λ1����
	jmp e0_e1

/*
 * This routine fills the buffer with max 8 bytes, taken from
 * %ebx:%eax. (%edx is high). The bytes are written in the
 * order %al,%ah,%eal,%eah,%bl,%bh ... until %eax is zero.
 */
/*
 * ������ӳ����ebx:eax �е����8 ���ַ����뻺������С�(edx ��
 * ��д���ַ���˳����al,ah,eal,eah,bl,bh...ֱ��eax ����0��
 */
put_queue:
	pushl %ecx						# ����ecx��edx ���ݡ�
	pushl %edx						# ȡ����̨tty �ṹ�ж��������ָ�롣
	movl _table_list,%edx			# read-queue for console
	movl head(%edx),%ecx			# ȡ���������ͷָ��->ecx��
1:	movb %al,buf(%edx,%ecx)			# ��al �е��ַ����뻺�����ͷָ��λ�ô���
	incl %ecx						# ͷָ��ǰ��1 �ֽڡ�
	andl $size-1,%ecx				# �Ի�������С����ͷָ��(�������򷵻ػ�������ʼ)��
	cmpl tail(%edx),%ecx			# buffer full - discard everything
									# ͷָ��==βָ����(���������)��
	je 3f							# ��������������δ������ַ�ȫ������
	shrdl $8,%ebx,%eax				# ��ebx ��8 λ����λ����8 λ��eax �У���ebx ���䡣
	je 2f							# �����ַ�����û��(����0)����ת��
	shrl $8,%ebx					# ��ebx �б���λ����8 λ������ת�����1 ����������
	jmp 1b
2:	movl %ecx,head(%edx)			# ���ѽ������ַ��������˶��У��򱣴�ͷָ�롣
	movl proc_list(%edx),%ecx		# �ö��еĵȴ�����ָ�룿
	testl %ecx,%ecx					# �������ṹָ���Ƿ�Ϊ��(�еȴ��ö��еĽ�����)��
	je 3f							# �ޣ�����ת��
	movl $0,(%ecx)					# �У����øý���Ϊ�����о���״̬(���Ѹý���)��
3:	popl %edx						# ���������ļĴ��������ء�
	popl %ecx
	ret

// ������δ������ctrl ��alt ��ɨ���룬�ֱ�����ģʽ��־����Ӧλ�������ɨ����֮ǰ�յ���
// 0xe0 ɨ����(e0 ��־��λ)����˵�����µ��Ǽ����ұߵ�ctrl ��alt �������Ӧ����ctrl ��alt
// ��ģʽ��־mode �еı���λ��
ctrl:	movb $0x04,%al				# 0x4 ��ģʽ��־mode ����ctrl ����Ӧ�ı���λ(λ2)��
	jmp 1f
alt:	movb $0x10,%al				# 0x10 ��ģʽ��־mode ����alt ����Ӧ�ı���λ(λ4)��
1:	cmpb $0,e0						# e0 ��־��λ����(���µ����ұߵ�ctrl ��alt ����)��
	je 2f							# ������ת��
	addb %al,%al					# �ǣ���ĳ�����Ӧ�Ҽ��ı�־λ(λ3 ��λ5)��
2:	orb %al,mode					# ����ģʽ��־mode �ж�Ӧ�ı���λ��
	ret
// ��δ��봦��ctrl ��alt ���ɿ���ɨ���룬��Ӧ��λģʽ��־mode �еı���λ���ڴ���ʱҪ����
// e0 ��־�Ƿ���λ���ж��Ƿ��Ǽ����ұߵ�ctrl ��alt ����
unctrl:	movb $0x04,%al				# ģʽ��־mode ����ctrl ����Ӧ�ı���λ(λ2)��
	jmp 1f
unalt:	movb $0x10,%al				# 0x10 ��ģʽ��־mode ����alt ����Ӧ�ı���λ(λ4)��
1:	cmpb $0,e0						# e0 ��־��λ����(�ͷŵ����ұߵ�ctrl ��alt ����)��
	je 2f							# ���ǣ���ת��
	addb %al,%al
2:	notb %al
	andb %al,mode
	ret

lshift:
	orb $0x01,mode 					# ����shift �����£�����mode �ж�Ӧ�ı�־λ(λ0)��
	ret
unlshift:
	andb $0xfe,mode 				# ����shift ���ɿ�����λmode �ж�Ӧ�ı�־λ(λ0)��
	ret
rshift:
	orb $0x02,mode 					# ����shift �����£�����mode �ж�Ӧ�ı�־λ(λ1)��
	ret
unrshift:
	andb $0xfd,mode 				# ����shift ���ɿ�����λmode �ж�Ӧ�ı�־λ(λ1)��
	ret

caps:	
	testb $0x80,mode				# ����ģʽ��־mode ��λ7 �Ƿ��Ѿ���λ(����״̬)��
	jne 1f							# ����Ѵ��ڰ���״̬���򷵻�(ret)��
	xorb $4,leds					# ��תleds ��־��caps-lock ����λ(λ2)��
	xorb $0x40,mode					# ��תmode ��־��caps �����µı���λ(λ6)��
	orb $0x80,mode					# ����mode ��־��caps ���Ѱ��±�־λ(λ7)��
// ��δ������ leds ��־��������ر� LED ָʾ����
set_leds:
	call kb_wait					# �ȴ����̿��������뻺��ա�
	movb $0xed,%al		/* set leds command */ /* ���� LED ������ */
	outb %al,$0x60					# ���ͼ�������0xed ��0x60 �˿ڡ�
	call kb_wait					# �ȴ����̿��������뻺��ա�
	movb leds,%al					# ȡleds ��־����Ϊ������
	outb %al,$0x60					# ���͸ò�����
	ret
uncaps:	andb $0x7f,mode				# caps ���ɿ�����λģʽ��־mode �еĶ�Ӧλ(λ7)��
	ret
scroll:
	xorb $1,leds					# scroll �����£���תleds ��־�еĶ�Ӧλ(λ0)��
	jmp set_leds					# ����leds ��־���¿�����ر�LED ָʾ����
num:	
	xorb $2,leds					# num �����£���תleds ��־�еĶ�Ӧλ(λ1)��
	jmp set_leds					# ����leds ��־���¿�����ر�LED ָʾ����

/*
 *  curosr-key/numeric keypad cursor keys are handled here.
 *  checking for numeric keypad etc.
 */
/*
 * ���ﴦ�������/����С���̷�������������С���̵ȡ�
 */
cursor:
	subb $0x47,%al  				# ɨ������С���ּ����ϵļ�(��ɨ����>=0x47)�����ģ�
	jb 1f							# ���С���򲻴��������ء�
	cmpb $12,%al					# ���ɨ���� > 0x53(0x53 - 0x47= 12)����
	ja 1f							# ɨ����ֵ����83(0x53)�������������ء�
	jne cur2		/* check for ctrl-alt-del *//* ����Ƿ�ctrl-alt-del */
	// �������12����˵��del ���ѱ����£�������ж�ctrl
	// ��alt �Ƿ�Ҳͬʱ���¡�
	testb $0x0c,mode				# ��ctrl ��������
	je cur2							# �ޣ�����ת
	testb $0x30,mode				# ��alt ��������
	jne reboot						# �У�����ת��������������
cur2:	cmpb $0x01,e0		/* e0 forces cursor movement */
	je cur
	testb $0x02,leds	/* not num-lock forces cursor *//* e0 ��λ��ʾ����ƶ� */
	// e0 ��־��λ����
	je cur							# ��λ�ˣ�����ת����ƶ�������cur��
	testb $0x03,mode	/* shift forces cursor *//* num-lock ������ */
	// ����ģʽ��־mode ��shift ���±�־��
	jne cur							# �����shift �����£���Ҳ���й���ƶ�������
	xorl %ebx,%ebx					# �����ѯɨ���ֱ�(����)��ȡ��Ӧ��������ASCII �롣
	movb num_table(%eax),%al		# ��eax ��Ϊ����ֵ��ȡ��Ӧ�����ַ�al��
	jmp put_queue					# �����ַ����뻺������С�
1:	ret

// ��δ��봦�������ƶ���
cur:	
	movb cur_table(%eax),%al		# ȡ����ַ�������Ӧ���Ĵ����ַ�al��
	cmpb $'9,%al					# ȡ����ַ�������Ӧ���Ĵ����ַ�al��
	ja ok_cur						# �����ַ�������Ҫ�����ַ�'~'��
	movb $'~,%ah
ok_cur:	
	shll $16,%eax					# ��ax �������Ƶ�eax �����С�
	movw $0x5b1b,%ax				# ��ax �з���'esc ['�ַ�����eax �������ַ�����ƶ����С�
	xorl %ebx,%ebx
	jmp put_queue					# �����ַ����뻺������С�


num_table:
	.ascii "789 456 1230,"			# ����С�����ϼ���Ӧ������ASCII �����
cur_table:
	.ascii "HA5 DGC YB623"			# ����С�����Ϸ���������ɾ������Ӧ���ƶ���ʾ�ַ�����

/*
 * this routine handles function keys
 */
 // �����ӳ��������ܼ���
func:
	pushl %eax
	pushl %ecx
	pushl %edx
	call _show_stat					# ������ʾ������״̬����(kernl/sched.c)��
	popl %edx
	popl %ecx
	popl %eax
	subb $0x3B,%al					# ���ܼ�'F1'��ɨ������0x3B����˴�ʱal ���ǹ��ܼ������š�
	jb end_func 					# ���ɨ����С��0x3b���򲻴��������ء�
	cmpb $9,%al 					# ���ܼ���F1-F10��
	jbe ok_func 					# �ǣ�����ת��
	subb $18,%al 					# �ǹ��ܼ�F11��F12 ��
	cmpb $10,%al 					# �ǹ��ܼ�F11��
	jb end_func 					# ���ǣ��򲻴��������ء�
	cmpb $11,%al 					# �ǹ��ܼ�F12��
	ja end_func 					# ���ǣ��򲻴��������ء�
	ok_func:
	cmpl $4,%ecx		/* check that there is enough room *//* ����Ƿ����㹻�ռ�*/
	jl end_func						# ��Ҫ����4 ���ַ����У�����Ų��£��򷵻ء�
	movl func_table(,%eax,4),%eax	# ȡ���ܼ���Ӧ�ַ����С�
	xorl %ebx,%ebx
	jmp put_queue					# ���뻺������С�
end_func:
	ret

/*
 * function keys send F1:'esc [ [ A' F2:'esc [ [ B' etc.
 */
/*
 * ���ܼ����͵�ɨ���룬F1 ��Ϊ��'esc [ [ A'�� F2 ��Ϊ��'esc [ [ B'�ȡ�
 */
func_table:
	.long 0x415b5b1b,0x425b5b1b,0x435b5b1b,0x445b5b1b
	.long 0x455b5b1b,0x465b5b1b,0x475b5b1b,0x485b5b1b
	.long 0x495b5b1b,0x4a5b5b1b,0x4b5b5b1b,0x4c5b5b1b


// ɨ����-ASCII �ַ�ӳ�����
// ������config.h �ж���ļ�������(FINNISH��US��GERMEN��FRANCH)������Ӧ����ɨ����ӳ��
// ��ASCII �ַ���

#if defined(KBD_US)
// �����Ƿ�������̵�ɨ����ӳ�����

key_map:
	.byte 0,27
	.ascii "1234567890-="
	.byte 127,9
	.ascii "qwertyuiop[]"
	.byte 13,0
	.ascii "asdfghjkl;'"
	.byte '`,0
	.ascii "\\zxcvbnm,./"
	.byte 0,'*,0,32		/* 36-39 */
	.fill 16,1,0		/* 3A-49 */
	.byte '-,0,0,0,'+	/* 4A-4E */
	.byte 0,0,0,0,0,0,0	/* 4F-55 */
	.byte '<
	.fill 10,1,0

// shift ��ͬʱ����ʱ��ӳ�����
shift_map:
	.byte 0,27
	.ascii "!@#$%^&*()_+"
	.byte 127,9
	.ascii "QWERTYUIOP{}"
	.byte 13,0
	.ascii "ASDFGHJKL:\""
	.byte '~,0
	.ascii "|ZXCVBNM<>?"
	.byte 0,'*,0,32		/* 36-39 */
	.fill 16,1,0		/* 3A-49 */
	.byte '-,0,0,0,'+	/* 4A-4E */
	.byte 0,0,0,0,0,0,0	/* 4F-55 */
	.byte '>
	.fill 10,1,0

// alt ��ͬʱ����ʱ��ӳ�����
alt_map:
	.byte 0,0
	.ascii "\0@\0$\0\0{[]}\\\0"
	.byte 0,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte '~,13,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte 0,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte 0,0,0,0		/* 36-39 */
	.fill 16,1,0		/* 3A-49 */
	.byte 0,0,0,0,0		/* 4A-4E */
	.byte 0,0,0,0,0,0,0	/* 4F-55 */
	.byte '|
	.fill 10,1,0
#else
#error "KBD-type not defined"
#endif
/*
 * do_self handles "normal" keys, ie keys that don't change meaning
 * and which have just one character returns.
 */
/*
 * do_self ���ڴ�������ͨ������Ҳ������û�б仯����ֻ��һ���ַ����صļ���
 */
do_self:
	lea alt_map,%ebx				# alt ��ͬʱ����ʱ��ӳ�����ַalt_map->ebx
	testb $0x20,mode		/* alt-gr *//* ��alt ��ͬʱ������? */
	jne 1f							# �ǣ�����ǰ��ת�����1 ����
	lea shift_map,%ebx				# shift ��ͬʱ����ʱ��ӳ�����ַshift_map->ebx��
	testb $0x03,mode				# ��shift ��ͬʱ��������
	jne 1f							# �У�����ǰ��ת�����1 ��
	lea key_map,%ebx				# ����ʹ����ͨӳ���key_map��
	// ȡӳ����ж�Ӧɨ�����ASCII �ַ�����û�ж�Ӧ�ַ����򷵻�(תnone)��
1:	movb (%ebx,%eax),%al			# ��ɨ������Ϊ����ֵ��ȡ��Ӧ��ASCII ��->al��
	orb %al,%al						# ��⿴�Ƿ��ж�Ӧ��ASCII �롣
	je none							# ��û��(��Ӧ��ASCII ��=0)���򷵻ء�
	// ��ctrl ���Ѱ��»�caps �������������ַ���'a'-'}'(0x61-0x7D)��Χ�ڣ�����ת�ɴ�д�ַ�
	// (0x41-0x5D)��
	testb $0x4c,mode		/* ctrl or caps */	/* ���Ƽ��Ѱ��»�caps ����*/
	je 2f							# û�У�����ǰ��ת���2 ����
	cmpb $'a,%al					# ��al �е��ַ���'a'�Ƚϡ�
	jb 2f							# ��al ֵ<'a'����ת���2 ����
	cmpb $'},%al					# ��al �е��ַ���'}'�Ƚϡ�
	ja 2f							# ��al ֵ>'}'����ת���2 ����
	subb $32,%al					# ��al ת��Ϊ��д�ַ�(��0x20)��
	// ��ctrl ���Ѱ��£������ַ���'`'--'_'(0x40-0x5F)֮��(�Ǵ�д�ַ�)������ת��Ϊ�����ַ�
	// (0x00-0x1F)��
2:	testb $0x0c,mode		/* ctrl *//* ctrl ��ͬʱ��������*/
	je 3f							# ��û����ת���3��
	cmpb $64,%al					# ��al ��'@'(64)�ַ��Ƚ�(���ж��ַ�������Χ)��
	jb 3f							# ��ֵ<'@'����ת���3��
	cmpb $64+32,%al					# ��al ��'`'(96)�ַ��Ƚ�(���ж��ַ�������Χ)��
	jae 3f							# ��ֵ>='`'����ת���3��
	subb $64,%al					# ����al ֵ��0x40��
	// �����ַ�ת��Ϊ0x00-0x1f ֮��Ŀ����ַ���
	// ����alt ��ͬʱ���£����ַ���λ7 ��λ��
3:	testb $0x10,mode		/* left alt *//* ��alt ��ͬʱ���£�*/
	je 4f							# û�У���ת���4
	orb $0x80,%al					# �ַ���λ7 ��λ��
	// ��al �е��ַ��������������С�
4:	andl $0xff,%eax					# ��eax �ĸ��ֺ�ah��
	xorl %ebx,%ebx
	call put_queue					# ���ַ����뻺������С�
none:	ret

/*
 * minus has a routine of it's own, as a 'E0h' before
 * the scan code for minus means that the numeric keypad
 * slash was pushed.
 */
/*
 * ���������Լ��Ĵ����ӳ�����Ϊ�ڼ���ɨ����֮ǰ��0xe0
 * ��ζ�Ű���������С�����ϵ�б�ܼ���
 */
minus: 
	cmpb $1,e0 						# e0 ��־��λ����
	jne do_self 					# û�У������do_self �Լ��ŷ�������ͨ������
	movl $'/,%eax 					# ������'/'�滻����'-'>al��
	xorl %ebx,%ebx
	jmp put_queue 					# �����ַ����뻺������С�

/*
 * This table decides which routine to call when a scan-code has been
 * gotten. Most routines just call do_self, or none, depending if
 * they are make or break.
 */
/* ������һ���ӳ����ַ��ת������ȡ��ɨ�����͸��ݴ˱�������Ӧ��ɨ���봦���ӳ���
 * ��������õ��ӳ�����do_self��������none����������ǰ���(make)�����ͷż�(break)��
 */
key_table:
	.long none,do_self,do_self,do_self	/* 00-03 s0 esc 1 2 */
	.long do_self,do_self,do_self,do_self	/* 04-07 3 4 5 6 */
	.long do_self,do_self,do_self,do_self	/* 08-0B 7 8 9 0 */
	.long do_self,do_self,do_self,do_self	/* 0C-0F + ' bs tab */
	.long do_self,do_self,do_self,do_self	/* 10-13 q w e r */
	.long do_self,do_self,do_self,do_self	/* 14-17 t y u i */
	.long do_self,do_self,do_self,do_self	/* 18-1B o p } ^ */
	.long do_self,ctrl,do_self,do_self	/* 1C-1F enter ctrl a s */
	.long do_self,do_self,do_self,do_self	/* 20-23 d f g h */
	.long do_self,do_self,do_self,do_self	/* 24-27 j k l | */
	.long do_self,do_self,lshift,do_self	/* 28-2B { para lshift , */
	.long do_self,do_self,do_self,do_self	/* 2C-2F z x c v */
	.long do_self,do_self,do_self,do_self	/* 30-33 b n m , */
	.long do_self,minus,rshift,do_self	/* 34-37 . - rshift * */
	.long alt,do_self,caps,func		/* 38-3B alt sp caps f1 */
	.long func,func,func,func		/* 3C-3F f2 f3 f4 f5 */
	.long func,func,func,func		/* 40-43 f6 f7 f8 f9 */
	.long func,num,scroll,cursor		/* 44-47 f10 num scr home */
	.long cursor,cursor,do_self,cursor	/* 48-4B up pgup - left */
	.long cursor,cursor,do_self,cursor	/* 4C-4F n5 right + end */
	.long cursor,cursor,cursor,cursor	/* 50-53 dn pgdn ins del */
	.long none,none,do_self,func		/* 54-57 sysreq ? < f11 */
	.long func,none,none,none		/* 58-5B f12 ? ? ? */
	.long none,none,none,none		/* 5C-5F ? ? ? ? */
	.long none,none,none,none		/* 60-63 ? ? ? ? */
	.long none,none,none,none		/* 64-67 ? ? ? ? */
	.long none,none,none,none		/* 68-6B ? ? ? ? */
	.long none,none,none,none		/* 6C-6F ? ? ? ? */
	.long none,none,none,none		/* 70-73 ? ? ? ? */
	.long none,none,none,none		/* 74-77 ? ? ? ? */
	.long none,none,none,none		/* 78-7B ? ? ? ? */
	.long none,none,none,none		/* 7C-7F ? ? ? ? */
	.long none,none,none,none		/* 80-83 ? br br br */
	.long none,none,none,none		/* 84-87 br br br br */
	.long none,none,none,none		/* 88-8B br br br br */
	.long none,none,none,none		/* 8C-8F br br br br */
	.long none,none,none,none		/* 90-93 br br br br */
	.long none,none,none,none		/* 94-97 br br br br */
	.long none,none,none,none		/* 98-9B br br br br */
	.long none,unctrl,none,none		/* 9C-9F br unctrl br br */
	.long none,none,none,none		/* A0-A3 br br br br */
	.long none,none,none,none		/* A4-A7 br br br br */
	.long none,none,unlshift,none		/* A8-AB br br unlshift br */
	.long none,none,none,none		/* AC-AF br br br br */
	.long none,none,none,none		/* B0-B3 br br br br */
	.long none,none,unrshift,none		/* B4-B7 br br unrshift br */
	.long unalt,none,uncaps,none		/* B8-BB unalt br uncaps br */
	.long none,none,none,none		/* BC-BF br br br br */
	.long none,none,none,none		/* C0-C3 br br br br */
	.long none,none,none,none		/* C4-C7 br br br br */
	.long none,none,none,none		/* C8-CB br br br br */
	.long none,none,none,none		/* CC-CF br br br br */
	.long none,none,none,none		/* D0-D3 br br br br */
	.long none,none,none,none		/* D4-D7 br br br br */
	.long none,none,none,none		/* D8-DB br ? ? ? */
	.long none,none,none,none		/* DC-DF ? ? ? ? */
	.long none,none,none,none		/* E0-E3 e0 e1 ? ? */
	.long none,none,none,none		/* E4-E7 ? ? ? ? */
	.long none,none,none,none		/* E8-EB ? ? ? ? */
	.long none,none,none,none		/* EC-EF ? ? ? ? */
	.long none,none,none,none		/* F0-F3 ? ? ? ? */
	.long none,none,none,none		/* F4-F7 ? ? ? ? */
	.long none,none,none,none		/* F8-FB ? ? ? ? */
	.long none,none,none,none		/* FC-FF ? ? ? ? */

/*
 * kb_wait waits for the keyboard controller buffer to empty.
 * there is no timeout - if the buffer doesn't empty, we hang.
 */
/*
 * �ӳ���kb_wait ���ڵȴ����̿���������ա������ڳ�ʱ���� - ���
 * ������Զ���յĻ�������ͻ���Զ�ȴ�(����)��
 */
kb_wait:
	pushl %eax
1:	inb $0x64,%al					# �����̿�����״̬��
	testb $0x02,%al					# �������뻺�����Ƿ�Ϊ��(����0)��
	jne 1b							# �����գ�����תѭ���ȴ���
	popl %eax
	ret
/*
 * This routine reboots the machine by asking the keyboard
 * controller to pulse the reset-line low.
 */
/*
 * ���ӳ���ͨ�����ü��̿���������λ����������壬ʹϵͳ��λ����(reboot)��
 */
reboot:
	call kb_wait					# ���ȵȴ����̿��������뻺�����ա�
	movw $0x1234,0x472	/* don't do memory check */
	movb $0xfc,%al		/* pulse reset and A20 low */
	outb %al,$0x64					# ��ϵͳ��λ��A20 ����������塣
die:	jmp die						# ������