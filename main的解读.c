/*Ϊ�˷���ֱ�Ӹĳ�.c�ļ���*/
/*����ֱ��ɾ���˰����ļ��ͱ����Ķ���*/
static void cleanup();/*������������˸���������cleanup������ʹ��close�رո��ֽӿڣ��������*/

struct option longopts[] = {
   /*������д���*/
};

void usage(int status)
{
   /*��ӡʹ�ð���*/
}

int set_kernel_options()
{
    /*�����ں˲���*/
}

int find_default_gw(void)
{
    /*Ѱ��Ĭ������*/
}

/*
 * Returns information on a network interface given its name...
 */
struct sockaddr_in *get_if_info(char *ifname, int type)
{
   /*�������ֺ�����Ѱ����Ӧ�ӿڣ���������Ϣ*/
}

/* This will limit the number of handler functions we can have for
   sockets and file descriptors and so on... */
#define CALLBACK_FUNCS 5
static struct callback {/*callbac�ṹ�嶨�壬�����������ͺ���*/
    int fd;
    callback_func_t func;
} callbacks[CALLBACK_FUNCS];

static int nr_callbacks = 0;

int attach_callback_func(int fd, callback_func_t func)/*����callback����Ԫ���еĺ�����������*/
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
    /*װ���ں�ģ��*/
}

void remove_modules(void)
{
	/*ɾ��ĳ��ģ��*/
}

void host_init(char *ifname)
{
    /*��ʼ��ĳ���˿�*/
}

/* This signal handler ensures clean exits */
void signal_handler(int type)
{
	/*�źŴ�����*/
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
    /*��ʼ�����ֱ����ṹ��*/

    /* Parse command line: */
    while (1) {                                                                         /*���������������ִ����*/
	/*�������ѭ��������������в��������ݲ�ͬ��������ͬ����*/
    }
    /* Check that we are running as root */
    if (geteuid() != 0) {/*�ǲ���rootȨ��*/
	fprintf(stderr, "must be root\n");
	exit(1);
    }

    /* Detach from terminal */
    if (daemonize) {/*�ǲ��Ǳ�׼�������*/
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

    /* Initialize data structures and services... *//*���ֳ�ʼ��*/
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

    while (1) {                          /*�������ѭ��������main�����ĺ���*/
	memcpy((char *) &rfds, (char *) &readers, sizeof(rfds));

	timeout = timer_age_queue();
	
	timeout_spec.tv_sec = timeout->tv_sec;
	timeout_spec.tv_nsec = timeout->tv_usec * 1000;

	if ((n = pselect(nfds, &rfds, NULL, NULL, &timeout_spec, &origmask)) < 0) {/*���pselect������ʹ�Ĺ����Ǽ���ļ������������Ƿ��пɶ�*/
	    if (errno != EINTR)
		alog(LOG_WARNING, errno, __FUNCTION__,
		     "Failed select (main loop)");
	    continue;
	}

	if (n > 0) {                                 /*����ɶ�����������Ӧcallback�����õĺ���������������������aodv_socket_init*/
	    for (i = 0; i < nr_callbacks; i++) {		/*llf_init();nl_init();���õ�attach_callback_func����ʾ*/
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
