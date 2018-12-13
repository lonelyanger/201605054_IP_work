/*为了方便直接改成.c文件了*/
/*这里直接删掉了包含文件和变量的定义*/
static void cleanup();/*这个函数调用了各个部件的cleanup函数，使用close关闭各种接口，用来清空*/

struct option longopts[] = {
   /*命令缩写替代*/
};

void usage(int status)
{
   /*打印使用帮助*/
}

int set_kernel_options()
{
    /*设置内核操作*/
}

int find_default_gw(void)
{
    /*寻找默认网关*/
}

/*
 * Returns information on a network interface given its name...
 */
struct sockaddr_in *get_if_info(char *ifname, int type)
{
   /*根据名字和类型寻找相应接口，返回其信息*/
}

/* This will limit the number of handler functions we can have for
   sockets and file descriptors and so on... */
#define CALLBACK_FUNCS 5
static struct callback {/*callbac结构体定义，包含描述符和函数*/
    int fd;
    callback_func_t func;
} callbacks[CALLBACK_FUNCS];

static int nr_callbacks = 0;

int attach_callback_func(int fd, callback_func_t func)/*设置callback数组元素中的函数和描述符*/
{
    if (nr_callbacks >= CALLBACK_FUNCS) {
	fprintf(stderr, "callback attach limit reached!!\n");
	exit(-1);
    }
    callbacks[nr_callbacks].fd = fd;
    callbacks[nr_callbacks].func = func;
    nr_callbacks++;
    return 0;
}

/* Here we find out how to load the kernel modules... If the modules
   are located in the current directory. use those. Otherwise fall
   back to modprobe. */

void load_modules(char *ifname)
{
    /*装载内核模块*/
}

void remove_modules(void)
{
	/*删除某个模块*/
}

void host_init(char *ifname)
{
    /*初始化某个端口*/
}

/* This signal handler ensures clean exits */
void signal_handler(int type)
{
	/*信号处理器*/
    switch (type) {
    case SIGSEGV:
	alog(LOG_ERR, 0, __FUNCTION__, "SEGMENTATION FAULT!!!! Exiting!!! "
	     "To get a core dump, compile with DEBUG option.");
    case SIGINT:
    case SIGHUP:
    case SIGTERM:
    default:
	exit(0);
    }
}

int main(int argc, char **argv)
{
    /*初始化各种变量结构体*/

    /* Parse command line: */
    while (1) {                                                                         /*把命令参数读出并执行完*/
	/*这段永真循环处理传入的命令行参数，根据不同参数处理不同命令*/
    }
    /* Check that we are running as root */
    if (geteuid() != 0) {/*是不是root权限*/
	fprintf(stderr, "must be root\n");
	exit(1);
    }

    /* Detach from terminal */
    if (daemonize) {/*是不是标准输入输出*/
	if (fork() != 0)
	    exit(0);
	/* Close stdin, stdout and stderr... */
	/*  close(0); */
	close(1);
	close(2);
	setsid();
    }
    /* Make sure we cleanup at exit... */
    atexit((void *) &cleanup);

    /* Initialize data structures and services... *//*各种初始化*/
    rt_table_init();
    log_init();
    /*   packet_queue_init(); */
    host_init(ifname);
    /*   packet_input_init(); */
    nl_init();
    nl_send_conf_msg();
    aodv_socket_init();
#ifdef LLFEEDBACK
    if (llfeedback) {
	llf_init();
    }
#endif

    /* Set sockets to watch... */
    FD_ZERO(&readers);
    for (i = 0; i < nr_callbacks; i++) {
	FD_SET(callbacks[i].fd, &readers);
	if (callbacks[i].fd >= nfds)
	    nfds = callbacks[i].fd + 1;
    }

    /* Set the wait on reboot timer... */
    if (wait_on_reboot) {
	timer_init(&worb_timer, wait_on_reboot_timeout, &wait_on_reboot);
	timer_set_timeout(&worb_timer, DELETE_PERIOD);
	alog(LOG_NOTICE, 0, __FUNCTION__,
	     "In wait on reboot for %d milliseconds. Disable with \"-D\".",
	     DELETE_PERIOD);
    }

    /* Schedule the first Hello */
    if (!optimized_hellos && !llfeedback)
	hello_start();

    if (rt_log_interval)
	log_rt_table_init();

    while (1) {                          /*这段永真循环是整个main函数的核心*/
	memcpy((char *) &rfds, (char *) &readers, sizeof(rfds));

	timeout = timer_age_queue();
	
	timeout_spec.tv_sec = timeout->tv_sec;
	timeout_spec.tv_nsec = timeout->tv_usec * 1000;

	if ((n = pselect(nfds, &rfds, NULL, NULL, &timeout_spec, &origmask)) < 0) {/*这个pselect函数行使的功能是检查文件描述符集中是否有可读*/
	    if (errno != EINTR)
		alog(LOG_WARNING, errno, __FUNCTION__,
		     "Failed select (main loop)");
	    continue;
	}

	if (n > 0) {                                 /*如果可读，则启动对应callback中设置的函数，而函数的设置则在aodv_socket_init*/
	    for (i = 0; i < nr_callbacks; i++) {		/*llf_init();nl_init();调用的attach_callback_func中显示*/
		if (FD_ISSET(callbacks[i].fd, &rfds)) {
		    /* We don't want any timer SIGALRM's while executing the
		       callback functions, therefore we block the timer... */
		    (*callbacks[i].func) (callbacks[i].fd);
		}
	    }
	}
    }				/* Main loop */
    return 0;
}

static void cleanup(void)
{
    DEBUG(LOG_DEBUG, 0, "CLEANING UP!");
    rt_table_destroy();
    aodv_socket_cleanup();
#ifdef LLFEEDBACK
    if (llfeedback)
	llf_cleanup();
#endif
    log_cleanup();
    nl_cleanup();
    remove_modules();
}
