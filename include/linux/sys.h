extern int sys_setup ();		// ϵͳ������ʼ�����ú����� (kernel/blk_drv/hd.c)
extern int sys_exit ();			// �����˳��� (kernel/exit.c)
extern int sys_fork ();			// �������̡� (kernel/system_call.s)
extern int sys_read ();			// ���ļ��� (fs/read_write.c)
extern int sys_write ();		// д�ļ��� (fs/read_write.c)
extern int sys_open ();			// ���ļ��� (fs/open.c)
extern int sys_close ();		// �ر��ļ��� (fs/open.c)
extern int sys_waitpid ();		// �ȴ�������ֹ�� (kernel/exit.c)
extern int sys_creat ();		// �����ļ��� (fs/open.c)
extern int sys_link ();			// ����һ���ļ���Ӳ���ӡ� (fs/namei.c)
extern int sys_unlink ();		// ɾ��һ���ļ���(��ɾ���ļ�)�� (fs/namei.c)
extern int sys_execve ();		// ִ�г��� (kernel/system_call.s)
extern int sys_chdir ();		// ���ĵ�ǰĿ¼�� (fs/open.c)
extern int sys_time ();			// ȡ��ǰʱ�䡣 (kernel/sys.c)
extern int sys_mknod ();		// ������/�ַ������ļ��� (fs/namei.c)
extern int sys_chmod ();		// �޸��ļ����ԡ� (fs/open.c)
extern int sys_chown ();		// �޸��ļ������������顣 (fs/open.c)
extern int sys_break ();		// (-kernel/sys.c)
extern int sys_stat ();			// ʹ��·����ȡ�ļ���״̬��Ϣ�� (fs/stat.c)
extern int sys_lseek ();		// ���¶�λ��/д�ļ�ƫ�ơ� (fs/read_write.c)
extern int sys_getpid ();		// ȡ����id�� (kernel/sched.c)
extern int sys_mount ();		// ��װ�ļ�ϵͳ�� (fs/super.c)
extern int sys_umount ();		// ж���ļ�ϵͳ�� (fs/super.c)
extern int sys_setuid ();		// ���ý����û�id�� (kernel/sys.c)
extern int sys_getuid ();		// ȡ�����û�id�� (kernel/sched.c)
extern int sys_stime ();		// ����ϵͳʱ�����ڡ� (-kernel/sys.c)
extern int sys_ptrace ();		// ������ԡ� (-kernel/sys.c)
extern int sys_alarm ();		// ���ñ����� (kernel/sched.c)
extern int sys_fstat ();		// ʹ���ļ����ȡ�ļ���״̬��Ϣ��(fs/stat.c)
extern int sys_pause ();		// ��ͣ�������С� (kernel/sched.c)
extern int sys_utime ();		// �ı��ļ��ķ��ʺ��޸�ʱ�䡣 (fs/open.c)
extern int sys_stty ();			// �޸��ն������á� (-kernel/sys.c)
extern int sys_gtty ();			// ȡ�ն���������Ϣ�� (-kernel/sys.c)
extern int sys_access ();		// ����û���һ���ļ��ķ���Ȩ�ޡ�(fs/open.c)
extern int sys_nice ();			// ���ý���ִ������Ȩ�� (kernel/sched.c)
extern int sys_ftime ();		// ȡ���ں�ʱ�䡣 (-kernel/sys.c)
extern int sys_sync ();			// ͬ�����ٻ������豸�����ݡ� (fs/buffer.c)
extern int sys_kill ();			// ��ֹһ�����̡� (kernel/exit.c)
extern int sys_rename ();		// �����ļ����� (-kernel/sys.c)
extern int sys_mkdir ();		// ����Ŀ¼�� (fs/namei.c)
extern int sys_rmdir ();		// ɾ��Ŀ¼�� (fs/namei.c)
extern int sys_dup ();			// �����ļ������ (fs/fcntl.c)
extern int sys_pipe ();			// �����ܵ��� (fs/pipe.c)
extern int sys_times ();		// ȡ����ʱ�䡣 (kernel/sys.c)
extern int sys_prof ();			// ����ִ��ʱ������ (-kernel/sys.c)
extern int sys_brk ();			// �޸����ݶγ��ȡ� (kernel/sys.c)
extern int sys_setgid ();		// ���ý�����id�� (kernel/sys.c)
extern int sys_getgid ();		// ȡ������id�� (kernel/sched.c)
extern int sys_signal ();		// �źŴ��� (kernel/signal.c)
extern int sys_geteuid ();		// ȡ������Ч�û�id�� (kenrl/sched.c)
extern int sys_getegid ();		// ȡ������Ч��id�� (kenrl/sched.c)
extern int sys_acct ();			// ���̼��ʡ� (-kernel/sys.c)
extern int sys_phys ();			// (-kernel/sys.c)
extern int sys_lock ();			// (-kernel/sys.c)
extern int sys_ioctl ();		// �豸���ơ� (fs/ioctl.c)
extern int sys_fcntl ();		// �ļ���������� (fs/fcntl.c)
extern int sys_mpx ();			// (-kernel/sys.c)
extern int sys_setpgid ();		// ���ý�����id�� (kernel/sys.c)
extern int sys_ulimit ();		// (-kernel/sys.c)
extern int sys_uname ();		// ��ʾϵͳ��Ϣ�� (kernel/sys.c)
extern int sys_umask ();		// ȡĬ���ļ����������롣 (kernel/sys.c)
extern int sys_chroot ();		// �ı��ϵͳ�� (fs/open.c)
extern int sys_ustat ();		// ȡ�ļ�ϵͳ��Ϣ�� (fs/open.c)
extern int sys_dup2 ();			// �����ļ������ (fs/fcntl.c)
extern int sys_getppid ();		// ȡ������id�� (kernel/sched.c)
extern int sys_getpgrp ();		// ȡ������id������getpgid(0)��(kernel/sys.c)
extern int sys_setsid ();		// ���»Ự�����г��� (kernel/sys.c)
extern int sys_sigaction ();	// �ı��źŴ�����̡� (kernel/signal.c)
extern int sys_sgetmask ();		// ȡ�ź������롣 (kernel/signal.c)
extern int sys_ssetmask ();		// �����ź������롣 (kernel/signal.c)
extern int sys_setreuid ();		// ������ʵ��/����Ч�û�id�� (kernel/sys.c)
extern int sys_setregid ();		// ������ʵ��/����Ч��id�� (kernel/sys.c)
extern int sys_sigpending();
extern int sys_sigsuspend();
extern int sys_sethostname();
extern int sys_setrlimit();
extern int sys_getrlimit();
extern int sys_getrusage();
extern int sys_gettimeofday();
extern int sys_settimeofday();
extern int sys_getgroups();
extern int sys_setgroups();
extern int sys_select();
extern int sys_symlink();
extern int sys_lstat();
extern int sys_readlink();
extern int sys_uselib();
// ϵͳ���ú���ָ�������ϵͳ�����жϴ������(int 0x80)����Ϊ��ת��
// ����Ԫ��Ϊϵͳ�����ں˺����ĺ���ָ�룬������ϵͳ���ú�
fn_ptr sys_call_table[] = { 
sys_setup,			//0 
sys_exit,           //1   
sys_fork,           //2
sys_read,           //3
sys_write,          //4
sys_open,           //5
sys_close,          //6
sys_waitpid,        //7
sys_creat,          //8
sys_link,           //9
sys_unlink,         //10 
sys_execve,         //11
sys_chdir,          //12
sys_time,           //13
sys_mknod,          //14
sys_chmod,          //15 
sys_chown,          //16 
sys_break,          //17
sys_stat,           //18
sys_lseek,          //19
sys_getpid,         //20
sys_mount,          //21
sys_umount,         //22 
sys_setuid,         //23
sys_getuid,         //24
sys_stime,          //25
sys_ptrace,         //26
sys_alarm,          //27
sys_fstat,          //28 
sys_pause,          //29
sys_utime,          //30
sys_stty,           //31
sys_gtty,           //32 
sys_access,         //33
sys_nice,           //34
sys_ftime,          //35
sys_sync,           //36
sys_kill,           //37
sys_rename,         //38
sys_mkdir,          //39
sys_rmdir,          //40
sys_dup,            //41   
sys_pipe,           //42
sys_times,          //43
sys_prof,           //44
sys_brk,            //45
sys_setgid,         //46
sys_getgid,         //47
sys_signal,         //48
sys_geteuid,        //49
sys_getegid,        //50
sys_acct,           //51
sys_phys,           //52
sys_lock,           //53
sys_ioctl,          //54
sys_fcntl,          //55
sys_mpx,            //56
sys_setpgid,        //57
sys_ulimit,         //58
sys_uname,          //59
sys_umask,          //60
sys_chroot,         //61
sys_ustat,          //62
sys_dup2,           //63
sys_getppid,        //64
sys_getpgrp,        //65
sys_setsid,         //66
sys_sigaction,      //67
sys_sgetmask,       //68
sys_ssetmask,       //69
sys_setreuid,       //70
sys_setregid,       //71
sys_sigsuspend, 	//72
sys_sigpending, 	//73
sys_sethostname,	//74
sys_setrlimit, 		//75
sys_getrlimit, 		//76
sys_getrusage, 		//77
sys_gettimeofday,	//78 
sys_settimeofday, 	//79
sys_getgroups, 		//80
sys_setgroups, 		//81
sys_select, 		//82
sys_symlink,		//83
sys_lstat, 			//84
sys_readlink, 		//85
sys_uselib 			//86 
};
