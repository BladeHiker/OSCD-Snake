/*
 *  linux/lib/_exit.c
 *
 *  (C) 1991  Linus Torvalds
 */

#define __LIBRARY__				// ����һ�����ų�����������˵����
#include <unistd.h>				// Linux ��׼ͷ�ļ��������˸��ַ��ų��������ͣ��������˸��ֺ�����
								// �綨����__LIBRARY__���򻹰���ϵͳ���úź���Ƕ���_syscall0()�ȡ�

//// �ں�ʹ�õĳ���(�˳�)��ֹ������
// ֱ�ӵ���ϵͳ�ж�int 0x80�����ܺ�__NR_exit��
// ������exit_code - �˳��롣
volatile void _exit(int exit_code)
{	
	__asm("INT $0x03");
	// %0 - eax(ϵͳ���ú�__NR_exit)��%1 - ebx(�˳���exit_code)��
	__asm__("int $0x80"::"a" (__NR_exit),"b" (exit_code));
}
