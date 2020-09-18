extern int fflag;//̰�����ܿ��� ����������
extern int sflag1;//�߷���
extern int sflag2;//�߷���
extern int sflag3;//�߷���

//// �л����û�ģʽ���С�
// �ú�������iret ָ��ʵ�ִ��ں�ģʽ�л����û�ģʽ����ʼ����0����
#define move_to_user_mode() \
__asm__ ("movl %%esp,%%eax\n\t" \
	"pushl $0x17\n\t" \
	"pushl %%eax\n\t" \
	"pushfl\n\t" \
	"pushl $0x0f\n\t" \
	"pushl $1f\n\t" \
	"iret\n" \
	"1:\tmovl $0x17,%%eax\n\t" \
	"movw %%ax,%%ds\n\t" \
	"movw %%ax,%%es\n\t" \
	"movw %%ax,%%fs\n\t" \
	"movw %%ax,%%gs" \
	:::"ax")

#define sti() __asm__ ("sti"::)			// ���ж�Ƕ����꺯����
#define cli() __asm__ ("cli"::)			// ���жϡ�
#define nop() __asm__ ("nop"::)			// �ղ�����
#define iret() __asm__ ("iret"::)		// �жϷ��ء�

//// �������������꺯����
// ������gate_addr -��������ַ��type -��������������ֵ��dpl -��������Ȩ��ֵ��addr -ƫ�Ƶ�ַ��
// %0 - (��dpl,type ��ϳɵ����ͱ�־��)��%1 - (��������4 �ֽڵ�ַ)��
// %2 - (��������4 �ֽڵ�ַ)��%3 - edx(����ƫ�Ƶ�ַaddr)��%4 - eax(�����к��ж�ѡ���)��
#define _set_gate(gate_addr,type,dpl,addr) \
__asm__ ("movw %%dx,%%ax\n\t" \
	"movw %0,%%dx\n\t" \
	"movl %%eax,%1\n\t" \
	"movl %%edx,%2" \
	: \
	: "i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	"o" (*((char *) (gate_addr))), \
	"o" (*(4+(char *) (gate_addr))), \
	"d" ((char *) (addr)),"a" (0x00080000))

//// �����ж��ź�����
// ������n - �жϺţ�addr - �жϳ���ƫ�Ƶ�ַ��
// &idt[n]��Ӧ�жϺ����ж����������е�ƫ��ֵ���ж���������������14����Ȩ����0��
#define set_intr_gate(n,addr) \
	_set_gate(&idt[n],14,0,addr)

//// ���������ź�����
// ������n - �жϺţ�addr - �жϳ���ƫ�Ƶ�ַ��
// &idt[n]��Ӧ�жϺ����ж����������е�ƫ��ֵ���ж���������������15����Ȩ����0��
#define set_trap_gate(n,addr) \
	_set_gate(&idt[n],15,0,addr)

//// ����ϵͳ�����ź�����
// ������n - �жϺţ�addr - �жϳ���ƫ�Ƶ�ַ��
// &idt[n]��Ӧ�жϺ����ж����������е�ƫ��ֵ���ж���������������15����Ȩ����3��
#define set_system_gate(n,addr) \
	_set_gate(&idt[n],15,3,addr)

//// ���ö�������������
// ������gate_addr -��������ַ��type -��������������ֵ��dpl -��������Ȩ��ֵ��
// base - �εĻ���ַ��limit - ���޳������μ����������ĸ�ʽ��
#define _set_seg_desc(gate_addr,type,dpl,base,limit) {\
	*(gate_addr) = ((base) & 0xff000000) | \
		(((base) & 0x00ff0000)>>16) | \
		((limit) & 0xf0000) | \
		((dpl)<<13) | \
		(0x00408000) | \
		((type)<<8); \
	*((gate_addr)+1) = (((base) & 0x0000ffff)<<16) | \
		((limit) & 0x0ffff); }

//// ��ȫ�ֱ�����������״̬��/�ֲ�����������
// ������n - ��ȫ�ֱ�����������n ����Ӧ�ĵ�ַ��addr - ״̬��/�ֲ��������ڴ�Ļ���ַ��
// type - �������еı�־�����ֽڡ�
// %0 - eax(��ַaddr)��%1 - (��������n �ĵ�ַ)��%2 - (��������n �ĵ�ַƫ��2 ��)��
// %3 - (��������n �ĵ�ַƫ��4 ��)��%4 - (��������n �ĵ�ַƫ��5 ��)��
// %5 - (��������n �ĵ�ַƫ��6 ��)��%6 - (��������n �ĵ�ַƫ��7 ��)��
#define _set_tssldt_desc(n,addr,type) \
__asm__ ("movw $104,%1\n\t" \
	"movw %%ax,%2\n\t" \
	"rorl $16,%%eax\n\t" \
	"movb %%al,%3\n\t" \
	"movb $" type ",%4\n\t" \
	"movb $0x00,%5\n\t" \
	"movb %%ah,%6\n\t" \
	"rorl $16,%%eax" \
	::"a" (addr), "m" (*(n)), "m" (*(n+2)), "m" (*(n+4)), \
	 "m" (*(n+5)), "m" (*(n+6)), "m" (*(n+7)) \
	)

//// ��ȫ�ֱ�����������״̬����������
// n - �Ǹ���������ָ�룻addr - ���������еĻ���ֵַ������״̬����������������0x89��
#define set_tss_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0x89")
//// ��ȫ�ֱ������þֲ�����������
// n - �Ǹ���������ָ�룻addr - ���������еĻ���ֵַ���ֲ�����������������0x82��
#define set_ldt_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0x82")

