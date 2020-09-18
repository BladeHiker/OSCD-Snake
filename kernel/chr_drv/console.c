/*
 *  linux/kernel/console.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 *	console.c
 *
 * This module implements the console io functions
 *	'void con_init(void)'
 *	'void con_write(struct tty_queue * queue)'
 * Hopefully this will be a rather complete VT102 implementation.
 *
 * Beeping thanks to John T Kohl.
 */
/*
 * ��ģ��ʵ�ֿ���̨�����������
 * 'void con_init(void)'
 * 'void con_write(struct tty_queue * queue)'
 * ϣ������һ���ǳ�������VT102 ʵ�֡�
 *
 * ��лJohn T Kohl ʵ���˷���ָʾ��
 */

/*
 *  NOTE!!! We sometimes disable and enable interrupts for a short while
 * (to put a word in video IO), but this will work even for keyboard
 * interrupts. We know interrupts aren't enabled when getting a keyboard
 * interrupt, as we use trap-gates. Hopefully all is well.
 */
/*
 * ע��!!! ������ʱ���ݵؽ�ֹ�������ж�(�ڽ�һ����(word)�ŵ���ƵIO)������ʹ
 * ���ڼ����ж���Ҳ�ǿ��Թ����ġ���Ϊ����ʹ�������ţ���������֪���ڻ��һ��
 * �����ж�ʱ�ж��ǲ�����ġ�ϣ��һ�о�������
 */

/*
 * Code to check for different video-cards mostly by Galen Hunt,
 * <g-hunt@ee.utah.edu>
 */
/*
 * ��ⲻͬ��ʾ���Ĵ���������Galen Hunt ��д�ģ�
 * <g-hunt@ee.utah.edu>
 */

#include <linux/sched.h> // ���ȳ���ͷ�ļ�������������ṹtask_struct����ʼ����0 �����ݣ�
						 // ����һЩ�й��������������úͻ�ȡ��Ƕ��ʽ��ຯ������䡣
#include <linux/tty.h>	 // tty ͷ�ļ����������й�tty_io������ͨ�ŷ���Ĳ�����������
#include <asm/io.h>		 // io ͷ�ļ�������Ӳ���˿�����/���������䡣
#include <asm/system.h>	 // ϵͳͷ�ļ������������û��޸�������/�ж��ŵȵ�Ƕ��ʽ���ꡣ

/*
 * These are set up by the setup-routine at boot-time:
 */
/*
 * ��Щ�������ӳ���setup ����������ϵͳʱ���õĲ�����
 */

#define ORIG_X (*(unsigned char *)0x90000)							   // ����кš�
#define ORIG_Y (*(unsigned char *)0x90001)							   // ����кš�
#define ORIG_VIDEO_PAGE (*(unsigned short *)0x90004)				   // ��ʾҳ�档
#define ORIG_VIDEO_MODE ((*(unsigned short *)0x90006) & 0xff)		   // ��ʾģʽ��
#define ORIG_VIDEO_COLS (((*(unsigned short *)0x90006) & 0xff00) >> 8) // �ַ�������
#define ORIG_VIDEO_LINES (25)										   // ��ʾ������
#define ORIG_VIDEO_EGA_AX (*(unsigned short *)0x90008)				   // [??]
#define ORIG_VIDEO_EGA_BX (*(unsigned short *)0x9000a)				   // ��ʾ�ڴ��С��ɫ��ģʽ��
#define ORIG_VIDEO_EGA_CX (*(unsigned short *)0x9000c)				   // ��ʾ�����Բ�����

// ������ʾ����ɫ/��ɫ��ʾģʽ���ͷ��ų�����
#define VIDEO_TYPE_MDA 0x10 /* Monochrome Text Display */	  /* ��ɫ�ı� */
#define VIDEO_TYPE_CGA 0x11 /* CGA Display */				  /* CGA ��ʾ�� */
#define VIDEO_TYPE_EGAM 0x20 /* EGA/VGA in Monochrome Mode */ /* EGA/VGA ��ɫ */
#define VIDEO_TYPE_EGAC 0x21 /* EGA/VGA in Color Mode */	  /* EGA/VGA ��ɫ */

#define NPAR 16

extern void keyboard_interrupt(void); // �����жϴ������(keyboard.S)��

static unsigned char video_type;		/* Type of display being used */
										/* ʹ�õ���ʾ���� */
static unsigned long video_num_columns; /* Number of text columns */
										/* ��Ļ�ı����� */
static unsigned long video_size_row;	/* Bytes per row */
										/* ÿ��ʹ�õ��ֽ��� */
static unsigned long video_num_lines;	/* Number of test lines */
										/* ��Ļ�ı����� */
static unsigned char video_page;		/* Initial video page */
										/* ��ʼ��ʾҳ�� */
static unsigned long video_mem_start;	/* Start of video RAM */
										/* ��ʾ�ڴ���ʼ��ַ */
static unsigned long video_mem_end;		/* End of video RAM (sort of) */
										/* ��ʾ�ڴ����(ĩ��)��ַ */
static unsigned short video_port_reg;	/* Video register select port */
										/* ��ʾ���������Ĵ����˿� */
static unsigned short video_port_val;	/* Video register value port */
										/* ��ʾ�������ݼĴ����˿� */
static unsigned short video_erase_char; /* Char+Attrib to erase with */
										/* �����ַ��������ַ�(0x0720) */
// ������Щ����������Ļ����������
static unsigned long origin; /* Used for EGA/VGA fast scroll */ // scr_start��
/* ����EGA/VGA ���ٹ��� */										// ������ʼ�ڴ��ַ��
static unsigned long scr_end;									/* Used for EGA/VGA fast scroll */
/* ����EGA/VGA ���ٹ��� */										// ����ĩ���ڴ��ַ��
static unsigned long pos;										// ��ǰ����Ӧ����ʾ�ڴ�λ�á�
static unsigned long x, y;										// ��ǰ���λ�á�
static unsigned long top, bottom;								// ����ʱ�����кţ������кš�
																// state ���ڱ�������ESC ת������ʱ�ĵ�ǰ���衣npar,par[]���ڴ��ESC ���е��м䴦�������
static unsigned long state = 0;									// ANSI ת���ַ����д���״̬��
static unsigned long npar, par[NPAR];							// ANSI ת���ַ����в��������Ͳ������顣
static unsigned long ques = 0;
static unsigned char attr = 0x07; // �ַ�����(�ڵװ���)��

static void sysbeep(void); // ϵͳ����������

/*
 * this is what the terminal answers to a ESC-Z or csi0c
 * query (= vt100 response).
 */
/*
 * �������ն˻�ӦESC-Z ��csi0c �����Ӧ��(=vt100 ��Ӧ)��
 */
// csi - ��������������(Control Sequence Introducer)��
#define RESPONSE "\033[?1;2c"

/* NOTE! gotoxy thinks x==video_num_columns is ok */
/* ע�⣡gotoxy ������Ϊx==video_num_columns��������ȷ�� */
//// ���ٹ�굱ǰλ�á�
// ������new_x - ��������кţ�new_y - ��������кš�
// ���µ�ǰ���λ�ñ���x,y��������pos ָ��������ʾ�ڴ��еĶ�Ӧλ�á�
static inline void gotoxy(unsigned int new_x, unsigned int new_y)
{
	// �������Ĺ���кų�����ʾ�����������߹���кų�����ʾ��������������˳���
	if (new_x > video_num_columns || new_y >= video_num_lines)
		return;
	// ���µ�ǰ�����������¹��λ�ö�Ӧ������ʾ�ڴ���λ�ñ���pos��
	x = new_x;
	y = new_y;
	pos = origin + y * video_size_row + (x << 1);
}

//// ���ù�����ʼ��ʾ�ڴ��ַ��
static inline void set_origin(void)
{
	cli();
	// ����ѡ����ʾ�������ݼĴ���r12��Ȼ��д�������ʼ��ַ���ֽڡ������ƶ�9 λ����ʾ�����ƶ�
	// 8 λ���ٳ���2(2 �ֽڴ�����Ļ��1 �ַ�)���������Ĭ����ʾ�ڴ�����ġ�
	outb_p(12, video_port_reg);
	outb_p(0xff & ((origin - video_mem_start) >> 9), video_port_val);
	// ��ѡ����ʾ�������ݼĴ���r13��Ȼ��д�������ʼ��ַ���ֽڡ������ƶ�1 λ��ʾ����2��
	outb_p(13, video_port_reg);
	outb_p(0xff & ((origin - video_mem_start) >> 1), video_port_val);
	sti();
}

//// ���Ͼ�һ�У���Ļ���������ƶ�����
// ����Ļ���������ƶ�һ�С�
static void scrup(void)
{
	// �����ʾ������EGA����ִ�����²�����
	if (video_type == VIDEO_TYPE_EGAC || video_type == VIDEO_TYPE_EGAM)
	{
		// ����ƶ���ʼ��top=0���ƶ������bottom=video_num_lines=25�����ʾ�������������ƶ���
		if (!top && bottom == video_num_lines)
		{
			// ������Ļ��ʾ��Ӧ�ڴ����ʼλ��ָ��origin Ϊ������һ����Ļ�ַ���Ӧ���ڴ�λ�ã�ͬʱҲ����
			// ��ǰ����Ӧ���ڴ�λ���Լ���Ļĩ��ĩ���ַ�ָ��scr_end ��λ�á�
			origin += video_size_row;
			pos += video_size_row;
			scr_end += video_size_row;
			// �����Ļĩ�����һ����ʾ�ַ�����Ӧ����ʾ�ڴ�ָ��scr_end ������ʵ����ʾ�ڴ��ĩ�ˣ���
			// ��Ļ�����ڴ������ƶ�����ʾ�ڴ����ʼλ��video_mem_start �������ڳ��ֵ�����������ո��ַ���
			if (scr_end > video_mem_end)
			{
				// %0 - eax(�����ַ�+����)��%1 - ecx((��ʾ���ַ�����-1)����Ӧ���ַ���/2�����Գ����ƶ�)��
				// %2 - edi(��ʾ�ڴ���ʼλ��video_mem_start)��%3 - esi(��Ļ���ݶ�Ӧ���ڴ���ʼλ��origin)��
				// �ƶ�����[edi]??[esi]���ƶ�ecx �����֡�
				__asm__("cld\n\t"						 // �巽��λ��
						"rep\n\t"						 // �ظ�����������ǰ��Ļ�ڴ�����
						"movsl\n\t"						 // �ƶ�����ʾ�ڴ���ʼ����
						"movl _video_num_columns,%1\n\t" // ecx=1 ���ַ�����
						"rep\n\t"						 // ������������ո��ַ���
						"stosw" ::"a"(video_erase_char),
						"c"((video_num_lines - 1) * video_num_columns >> 1),
						"D"(video_mem_start),
						"S"(origin));
				// ������Ļ�ڴ������ƶ������������µ�����ǰ��Ļ��Ӧ�ڴ����ʼָ�롢���λ��ָ�����Ļĩ��
				// ��Ӧ�ڴ�ָ��scr_end��
				scr_end -= origin - video_mem_start;
				pos -= origin - video_mem_start;
				origin = video_mem_start;
			}
			else
			{
				// ������������Ļĩ�˶�Ӧ���ڴ�ָ��scr_end û�г�����ʾ�ڴ��ĩ��video_mem_end����ֻ����
				// ��������������ַ�(�ո��ַ�)��
				// %0 - eax(�����ַ�+����)��%1 - ecx(��ʾ���ַ�����)��%2 - edi(��Ļ��Ӧ�ڴ����һ�п�ʼ��)��
				__asm__(
					"cld\n\t" // �巽��λ��
					"rep\n\t" // �ظ����������³�������
					"stosw"	  // ��������ַ�(�ո��ַ�)��
					::"a"(video_erase_char),
					"c"(video_num_columns),
					"D"(scr_end - video_size_row));
			}
			// ����ʾ��������д���µ���Ļ���ݶ�Ӧ���ڴ���ʼλ��ֵ��
			set_origin();
			// �����ʾ���������ƶ���Ҳ����ʾ��ָ����top ��ʼ�������������ƶ�1 ��(ɾ��1 ��)����ʱֱ��
			// ����Ļ��ָ����top ����Ļĩ�������ж�Ӧ����ʾ�ڴ����������ƶ�1 �У������³��ֵ����������
			// ���ַ���
			// %0-eax(�����ַ�+����)��%1-ecx(top ����1 �п�ʼ����Ļĩ�е���������Ӧ���ڴ泤����)��
			// %2-edi(top ���������ڴ�λ��)��%3-esi(top+1 ���������ڴ�λ��)��
		}
		else
		{
			__asm__("cld\n\t"
					"rep\n\t"
					"movsl\n\t"
					"movl _video_num_columns,%%ecx\n\t"
					"rep\n\t"
					"stosw" ::"a"(video_erase_char),
					"c"((bottom - top - 1) * video_num_columns >> 1),
					"D"(origin + video_size_row * top),
					"S"(origin + video_size_row * (top + 1)));
		}
	}
	// �����ʾ���Ͳ���EGA(��MDA)����ִ�������ƶ���������ΪMDA ��ʾ���ƿ����Զ�����������ʾ��Χ
	// �������Ҳ�����Զ�����ָ�룬�������ﲻ����Ļ���ݶ�Ӧ�ڴ泬����ʾ�ڴ�����������������
	// ������EGA �������ƶ������ȫһ����
	else /* Not EGA/VGA */
	{
		__asm__("cld\n\t"
				"rep\n\t"
				"movsl\n\t"
				"movl _video_num_columns,%%ecx\n\t"
				"rep\n\t"
				"stosw" ::"a"(video_erase_char),
				"c"((bottom - top - 1) * video_num_columns >> 1),
				"D"(origin + video_size_row * top),
				"S"(origin + video_size_row * (top + 1)));
	}
}

//// ���¾�һ�У���Ļ���������ƶ�����
// ����Ļ���������ƶ�һ�У���Ļ��ʾ�����������ƶ�1 �У��ڱ��ƶ���ʼ�е��Ϸ�����һ���С��μ�
// �����б��˵������������scrup()���ƣ�ֻ��Ϊ�����ƶ���ʾ�ڴ�����ʱ���������ݸ��Ǵ�����
// �����������Է�������еģ�Ҳ������Ļ������2 �е����һ���ַ���ʼ����
static void scrdown(void)
{
	// �����ʾ������EGA����ִ�����в�����
	if (video_type == VIDEO_TYPE_EGAC || video_type == VIDEO_TYPE_EGAM)
	{
		// %0-eax(�����ַ�+����)��%1-ecx(top �п�ʼ����Ļĩ��-1 �е���������Ӧ���ڴ泤����)��
		// %2-edi(��Ļ���½����һ������λ��)��%3-esi(��Ļ������2 �����һ������λ��)��
		// �ƶ�����[esi]->[edi]���ƶ�ecx �����֡�
		__asm__("std\n\t"							// �÷���λ��
				"rep\n\t"							// �ظ������������ƶ���top �е�bottom-1 ��
				"movsl\n\t"							// ��Ӧ���ڴ����ݡ�
				"addl $2,%%edi\n\t"					/* %edi has been decremented by 4 */
													/* %edi �Ѿ���4����ΪҲ�Ƿ���������ַ� */
				"movl _video_num_columns,%%ecx\n\t" // ��ecx=1 ���ַ�����
				"rep\n\t"							// �������ַ������Ϸ������С�
				"stosw" ::"a"(video_erase_char),
				"c"((bottom - top - 1) * video_num_columns >> 1),
				"D"(origin + video_size_row * bottom - 4),
				"S"(origin + video_size_row * (bottom - 1) - 4));
	}
	else /* Not EGA/VGA */ // �������EGA ��ʾ���ͣ���ִ�����²�����Ŀǰ��������ȫһ������
	{
		__asm__(
			"std\n\t"
			"rep\n\t"
			"movsl\n\t"
			"addl $2,%%edi\n\t" /* %edi has been decremented by 4 */
			"movl _video_num_columns,%%ecx\n\t"
			"rep\n\t"
			"stosw" ::"a"(video_erase_char),
			"c"((bottom - top - 1) * video_num_columns >> 1),
			"D"(origin + video_size_row * bottom - 4),
			"S"(origin + video_size_row * (bottom - 1) - 4));
	}
}

//// ���λ������һ��(lf - line feed ����)��
static void lf(void)
{
	// ������û�д��ڵ�����2 ��֮����ֱ���޸Ĺ�굱ǰ�б���y++������������Ӧ��ʾ�ڴ�λ��
	// pos(������Ļһ���ַ�����Ӧ���ڴ泤��)��
	if (y + 1 < bottom)
	{
		y++;
		pos += video_size_row;
		return;
	}
	// ������Ҫ����Ļ��������һ�С�
	scrup();
}

//// �������һ��(ri - reverse line feed ������)��
static void ri(void)
{
	// �����겻�ڵ�1 ���ϣ���ֱ���޸Ĺ�굱ǰ�б���y--������������Ӧ��ʾ�ڴ�λ��pos����ȥ
	// ��Ļ��һ���ַ�����Ӧ���ڴ泤���ֽ�����
	if (y > top)
	{
		y--;
		pos -= video_size_row;
		return;
	}
	// ������Ҫ����Ļ��������һ�С�
	scrdown();
}

// ���ص���1 ��(0 ��)���(cr - carriage return �س�)��
static void cr(void)
{
	// ������ڵ��к�*2 ��0 �е���������ж�Ӧ���ڴ��ֽڳ��ȡ�
	pos -= x << 1;
	x = 0;
}

// �������ǰһ�ַ�(�ÿո����)(del - delete ɾ��)��
static void del(void)
{
	// ������û�д���0 �У��򽫹���Ӧ�ڴ�λ��ָ��pos ����2 �ֽ�(��Ӧ��Ļ��һ���ַ�)��Ȼ��
	// ����ǰ��������ֵ��1�������������λ���ַ�������
	if (x)
	{
		pos -= 2;
		x--;
		*(unsigned short *)pos = video_erase_char;
	}
}

//// ɾ����Ļ������λ����صĲ��֣�����ĻΪ��λ��csi - ��������������(Control Sequence
// Introducer)��
// ANSI ת�����У�'ESC [sJ'(s = 0 ɾ����굽��Ļ�׶ˣ�1 ɾ����Ļ��ʼ����괦��2 ����ɾ��)��
// ������par - ��Ӧ����s��
static void csi_J(int par)
{
	long count;
	long start;

	// ���ȸ�����������ֱ�������Ҫɾ�����ַ�����ɾ����ʼ����ʾ�ڴ�λ�á�
	switch (par)
	{
	case 0: /* erase from cursor to end of display */ /* ������굽��Ļ�׶� */
		count = (scr_end - pos) >> 1;
		start = pos;
		break;
	case 1: /* erase from start to cursor */ /* ɾ������Ļ��ʼ����괦���ַ� */
		count = (pos - origin) >> 1;
		start = origin;
		break;
	case 2: /* erase whole display */ /* ɾ��������Ļ�ϵ��ַ� */
		count = video_num_columns * video_num_lines;
		start = origin;
		break;
	default:
		return;
	}
	// Ȼ��ʹ�ò����ַ���дɾ���ַ��ĵط���
	// %0 - ecx(Ҫɾ�����ַ���count)��%1 - edi(ɾ��������ʼ��ַ)��%2 - eax������Ĳ����ַ�����
	__asm__("cld\n\t"
			"rep\n\t"
			"stosw\n\t" ::"c"(count),
			"D"(start), "a"(video_erase_char));
}

//// ɾ����������λ����صĲ��֣���һ��Ϊ��λ��
// ANSI ת���ַ����У�'ESC [sK'(s = 0 ɾ������β��1 �ӿ�ʼɾ����2 ���ж�ɾ��)��
static void csi_K(int par)
{
	long count;
	long start;

	// ���ȸ�����������ֱ�������Ҫɾ�����ַ�����ɾ����ʼ����ʾ�ڴ�λ�á�
	switch (par)
	{
	case 0: /* erase from cursor to end of line */ /* ɾ����굽��β�ַ� */
		if (x >= video_num_columns)
			return;
		count = video_num_columns - x;
		start = pos;
		break;
	case 1: /* erase from start of line to cursor */ /* ɾ�����п�ʼ����괦 */
		start = pos - (x << 1);
		count = (x < video_num_columns) ? x : video_num_columns;
		break;
	case 2: /* erase whole line */ /* �������ַ�ȫɾ�� */
		start = pos - (x << 1);
		count = video_num_columns;
		break;
	default:
		return;
	}
	// Ȼ��ʹ�ò����ַ���дɾ���ַ��ĵط���
	// %0 - ecx(Ҫɾ�����ַ���count)��%1 - edi(ɾ��������ʼ��ַ)��%2 - eax������Ĳ����ַ�����
	__asm__("cld\n\t"
			"rep\n\t"
			"stosw\n\t" ::"c"(count),
			"D"(start), "a"(video_erase_char));
}

//// ������(����)���������������ַ���ʾ��ʽ������Ӵ֡����»��ߡ���˸�����Եȣ���
// ANSI ת���ַ����У�'ESC [nm'��n = 0 ������ʾ��1 �Ӵ֣�4 ���»��ߣ�7 ���ԣ�27 ������ʾ��
void csi_m(void)
{
	int i;

	for (i = 0; i <= npar; i++)
		switch (par[i])
		{
		case 0:
			attr = 0x07;
			break;
		case 1:
			attr = 0x0f;
			break;
		case 4:
			attr = 0x0f;
			break;
		case 7:
			attr = 0x70;
			break;
		case 27:
			attr = 0x07;
			break;
		}
}

//// ����������ʾ��ꡣ
// ������ʾ�ڴ����Ӧλ��pos��������ʾ������������ʾλ�á�
static inline void set_cursor(void)
{
	cli();
	// ����ʹ�������Ĵ����˿�ѡ����ʾ�������ݼĴ���r14(��굱ǰ��ʾλ�ø��ֽ�)��Ȼ��д����
	// ��ǰλ�ø��ֽ�(�����ƶ�9 λ��ʾ���ֽ��Ƶ����ֽ��ٳ���2)���������Ĭ����ʾ�ڴ�����ġ�
	outb_p(14, video_port_reg);
	outb_p(0xff & ((pos - video_mem_start) >> 9), video_port_val);
	// ��ʹ�������Ĵ���ѡ��r15��������굱ǰλ�õ��ֽ�д�����С�
	outb_p(15, video_port_reg);
	outb_p(0xff & ((pos - video_mem_start) >> 1), video_port_val);
	sti();
}

//// ���Ͷ��ն�VT100 ����Ӧ���С�
// ����Ӧ���з������������С�
static void respond(struct tty_struct *tty)
{
	char *p = RESPONSE;

	cli();
	while (*p)
	{ // ���ַ����з���д���С�
		PUTCH(*p, tty->read_q);
		p++;
	}
	sti();
	copy_to_cooked(tty); // ת���ɹ淶ģʽ(���븨��������)��
}

//// �ڹ�괦����һ�ո��ַ���
static void insert_char(void)
{
	int i = x;
	unsigned short tmp, old = video_erase_char;
	unsigned short *p = (unsigned short *)pos;

	// ��꿪ʼ�������ַ�����һ�񣬲��������ַ������ڹ�����ڴ���
	// ��һ���϶����ַ��Ļ����������һ���ַ����������??
	while (i++ < video_num_columns)
	{
		tmp = *p;
		*p = old;
		old = tmp;
		p++;
	}
}

//// �ڹ�괦����һ�У����꽫�����µĿ����ϣ���
// ����Ļ�ӹ�������е���Ļ�����¾�һ�С�
static void insert_line(void)
{
	int oldtop, oldbottom;

	oldtop = top; // ����ԭtop��bottom ֵ��
	oldbottom = bottom;
	top = y;				  // ������Ļ����ʼ�С�
	bottom = video_num_lines; // ������Ļ������С�
	scrdown();				  // �ӹ�꿪ʼ������Ļ�������¹���һ�С�
	top = oldtop;			  // �ָ�ԭtop��bottom ֵ��
	bottom = oldbottom;
}

//// ɾ����괦��һ���ַ���
static void delete_char(void)
{
	int i;
	unsigned short *p = (unsigned short *)pos;

	// �����곬����Ļ�����У��򷵻ء�
	if (x >= video_num_columns)
		return;
	// �ӹ����һ���ַ���ʼ����ĩ�����ַ�����һ��
	i = x;
	while (++i < video_num_columns)
	{
		*p = *(p + 1);
		p++;
	}
	// ���һ���ַ�����������ַ�(�ո��ַ�)��
	*p = video_erase_char;
}

//// ɾ����������С�
// �ӹ�������п�ʼ��Ļ�����Ͼ�һ�С�
static void delete_line(void)
{
	int oldtop, oldbottom;

	oldtop = top; // ����ԭtop��bottom ֵ��
	oldbottom = bottom;
	top = y;				  // ������Ļ����ʼ�С�
	bottom = video_num_lines; // ������Ļ������С�
	scrup();				  // �ӹ�꿪ʼ������Ļ�������Ϲ���һ�С�
	top = oldtop;			  // �ָ�ԭtop��bottom ֵ��
	bottom = oldbottom;
}

//// �ڹ�괦����nr ���ַ���
// ANSI ת���ַ����У�'ESC [n@ '��
// ���� nr = ����n��
static void csi_at(unsigned int nr)
{
	// ���������ַ�������һ���ַ��������Ϊһ���ַ������������ַ���nr Ϊ0�������1 ���ַ���
	if (nr > video_num_columns)
		nr = video_num_columns;
	else if (!nr)
		nr = 1;
	// ѭ������ָ�����ַ�����
	while (nr--)
		insert_char();
}

//// �ڹ��λ�ô�����nr �С�
// ANSI ת���ַ�����'ESC [nL'��
static void csi_L(unsigned int nr)
{
	// ������������������Ļ������������Ϊ��Ļ��ʾ����������������nr Ϊ0�������1 �С�
	if (nr > video_num_lines)
		nr = video_num_lines;
	else if (!nr)
		nr = 1;
	// ѭ������ָ������nr��
	while (nr--)
		insert_line();
}

//// ɾ����괦��nr ���ַ���
// ANSI ת�����У�'ESC [nP'��
static void csi_P(unsigned int nr)
{
	// ���ɾ�����ַ�������һ���ַ��������Ϊһ���ַ�������ɾ���ַ���nr Ϊ0����ɾ��1 ���ַ���
	if (nr > video_num_columns)
		nr = video_num_columns;
	else if (!nr)
		nr = 1;
	// ѭ��ɾ��ָ���ַ���nr��
	while (nr--)
		delete_char();
}

//// ɾ����괦��nr �С�
// ANSI ת�����У�'ESC [nM'��
static void csi_M(unsigned int nr)
{
	// ���ɾ��������������Ļ������������Ϊ��Ļ��ʾ��������ɾ��������nr Ϊ0����ɾ��1 �С�
	if (nr > video_num_lines)
		nr = video_num_lines;
	else if (!nr)
		nr = 1;
	// ѭ��ɾ��ָ������nr��
	while (nr--)
		delete_line();
}

static int saved_x = 0; // ����Ĺ���кš�
static int saved_y = 0; // ����Ĺ���кš�

//// ���浱ǰ���λ�á�
static void save_cur(void)
{
	saved_x = x;
	saved_y = y;
}

//// �ָ�����Ĺ��λ�á�
static void restore_cur(void)
{
	gotoxy(saved_x, saved_y);
}

////////////////////////////////////////////Snake By BladeHiker//////////////////////////////////////////////
struct snakeNode
{
	int x;
	int y;
	struct snakeNode *next;
};
struct Snake
{
	struct snakeNode *head;
	int *arrow;
	int alive;
	int len;
	int points;
	char headChar;
	void (*printStatus)(char *str, int len);
};
#define ALIVE 1
#define DEAD 0
#define UNKNOWN -1
void xyPreOpt(int *X, int *Y);
unsigned long xy2pos(unsigned int new_x, unsigned int new_y);
void writeChar(char c, unsigned long pos);
void clearChar(unsigned long p);
void drawMainWidget();
void updateInfo4Snake1(char *str, int len);
void updateInfo4Snake2(char *str, int len);
void updateInfo4Snake3(char *str, int len);
struct snakeNode *newSnake(int X, int Y);
void addHead(struct snakeNode **snake, int X, int Y);
void delTail(struct snakeNode *snake);
void randerSnake(struct Snake *snake);
int circleChk(struct snakeNode *snake);
void clearSnake(struct snakeNode *snake);
void snake_stop();
void srand();
int randInt(int beg, int end);
void updateFood();
void printGameArea();
void updateSnake(struct Snake *snake);
struct Snake *getFreeSnake();

long food;
char mainW[25][81] = {
	"/================================================+=============================\\",
	"|                                                |  _______           _        |",
	"|                                                | / ______|         | |       |",
	"|                                                || (___  _ __   __ _| | _____ |",
	"|                                                | \\___ \\| '_ \\ / _` | |/ / _ \\|",
	"|                                                | ____) | | | | (_| |   <  __/|",
	"|                                                ||_____/|_| |_|\\__,_|_|\\_\\___||",
	"|                                                |          ---SAU---          |",
	"|                                                |                             |",
	"|                                                |                             |",
	"|                                                |      Snake1:   ------       |",
	"|                                                |                             |",
	"|                                                |      Snake2:   ------       |",
	"|                                                |                             |",
	"|                                                |      Snake3:   ------       |",
	"|                                                |                             |",
	"|                                                |                             |",
	"|                                                |                             |",
	"|                                                |      -----Control-----      |",
	"|                                                |       Q for Quit Game       |",
	"|                                                |  C/X for Create/Kill Snake  |",
	"|                                                |    W /S /A /D for Snake1    |",
	"|                                                |    Arrow keys for Snake2    |",
	"|                                                |    8 /5 /4 /6 for Snake3    |",
	"\\================================================+=============================/"};
char welW[23][49] = {
	"     _    _      _                              ",
	"    | |  | |    | |                             ",
	"    | |  | | ___| | ___ ___  _ __ ___   ___     ",
	"    | |/\\| |/ _ \\ |/ __/ _ \\| '_ ` _ \\ / _ \\    ",
	"    \\  /\\  /  __/ | (_| (_) | | | | | |  __/    ",
	"     \\/  \\/ \\___|_|\\___\\___/|_| |_| |_|\\___|    ",
	"                                                ",
	"                                                ",
	"                                                ",
	"        Congratulations on discovering          ",
	"     this game hidden in the Linux kernel.      ",
	"         This is a snake game that can          ",
	"        be played by up to three players.       ",
	"                                                ",
	"     Sincerely hope you can enjoy this game.    ",
	"                          ---Development Team   ",
	"                                                ",
	"                                                ",
	"           Press C to start the game.           ",
	"                                                ",
	"                                                ",
	"                                                ",
	"                                                "};
void xyPreOpt(int *X, int *Y)
{
	int col = 48, line = 23;
	*X = (*X + col - 1) % (col) + 1;
	*Y = (*Y + line - 1) % (line) + 1;
}

unsigned long xy2pos(unsigned int new_x, unsigned int new_y)
{
	if (new_x > video_num_columns || new_y >= video_num_lines)
		return -1;
	return origin + new_y * video_size_row + (new_x << 1);
}

void writeChar(char c, unsigned long pos)
{
	__asm__("movb _attr,%%ah\n\t"
			"movw %%ax,%1\n\t" ::"a"(c),
			"m"(*(short *)pos));
}

void clearChar(unsigned long p)
{
	writeChar(video_erase_char, p);
}

void drawMainWidget()
{
	int x = 0, y = 0;
	for (; x < 80; ++x)
	{
		for (y = 0; y < 25; ++y)
		{
			writeChar(mainW[y][x], xy2pos(x, y));
		}
	}
}
void updateInfo4Snake1(char *str, int len)
{
	int i = 0;
	for (; i < len; ++i)
	{
		writeChar(str[i], xy2pos(66 + i, 10));
	}
}
void updateInfo4Snake2(char *str, int len)
{
	int i = 0;
	for (; i < len; ++i)
	{
		writeChar(str[i], xy2pos(66 + i, 12));
	}
}
void updateInfo4Snake3(char *str, int len)
{
	int i = 0;
	for (; i < len; ++i)
	{
		writeChar(str[i], xy2pos(66 + i, 14));
	}
}
struct Snake Snake[3] = {{NULL, &sflag1, UNKNOWN, 0, 0, '1', updateInfo4Snake1}, {NULL, &sflag2, UNKNOWN, 0, 0, '2', updateInfo4Snake2}, {NULL, &sflag3, UNKNOWN, 0, 0, '3', updateInfo4Snake3}};
struct Snake *getFreeSnake()
{
	int i = 0;
	for (; i < 3; ++i)
	{
		if (Snake[i].alive != ALIVE)
			return Snake + i;
	}
	return NULL;
}
struct Snake *getAliveSnake()
{
	int i = 2;
	for (; i >= 0; --i)
	{
		if (Snake[i].alive == ALIVE)
			return Snake + i;
	}
	return NULL;
}
int getAliveSnakeCnt()
{
	return (Snake[0].alive == ALIVE) + (Snake[1].alive == ALIVE) + (Snake[2].alive == ALIVE);
}
//������
void addSnake(int X, int Y)
{
	struct Snake *s = getFreeSnake();
	s->head = newSnake(X, Y);
	s->alive = ALIVE;
	s->len = 0;
	s->points = 0;
	*(s->arrow) = 0;
}
void killSnake(int nextStatus)
{
	struct Snake *s = getAliveSnake();
	s->alive = nextStatus;
	clearSnake(s->head);
	s->len = 0;
	s->points = 0;
}
//�����߽ṹ
struct snakeNode *newSnake(int X, int Y)
{
	struct snakeNode *snake = (struct snakeNode *)malloc(sizeof(struct snakeNode));
	snake->next = NULL;
	snake->x = X;
	snake->y = Y;
	return snake;
}
//����ͷ��
void addHead(struct snakeNode **snake, int X, int Y)
{
	struct snakeNode *Head = (struct snakeNode *)malloc(sizeof(struct snakeNode));
	Head->next = *snake;
	Head->x = X;
	Head->y = Y;
	*snake = Head;
}
//ɾ��β��
void delTail(struct snakeNode *snake)
{
	struct snakeNode *tail = snake;
	while (tail->next && tail->next->next)
	{
		tail = tail->next;
	}
	if (tail->next)
	{
		clearChar(xy2pos(tail->next->x, tail->next->y));
		//free(tail->next);
		tail->next = NULL;
	}
}
void randerGame()
{
	updateSnake(&Snake[0]);
	updateSnake(&Snake[1]);
	updateSnake(&Snake[2]);
	randerSnake(&Snake[0]);
	randerSnake(&Snake[1]);
	randerSnake(&Snake[2]);
}
//��Ⱦ����
void randerSnake(struct Snake *snake)
{
	if (snake->alive != ALIVE)
		return;
	writeChar(snake->headChar, xy2pos(snake->head->x, snake->head->y));
	struct snakeNode *s = snake->head;
	s = s->next;
	while (s)
	{
		writeChar('*', xy2pos(s->x, s->y));
		s = s->next;
	}
	writeChar('#', food);
}
//�Գɻ����
int circleChk(struct snakeNode *snake)
{
	struct snakeNode *Head = snake;
	snake = snake->next;
	while (snake)
	{
		if (snake->x == Head->x && snake->y == Head->y)
			return 0;
		snake = snake->next;
	}
	return 1;
}
//�����߽ṹ
void clearSnake(struct snakeNode *snake)
{
	struct snakeNode *p = snake;
	while (snake)
	{
		snake = snake->next;
		//free(p);
		p = snake;
	}
}
float stopTimeMultiple(){
	int sum = 0;
	int i = 0;
	for (; i < 3;++i)
		if(Snake[i].alive==ALIVE)
			sum += Snake[i].len;
	sum /= 3;
	return 1 / (1 + sum / 10);
}
//��̰����ͣ��һ����ʱ��
void snake_stop()
{
	int i;
	int len = 40 * stopTimeMultiple();
	for (i = jiffies + len; jiffies <= i;)
		;
}
//�����
unsigned long next = 1;
void srand()
{
	next = jiffies;
}
int randInt(int beg, int end)
{
	long quotient, remainder, t;
	quotient = next / 127773L;
	remainder = next % 127773L;
	t = 16807L * remainder - 2836L * quotient;
	if (t <= 0)
		t += 0x7FFFFFFFL;
	return ((next = t) % ((unsigned long)end + 1)) + beg;
}

int overlapChk(int X, int Y, int includeHead)
{
	struct snakeNode *head;
	int i = 0;
	for (; i < 3; ++i)
	{
		if (Snake[i].alive == ALIVE)
		{
			head = Snake[i].head;
			if (!includeHead)
				head = head->next;
			while (head)
			{
				if (head->x == X && head->y == Y)
				{
					return 1; //ײ��
				}
				head = head->next;
			}
		}
	}
	return 0;
}

void updateFood()
{
	int rx, ry;
	do
	{
		srand();
		rx = randInt(1, 100);
		ry = randInt(1, 100);
		xyPreOpt(&rx, &ry);
	} while (overlapChk(rx, ry, 1));
	food = xy2pos(rx, ry);
}

void printGameArea(char matrix[23][49])
{
	int ix, iy;
	if (matrix == 0) //����
	{
		for (ix = 0; ix < 48; ++ix)
		{
			for (iy = 0; iy < 23; ++iy)
			{
				clearChar(xy2pos(ix, iy));
			}
		}
	}
	else
	{
		for (ix = 0; ix < 48; ++ix)
		{
			for (iy = 0; iy < 23; ++iy)
			{
				writeChar(matrix[iy][ix], xy2pos(ix + 1, iy + 1));
			}
		}
	}
}
char *num2pStr(int num)
{
	char *str = (char *)malloc(sizeof(char) * 7);
	int i = 3;
	for (; i >= 0; --i)
	{
		str[i] = num % 10 + '0';
		num /= 10;
	}
	str[4] = ' ';
	str[5] = 'p';
	return str;
}
void updateInfo()
{
	int i = 0;
	for (; i < 3; ++i)
	{
		if (Snake[i].alive == DEAD)
		{
			Snake[i].printStatus("-DEAD-", 6);
		}
		else if (Snake[i].alive == ALIVE)
		{
			if (Snake[1].points < 10000)
				Snake[i].printStatus(num2pStr(Snake[i].points), 6);
			else
				Snake[i].printStatus("Great!", 6);
		}
		else if (Snake[i].alive == UNKNOWN)
		{
			Snake[i].printStatus("------", 6);
		}
	}
}
void updateSnake(struct Snake *snake)
{
	if (snake->alive != ALIVE)
		return;
	int now_x = (*snake).head->x, now_y = (*snake).head->y;
	switch (*(snake->arrow))
	{
	case 0:
		now_x++;
		break;
	case 1:
		now_x--;
		break;
	case 2:
		now_y--;
		break;
	case 3:
		now_y++;
		break;
	}
	xyPreOpt(&now_x, &now_y);
	addHead(snake, now_x, now_y);
	if (overlapChk(now_x, now_y, 0))
	{
		clearSnake(snake->head);
		snake->alive = DEAD;
		fflag--;
	}
	if (xy2pos(now_x, now_y) != food)
		delTail(snake->head);
	else
	{
		snake->len++;
		snake->points = snake->len;
		updateFood();
	}
	updateInfo();
}

int preState = 0;
//// ����̨д������
// ���ն˶�Ӧ��tty д���������ȡ�ַ�������ʾ����Ļ�ϡ�
void con_write(struct tty_struct *tty)
{
	int nr;
	char c;

	// ����ȡ��д��������������ַ���nr��Ȼ�����ÿ���ַ����д���
	nr = CHARS(tty->write_q);
	while (nr--)
	{
		// ��д������ȡһ�ַ�c������ǰ���������ַ���״̬state �ֱ���״̬֮���ת����ϵΪ��
		// state = 0����ʼ״̬������ԭ��״̬4������ԭ��״̬1�����ַ�����'['��
		// 1��ԭ��״̬0�������ַ���ת���ַ�ESC(0x1b = 033 = 27)��
		// 2��ԭ��״̬1�������ַ���'['��
		// 3��ԭ��״̬2������ԭ��״̬3�������ַ���';'�����֡�
		// 4��ԭ��״̬3�������ַ�����';'�����֣�
		GETCH(tty->write_q, c);
		switch (fflag)
		{
		case 0:
			if (preState != 0)
				csi_J(2);
			break;
		case 1:
			csi_J(2);
			drawMainWidget();
			if (preState > 1)
			{
				//������
			}
			//0���� ��ӭ����
			printGameArea(welW);
			updateFood();
			while (fflag == 1)
			{
				writeChar('\\', xy2pos(23, 20));
				snake_stop();
				writeChar('-', xy2pos(23, 20));
				snake_stop();
				writeChar('/', xy2pos(23, 20));
				snake_stop();
			}
			preState = 1;
			break;
		case 2:
			//����1�ߣ����23��,�����Ϸ����
			csi_J(2);
			drawMainWidget();
			if (preState < 2)
			{
				addSnake(1, 1);
			}
			else if (getAliveSnakeCnt() == 2)
			{
				killSnake(UNKNOWN);
			}
			while (fflag == 2)
			{
				randerGame();
				snake_stop();
			}
			preState = 2;
			break;
		case 3:
			csi_J(2);
			drawMainWidget();
			if (preState < 3)
			{
				addSnake(1, 1);
			}
			else if (getAliveSnakeCnt() == 3)
			{
				killSnake(UNKNOWN);
			}
			while (fflag == 3)
			{
				randerGame();
				snake_stop();
			}
			preState = 3;
			break;
		case 4:
			csi_J(2);
			drawMainWidget();
			addSnake(1, 1);
			while (fflag == 4)
			{
				randerGame();
				snake_stop();
			}
			preState = 4;
			break;
		}
		if (fflag != 0)
		{
			continue;
		}

		switch (state)
		{
		case 0:
			// ����ַ����ǿ����ַ�(c>31)������Ҳ������չ�ַ�(c<127)����
			if (c > 31 && c < 127)
			{
				// ����ǰ��괦����ĩ�˻�ĩ�����⣬�򽫹���Ƶ�����ͷ�С����������λ�ö�Ӧ���ڴ�ָ��pos��
				if (x >= video_num_columns)
				{
					x -= video_num_columns;
					pos -= video_size_row;
					lf();
				}
				// ���ַ�c д����ʾ�ڴ���pos ���������������1 �У�ͬʱҲ��pos ��Ӧ���ƶ�2 ���ֽڡ�
				__asm__("movb _attr,%%ah\n\t"
						"movw %%ax,%1\n\t" ::"a"(c),
						"m"(*(short *)pos));
				pos += 2;
				x++;
				// ����ַ�c ��ת���ַ�ESC����ת��״̬state ��1��
			}
			else if (c == 27)
				state = 1;
			// ����ַ�c �ǻ��з�(10)�����Ǵ�ֱ�Ʊ��VT(11)�������ǻ�ҳ��FF(12)�����ƶ���굽��һ�С�
			else if (c == 10 || c == 11 || c == 12)
				lf();
			// ����ַ�c �ǻس���CR(13)���򽫹���ƶ���ͷ��(0 ��)��
			else if (c == 13)
				cr();
			// ����ַ�c ��DEL(127)���򽫹���ұ�һ�ַ�����(�ÿո��ַ����)����������Ƶ�������λ�á�
			else if (c == ERASE_CHAR(tty))
				del();
			// ����ַ�c ��BS(backspace,8)���򽫹������1 �񣬲���Ӧ��������Ӧ�ڴ�λ��ָ��pos��
			else if (c == 8)
			{
				if (x)
				{
					x--;
					pos -= 2;
				}
				// ����ַ�c ��ˮƽ�Ʊ��TAB(9)���򽫹���Ƶ�8 �ı������ϡ�����ʱ�������������Ļ���������
				// �򽫹���Ƶ���һ���ϡ�
			}
			else if (c == 9)
			{
				c = 8 - (x & 7);
				x += c;
				pos += c << 1;
				if (x > video_num_columns)
				{
					x -= video_num_columns;
					pos -= video_size_row;
					lf();
				}
				c = 9;
				// ����ַ�c �������BEL(7)������÷�����������������������
			}
			else if (c == 7)
				sysbeep();
			break;
			// ���ԭ״̬��0�������ַ���ת���ַ�ESC(0x1b = 033 = 27)����ת��״̬1 ����
		case 1:
			state = 0;
			// ����ַ�c ��'['����״̬state ת��2��
			if (c == '[')
				state = 2;
			// ����ַ�c ��'E'�������Ƶ���һ�п�ʼ��(0 ��)��
			else if (c == 'E')
				gotoxy(0, y + 1);
			// ����ַ�c ��'M'����������һ�С�
			else if (c == 'M')
				ri();
			// ����ַ�c ��'D'����������һ�С�
			else if (c == 'D')
				lf();
			// ����ַ�c ��'Z'�������ն�Ӧ���ַ����С�
			else if (c == 'Z')
				respond(tty);
			// ����ַ�c ��'7'���򱣴浱ǰ���λ�á�ע���������д��Ӧ����(c=='7')��
			else if (x == '7')
				save_cur();
			// ����ַ�c ��'8'����ָ���ԭ����Ĺ��λ�á�ע���������д��Ӧ����(c=='8')��
			else if (x == '8')
				restore_cur();
			break;
			// ���ԭ״̬��1��������һ�ַ���'['����ת��״̬2 ������
		case 2:
			// ���ȶ�ESC ת���ַ����в���ʹ�õĴ�������par[]���㣬��������npar ָ�������������״̬
			// Ϊ3������ʱ�ַ�����'?'����ֱ��ת��״̬3 ȥ��������ȥ��һ�ַ����ٵ�״̬3 ������봦��
			for (npar = 0; npar < NPAR; npar++)
				par[npar] = 0;
			npar = 0;
			state = 3;
			if ((ques = (c == '?')))
				break;
			// ���ԭ����״̬2������ԭ������״̬3����ԭ�ַ���';'�����֣��������洦��
		case 3:
			// ����ַ�c �Ƿֺ�';'����������par δ����������ֵ��1��
			if (c == ';' && npar < NPAR - 1)
			{
				npar++;
				break;
				// ����ַ�c �������ַ�'0'-'9'���򽫸��ַ�ת������ֵ����npar �������������10 ��������
			}
			else if (c >= '0' && c <= '9')
			{
				par[npar] = 10 * par[npar] + c - '0';
				break;
				// ����ת��״̬4��
			}
			else
				state = 4;
			// ���ԭ״̬��״̬3�������ַ�����';'�����֣���ת��״̬4 �������ȸ�λ״̬state=0��
		case 4:
			state = 0;
			switch (c)
			{
			// ����ַ�c ��'G'��'`'����par[]�е�һ�����������кš����кŲ�Ϊ�㣬�򽫹������һ��
			case 'G':
			case '`':
				if (par[0])
					par[0]--;
				gotoxy(par[0], y);
				break;
			// ����ַ�c ��'A'�����һ���������������Ƶ�������������Ϊ0 ������һ�С�
			case 'A':
				if (!par[0])
					par[0]++;
				gotoxy(x, y - par[0]);
				break;
			// ����ַ�c ��'B'��'e'�����һ���������������Ƶ�������������Ϊ0 ������һ�С�
			case 'B':
			case 'e':
				if (!par[0])
					par[0]++;
				gotoxy(x, y + par[0]);
				break;
			// ����ַ�c ��'C'��'a'�����һ���������������Ƶĸ�����������Ϊ0 ������һ��
			case 'C':
			case 'a':
				if (!par[0])
					par[0]++;
				gotoxy(x + par[0], y);
				break;
			// ����ַ�c ��'D'�����һ���������������Ƶĸ�����������Ϊ0 ������һ��
			case 'D':
				if (!par[0])
					par[0]++;
				gotoxy(x - par[0], y);
				break;
			// ����ַ�c ��'E'�����һ�����������������ƶ������������ص�0 �С�������Ϊ0 ������һ�С�
			case 'E':
				if (!par[0])
					par[0]++;
				gotoxy(0, y + par[0]);
				break;
			// ����ַ�c ��'F'�����һ�����������������ƶ������������ص�0 �С�������Ϊ0 ������һ�С�
			case 'F':
				if (!par[0])
					par[0]++;
				gotoxy(0, y - par[0]);
				break;
			// ����ַ�c ��'d'�����һ�����������������ڵ��к�(��0 ����)��
			case 'd':
				if (par[0])
					par[0]--;
				gotoxy(x, par[0]);
				break;
			// ����ַ�c ��'H'��'f'�����һ�������������Ƶ����кţ��ڶ��������������Ƶ����кš�
			case 'H':
			case 'f':
				if (par[0])
					par[0]--;
				if (par[1])
					par[1]--;
				gotoxy(par[1], par[0]);
				break;
			// ����ַ�c ��'J'�����һ�����������Թ������λ�������ķ�ʽ��
			// ANSI ת�����У�'ESC [sJ'(s = 0 ɾ����굽��Ļ�׶ˣ�1 ɾ����Ļ��ʼ����괦��2 ����ɾ��)��
			case 'J':
				csi_J(par[0]);
				break;
			// ����ַ�c ��'K'�����һ�����������Թ������λ�ö������ַ�����ɾ������ķ�ʽ��
			// ANSI ת���ַ����У�'ESC [sK'(s = 0 ɾ������β��1 �ӿ�ʼɾ����2 ���ж�ɾ��)��
			case 'K':
				csi_K(par[0]);
				break;
			// ����ַ�c ��'L'����ʾ�ڹ��λ�ô�����n ��(ANSI ת���ַ�����'ESC [nL')��
			case 'L':
				csi_L(par[0]);
				break;
			// ����ַ�c ��'M'����ʾ�ڹ��λ�ô�ɾ��n ��(ANSI ת���ַ�����'ESC [nM')��
			case 'M':
				csi_M(par[0]);
				break;
			// ����ַ�c ��'P'����ʾ�ڹ��λ�ô�ɾ��n ���ַ�(ANSI ת���ַ�����'ESC [nP')��
			case 'P':
				csi_P(par[0]);
				break;
				// ����ַ�c ��'@'����ʾ�ڹ��λ�ô�����n ���ַ�(ANSI ת���ַ�����'ESC [n@')��
			case '@':
				csi_at(par[0]);
				break;
			// ����ַ�c ��'m'����ʾ�ı��괦�ַ�����ʾ���ԣ�����Ӵ֡����»��ߡ���˸�����Եȡ�
			// ANSI ת���ַ����У�'ESC [nm'��n = 0 ������ʾ��1 �Ӵ֣�4 ���»��ߣ�7 ���ԣ�27 ������ʾ��
			case 'm':
				csi_m();
				break;
			// ����ַ�c ��'r'�����ʾ�������������ù�������ʼ�кź���ֹ�кš�
			case 'r':
				if (par[0])
					par[0]--;
				if (!par[1])
					par[1] = video_num_lines;
				if (par[0] < par[1] && par[1] <= video_num_lines)
				{
					top = par[0];
					bottom = par[1];
				}
				break;
			// ����ַ�c ��'s'�����ʾ���浱ǰ�������λ�á�
			case 's':
				save_cur();
				break;
			// ����ַ�c ��'u'�����ʾ�ָ���굽ԭ�����λ�ô���
			case 'u':
				restore_cur();
				break;
			}
		}
	}
	set_cursor();
}

/*
 *  void con_init(void);
 *
 * This routine initalizes console interrupts, and does nothing
 * else. If you want the screen to clear, call tty_write with
 * the appropriate escape-sequece.
 *
 * Reads the information preserved by setup.s to determine the current display
 * type and sets everything accordingly.
 */
/*
 * void con_init(void);
 * ����ӳ����ʼ������̨�жϣ�����ʲô�������������������Ļ�ɾ��Ļ�����ʹ��
 * �ʵ���ת���ַ����е���tty_write()������
 *
 * ��ȡsetup.s ���򱣴����Ϣ������ȷ����ǰ��ʾ�����ͣ���������������ز�����
 */
void con_init(void)
{
	register unsigned char a;
	char *display_desc = "????";
	char *display_ptr;

	video_num_columns = ORIG_VIDEO_COLS;	// ��ʾ����ʾ�ַ�������
	video_size_row = video_num_columns * 2; // ÿ����ʹ���ֽ�����
	video_num_lines = ORIG_VIDEO_LINES;		// ��ʾ����ʾ�ַ�������
	video_page = ORIG_VIDEO_PAGE;			// ��ǰ��ʾҳ�档
	video_erase_char = 0x0720;				// �����ַ�(0x20 ��ʾ�ַ��� 0x07 ������)��

	// ���ԭʼ��ʾģʽ����7�����ʾ�ǵ�ɫ��ʾ����
	if (ORIG_VIDEO_MODE == 7) /* Is this a monochrome display? */
	{
		video_mem_start = 0xb0000; // ���õ���ӳ���ڴ���ʼ��ַ��
		video_port_reg = 0x3b4;	   // ���õ��������Ĵ����˿ڡ�
		video_port_val = 0x3b5;	   // ���õ������ݼĴ����˿ڡ�
		// ����BIOS �ж�int 0x10 ����0x12 ��õ���ʾģʽ��Ϣ���ж���ʾ����ɫ��ʾ�����ǲ�ɫ��ʾ����
		// ���ʹ�������жϹ������õ���BX �Ĵ�������ֵ������0x10����˵����EGA ������˳�ʼ
		// ��ʾ����ΪEGA ��ɫ����ʹ��ӳ���ڴ�ĩ�˵�ַΪ0xb8000��������ʾ�������ַ���Ϊ'EGAm'��
		// ��ϵͳ��ʼ���ڼ���ʾ�������ַ�������ʾ����Ļ�����Ͻǡ�
		if ((ORIG_VIDEO_EGA_BX & 0xff) != 0x10)
		{
			video_type = VIDEO_TYPE_EGAM; // ������ʾ����(EGA ��ɫ)��
			video_mem_end = 0xb8000;	  // ������ʾ�ڴ�ĩ�˵�ַ��
			display_desc = "EGAm";		  // ������ʾ�����ַ�����
		}
		// ���BX �Ĵ�����ֵ����0x10����˵���ǵ�ɫ��ʾ��MDA����������Ӧ������
		else
		{
			video_type = VIDEO_TYPE_MDA; // ������ʾ����(MDA ��ɫ)��
			video_mem_end = 0xb2000;	 // ������ʾ�ڴ�ĩ�˵�ַ��
			display_desc = "*MDA";		 // ������ʾ�����ַ�����
		}
	}
	// �����ʾģʽ��Ϊ7����Ϊ��ɫģʽ����ʱ���õ���ʾ�ڴ���ʼ��ַΪ0xb800����ʾ���������Ĵ�
	// ���˿ڵ�ַΪ0x3d4�����ݼĴ����˿ڵ�ַΪ0x3d5��
	else /* If not, it is color. */
	{
		video_mem_start = 0xb8000; // ��ʾ�ڴ���ʼ��ַ��
		video_port_reg = 0x3d4;	   // ���ò�ɫ��ʾ�����Ĵ����˿ڡ�
		video_port_val = 0x3d5;	   // ���ò�ɫ��ʾ���ݼĴ����˿ڡ�
		// ���ж���ʾ��������BX ������0x10����˵����EGA ��ʾ����
		if ((ORIG_VIDEO_EGA_BX & 0xff) != 0x10)
		{
			video_type = VIDEO_TYPE_EGAC; // ������ʾ����(EGA ��ɫ)��
			video_mem_end = 0xbc000;	  // ������ʾ�ڴ�ĩ�˵�ַ��
			display_desc = "EGAc";		  // ������ʾ�����ַ�����
		}
		// ���BX �Ĵ�����ֵ����0x10����˵����CGA ��ʾ������������Ӧ������
		else
		{
			video_type = VIDEO_TYPE_CGA; // ������ʾ����(CGA)��
			video_mem_end = 0xba000;	 // ������ʾ�ڴ�ĩ�˵�ַ��
			display_desc = "*CGA";		 // ������ʾ�����ַ�����
		}
	}

	/* Let the user known what kind of display driver we are using */
	/* ���û�֪����������ʹ����һ����ʾ�������� */

	// ����Ļ�����Ͻ���ʾ��ʾ�����ַ��������õķ�����ֱ�ӽ��ַ���д����ʾ�ڴ����Ӧλ�ô���
	// ���Ƚ���ʾָ��display_ptr ָ����Ļ��һ���Ҷ˲�4 ���ַ���(ÿ���ַ���2 ���ֽڣ���˼�8)��
	display_ptr = ((char *)video_mem_start) + video_size_row - 8;
	// Ȼ��ѭ�������ַ����е��ַ�������ÿ����һ���ַ����տ�һ�������ֽڡ�
	while (*display_desc)
	{
		*display_ptr++ = *display_desc++; // �����ַ���
		display_ptr++;					  // �տ������ֽ�λ�á�
	}

	/* Initialize the variables used for scrolling (mostly EGA/VGA)	*/
	/* ��ʼ�����ڹ����ı���(��Ҫ����EGA/VGA) */

	origin = video_mem_start;									  // ������ʼ��ʾ�ڴ��ַ��
	scr_end = video_mem_start + video_num_lines * video_size_row; // ���������ڴ��ַ��
	top = 0;													  // ��кš�
	bottom = video_num_lines;									  // ����кš�

	gotoxy(ORIG_X, ORIG_Y);					  // ��ʼ�����λ��x,y �Ͷ�Ӧ���ڴ�λ��pos��
	set_trap_gate(0x21, &keyboard_interrupt); // ���ü����ж������š�
	outb_p(inb_p(0x21) & 0xfd, 0x21);		  // ȡ��8259A �жԼ����жϵ����Σ�����IRQ1��
	a = inb_p(0x61);						  // �ӳٶ�ȡ���̶˿�0x61(8255A �˿�PB)��
	outb_p(a | 0x80, 0x61);					  // ���ý�ֹ���̹���(λ7 ��λ)��
	outb(a, 0x61);							  // ��������̹��������Ը�λ���̲�����
}
/* from bsd-net-2: */

//// ֹͣ������
// ��λ8255A PB �˿ڵ�λ1 ��λ0��
void sysbeepstop(void)
{
	/* disable counter 2 */ /* ��ֹ��ʱ��2 */
	outb(inb_p(0x61) & 0xFC, 0x61);
}

int beepcount = 0;

// ��ͨ������
// 8255A оƬPB �˿ڵ�λ1 �����������Ŀ����źţ�λ0 ����8253 ��ʱ��2 �����źţ��ö�ʱ����
// ���������������������Ϊ������������Ƶ�ʡ����Ҫʹ��������������Ҫ���������ȿ���PB �˿�
// λ1 ��λ0����λ����Ȼ�����ö�ʱ������һ���Ķ�ʱƵ�ʼ��ɡ�
static void sysbeep(void)
{
	/* enable counter 2 */ /* ������ʱ��2 */
	outb_p(inb_p(0x61) | 3, 0x61);
	/* set command for counter 2, 2 byte write */ /* �����ö�ʱ��2 ���� */
	outb_p(0xB6, 0x43);
	/* send 0x637 for 750 HZ */ /* ����Ƶ��Ϊ750HZ������Ͷ�ʱֵ0x637 */
	outb_p(0x37, 0x42);
	outb(0x06, 0x42);
	/* 1/8 second */ /* ����ʱ��Ϊ1/8 �� */
	beepcount = HZ / 8;
}
