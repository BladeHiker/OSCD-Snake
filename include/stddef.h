#ifndef _STDDEF_H
#define _STDDEF_H

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef long ptrdiff_t;				// ����ָ��������������.
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long size_t;		// sizeof���ص�����.
#endif

#undef NULL
#define NULL ((void *)0)			// ��ָ��

// ���涨����һ������ĳ��Ա��������ƫ��λ�õĺꡣʹ�øú����ȷ��һ����Ա���ֶΣ�
// �ڰ������Ľṹ�����дӽṹ��ʼ��������ֽ�ƫ��������Ľ��ʱ����λsize_t������
// �������ʽ��������һ�������ַ�������TYPE *��0���ǽ�һ������0����Ͷ�䣨typecast��
// �����ݶ���ָ�����ͣ�Ȼ���ڸý���Ͻ�������.
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#endif
