#include <stdio.h> // 입출력
#include <stdlib.h> // 문자열 변환 , 난수 생성
#include <unistd.h> // 표준 심볼 상수 및 자료형 
#include <string.h> // 문자열 상수
#include <arpa/inet.h> 
#include <sys/socket.h> // 소켓 연결
#include <pthread.h> // 쓰레드 사용

#define BUF_SIZE 100
#define NAME_SIZE 20

void * send_msg(void *arg); // 메시지 보냄
void * recv_msg(void * arg); // 메시지 받음 
void error_handling(char * msg); // 에러 처리

char name[NAME_SIZE]="[DEFAULT]"; // 이름 선언 및 초기화
char msg[BUF_SIZE]; 

int main(int argc, char *argv[])
{ 
	int sock; // 소켓 선언 

	struct sockaddr_in serv_addr; // 소켓 주소 구조체 선언 

	pthread_t snd_thread, rcv_thread; // 보내고, 받는 쓰레드 선언 

	void * thread_return; 
	if(argc!=4) { // 실행파일 경로 / port 번호 / name 입력받아야 함 
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	}

	sprintf(name,"[%s]",argv[3]); // 이름 입력 받음 

	/* 서버 소켓 (리스닝 소켓) 생성 */
	sock=socket(PF_INET, SOCK_STREAM, 0); 

	/* 주소 정보 초기화 */
	memset(&serv_addr, 0, sizeof(serv_addr)); 
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));

	/* 서버 주소 정보를 기반으로 연결 요청 , 실패 시 -1 반환 */
	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1) 
							// sock 은 클라이언트 소켓의 파일 디스크립터 
							// &serv_addr 은 연결 요청할 서버의 주소 정보를 담은 변수의 주소 값 
							// sizeof(serv_addr) 은 serv_addr에 전달된 주소의 변수 크기를 바이트 단위로 전달
		
		/* connect() 에러 생겼을 때 */
		error_handling("connect() error");

	/* 쓰레드 생성과 실행 흐름의 구성 */
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);  
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock); 
	
	/* 실행 시간 예측하지 않고 해결 , 쓰레드 종료까지 대기 */
	pthread_join(snd_thread, &thread_return); 
					   // snd_thread 은 인자로 들어오는 ID의 쓰레드가 종료할 때까지 실행 지연 시킴 
					   // &thread_return 은 쓰레드 종료 시 반환하는 값에 접근할 수 있는 포인터 

	pthread_join(rcv_thread, &thread_return); 

	close(sock); // 클라이언트 소켓 연결 종료

	return 0;
}

void * send_msg(void * arg)
{
	int sock=*((int*)arg); // 클라이언트의 파일 디스크립터
	char name_msg[NAME_SIZE+BUF_SIZE];
	while(1)
	{
		fgets(msg, BUF_SIZE, stdin); // 문자열 입력 
		if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n")) // ‘q’ 또는 ‘Q’ 입력 시 종료
		{	
			close(sock); // 클라이언트 소켓 연결 종료 후

			exit(0); // 프로그램 종료
		}
		sprintf(name_msg,"%s %s",name,msg); // 클라이언트 이름과 메시지 합침

		write(sock, name_msg, strlen(name_msg)); // 널문자 제외하고 서버로 문자열 송신 
	}
	return NULL;
}

void * recv_msg(void * arg)
{
	int sock=*((int*)arg); // 클라이언트의 파일 디스크립터

	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	while(1)
	{	
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1); // 데이터 수신
			// sock 은  데이터 수신 대상을 나타내는 파일 디스크립터 전달
			// name_msg 는 수신 데이터가 저장된 버퍼의 주소값 전달
			// NAME_SIZE+BUF_SIZE-1 은 수신 데이터의 바이트 수 전달 

		if(str_len==-1) // read() 실패 시 -1 반환 
			return (void*)-1;
		name_msg[str_len]=0;
		fputs(name_msg, stdout); // 문자열 출력
	}
	return NULL;
}

void error_handling(char * msg)
{
	fputs(msg, stderr); // 에러 메시지(문자열) 출력 
	fputc('\n', stderr); // 문자 출력
	exit(1);
}


