#ifndef _CONST_H
#define _CONST_H

#define BUFFER_END 0x200000				// ���建��ʹ���ڴ��ĩ�ˣ�������û��ʹ�øó�����

// i�ڵ����ݽṹ�� i_mode �ֶεĸ���־λ
#define I_TYPE          0170000			// ָ��i�ڵ����ͣ����������룩
#define I_DIRECTORY	0040000				//����Ŀ¼�ļ���
#define I_REGULAR       0100000			//  �ǳ����ļ�������Ŀ¼�ļ��������ļ�
#define I_BLOCK_SPECIAL 0060000			//  �ǿ��豸�����ļ�
#define I_CHAR_SPECIAL  0020000			//  ���ַ��豸�����ļ�
#define I_NAMED_PIPE	0010000			//  �������ܵ��ڵ�
#define I_SET_UID_BIT   0004000			//  ��ִ��ʱ������Ч�û�ID����
#define I_SET_GID_BIT   0002000			//  ��ִ��ʱ������Ч�� ID����

#endif
