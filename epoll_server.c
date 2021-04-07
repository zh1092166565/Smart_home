/*
	端口号
	IP地址
	几路客户端  （一般为5）
1，创建套接字--同时指定协议和类型
		socket()
2，绑定 --将保存在相应地址结构中的地址信息，与套接字进行绑定
		2.1 填充struct sockaddr_in结构体变量
			（1）struct sockaddr_in ,
			（2）struct in_addr
			（3）htons()
			（4）inet_pton()
		2.2 绑定
			 bind() 
3，将主动套接字转换为被动套接字
		listen()
4，阻塞的等待客户端连接
		accept()
		获取刚建立客户端的ip地址和端口号
			inet_ntop()
			ntohs()
		
5，读写操作
*/
#include"net.h"

#define epoll_num 20
#define BUFLEN 128

//多线程
pthread_t led_on;
pthread_t led_off;
pthread_t buzer_on;
pthread_t buzer_off;
pthread_t gyr_on;
pthread_t gyr_off;
pthread_t camera;
 

//线程锁
/*
pthread_mutex_t led_on_lock;
pthread_mutex_t led_off_lock;
pthread_mutex_t buzer_on_lock;
pthread_mutex_t buzer_off_lock;
pthread_mutex_t gyr_on_lock;
pthread_mutex_t gyr_off_lock;
*/
//LED
int led_fd;

//陀螺仪
int gyr_fd;
unsigned char gry_buf[6] = {0};
int x,y,z;


//多线程相关函数声明
void *led_on_fun(void *arg);
void *led_off_fun(void *arg);
void *buzer_on_fun(void *arg);
void *buzer_off_fun(void *arg);
void *gyr_on_fun(void *arg);
void *gyr_off_fun(void *arg);
void *camera_fun(void *arg);



int socket_server(char *addr,char *port)
{
	int sockfd;
	struct sockaddr_in sin;
	//创建套接字--同时指定协议和类型
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
	{
		perror("socket\n");
		return -1;
	}

	int b = -1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &b,sizeof(int));

	//绑定 --将保存在相应地址结构中的地址信息，与套接字进行绑定
	memset(&sin, 0 ,sizeof(sin));

	sin.sin_family = AF_INET;    //ipv4协议
	sin.sin_addr.s_addr = inet_addr(addr);   //IP地址
	sin.sin_port  = htons(atoi(port));       //端口号

	if(bind(sockfd, (struct sockaddr *)&sin,sizeof(sin)) < 0)
	{
		perror("bind\n");
		return -1;
	}

	//将主动套接字转换为被动套接字
	if(listen(sockfd, 5) < 0)
	{
		perror("listen\n");
		return -1;
	}
	return sockfd;
}


int main(int argc, char * argv [])
{
	char *addr , *port;
	int epoll_sockfd;
	int client_fd;
	int epoll_fd;
	int ret = 0;
	int i = 0; 
	int rdlen = 0;
	char buf[BUFLEN]= {0};

	struct sockaddr_in client_sin;
	struct epoll_event event ;
	struct epoll_event events[epoll_num];


	//初始化线程锁
	/*
	pthread_mutex_init(&led_on_lock,NULL);
	pthread_mutex_init(&led_off_lock,NULL);
	pthread_mutex_init(&buzer_on_lock,NULL);
	pthread_mutex_init(&buzer_off_lock,NULL);
	pthread_mutex_init(&gyr_on_lock,NULL);
	pthread_mutex_init(&gyr_off_lock,NULL);
	*/
	if(argc !=3)
	{
		printf("Usage:%s serverip port.\n",argv[0]);
		return -1;
	}

	addr = argv[1];
	port = argv[2];

	//获取连接的文件描述符
	epoll_sockfd = socket_server(addr, port);
	if(epoll_sockfd < 0)
	{
		printf("socket_server error\n");
		return -1;
	}

	
	//epoll连接
	memset(events, 0, epoll_num*sizeof(struct epoll_event)); //清空结构体
	event.events = EPOLLIN;
	event.data.fd = epoll_sockfd;
	
	epoll_fd = epoll_create(1);

	epoll_ctl(epoll_fd,EPOLL_CTL_ADD,epoll_sockfd,&event);

	while(1)
	{
		ret = epoll_wait(epoll_fd,events,20,-1);  //阻塞等待时间的到来      ，返回值为响应的fd的个数
		//printf("epoll_wait return = %d\n",ret);

		for(i=0;i < ret ;i++)
		{
			printf("epoll_wait return = %d\n",events[i].data.fd);
			if(events[i].data.fd == epoll_sockfd)
			{
				client_fd = accept(events[i].data.fd,NULL,NULL);
				if(client_fd < 0)
				{
					printf("accept error\n");
					exit(1);
				}

				struct epoll_event event;
				event.events = EPOLLIN;
				event.data.fd = client_fd;
				epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_fd,&event);
			}
			else 
			{	
				int  choose = 0;
				memset(buf, 0 ,BUFLEN);
				rdlen = read(events[i].data.fd, buf,BUFLEN);
				if(rdlen > 0)
				{
					printf("read buf=%s\n",buf);
					choose = buf[0]-48;
					switch (choose)
					{
						case LED_ON:
							if(pthread_create(&led_on,NULL,led_on_fun,NULL) != 0)
							{
								printf("pthread_create led on error\n");
								return -1;
							}
							break;
						case LED_OFF:
							if(pthread_create(&led_off,NULL,led_off_fun,NULL) != 0)
							{
								printf("pthread_create led off error\n");
								return -1;
							}
							break;
						case BUZER_ON:
							if(pthread_create(&buzer_on,NULL,buzer_on_fun,NULL) != 0)
							{
								printf("pthread_create buzer on error\n");
								return -1;
							}
							break;
						case BUZER_OFF:
							if(pthread_create(&buzer_off,NULL,buzer_off_fun,NULL) != 0)
							{
								printf("pthread_create buzer off error\n");
								return -1;
							}
							break;
						case GYR_ON:
							if(pthread_create(&gyr_on,NULL,gyr_on_fun,NULL) != 0)
							{
								printf("pthread_create gyr on error\n");
								return -1;
							}
							break;
						case GYR_OFF:
							if(pthread_create(&gyr_off,NULL,gyr_off_fun,NULL) != 0)
							{
								printf("pthread_create gyr off error\n");
								return -1;
							}
							break;
						case CAMERA:
							if(pthread_create(&camera,NULL,camera_fun,NULL) != 0)
							{
								printf("pthread_create camera error\n");
								return -1;
							}
							break;
					
					 }
					

				}
				else if(rdlen == 0)
				{
					event.events = EPOLLIN;
					event.data.fd = events[i].data.fd;
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &event);
				}

			}
		}
	}	
	
	pthread_join(led_on, NULL);
	pthread_join(led_off, NULL);
	pthread_join(buzer_on, NULL);
	pthread_join(buzer_off, NULL);
	pthread_join(gyr_on, NULL);
	pthread_join(gyr_off, NULL);
	/*				
					//销毁锁
					pthread_mutex_destroy(&led_on_lock);
					pthread_mutex_destroy(&led_off_lock);
					pthread_mutex_destroy(&buzer_on_lock);
					pthread_mutex_destroy(&buzer_off_lock);
					pthread_mutex_destroy(&gyr_on_lock);
					pthread_mutex_destroy(&gyr_off_lock);
					*/
}

//多线程相关函数
void *led_on_fun(void *arg)
{
	//pthread_mutex_lock(&led_on_lock);
	int on = 0;
	led_fd = open("/dev/led0",O_RDWR);
	if(led_fd < 0)
	{
		perror("open");
		exit(1);
	}
	on = 1;
	write(led_fd,&on,4);
	printf("led is open \n");
	sleep(1);
	//pthread_mutex_unlock(&led_off_lock);
	return 0;
}
void *led_off_fun(void *arg)
{
	//pthread_mutex_lock(&led_off_lock);
	
	int on = 0;
	on = 2;
	write(led_fd,&on,4);
	printf("led is down \n");
	sleep(1);
	close(led_fd);
	//pthread_mutex_unlock(&led_on_lock);
	return 0;
}

void *buzer_on_fun(void *arg)
{
	//pthread_mutex_lock(&buzer_on_lock);
	int on = 0;
	led_fd = open("/dev/led0",O_RDWR);
	if(led_fd < 0)
	{
		perror("open");
		exit(1);
	}
	on = 3;
	write(led_fd,&on,4);
	printf("buzer is open \n");
	//pthread_mutex_unlock(&buzer_off_lock);
	return 0;
}

void *buzer_off_fun(void *arg)
{
	//pthread_mutex_lock(&buzer_off_lock);
	int on = 0;
	on = 4;
	write(led_fd,&on,4);
	printf("buzer is down \n");
	sleep(1);
	close(led_fd);
	//pthread_mutex_unlock(&buzer_on_lock);
	return 0;
}
void *gyr_on_fun(void *arg)
{
	//pthread_mutex_lock(&gyr_on_lock);
	if((gyr_fd = open("/dev/i2c_mma8451q",O_RDWR)) < 0)
	{
		perror("open error\n");
		return -1;
	}
	while(1)
	{
		read(gyr_fd,gry_buf,sizeof(gry_buf));
		x = gry_buf[0];
		x = x<<6;
		x |= gry_buf[1]>>2;
		
		y = gry_buf[2];
		y = y<<6;
		y |= gry_buf[3]>>2;
		
		z = gry_buf[4];
		z = z<<6;
		z |= gry_buf[5]>>2;
		
		printf("x = %d,y = %d,z = %d\n",x,y,z);
		sleep(1);
	}
	printf("gyr is open \n");
	//pthread_mutex_unlock(&gyr_off_lock);
	return 0;
}
void *gyr_off_fun(void *arg)
{
	//pthread_mutex_lock(&gyr_off_lock);
	printf("gyr is down \n");
	close(gyr_fd);
	//pthread_mutex_unlock(&gyr_on_lock);
	return 0;
}

void *camera_fun(void *arg)
{
	system("./test.sh");
	printf("camera is open \n");
	return 0;
}






