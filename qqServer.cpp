#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include<time.h>
#include<iostream>
#include<map>

using namespace std;

struct user_psd
{
	char name[50];
	char password[50];
};


class qqServer {
public:
	int m_listenfd;
	int m_clientfd[10];
	FILE *userInfo;
	map<string,int> onlineUser;
	
	qqServer();

	void start();

	void login(int fd);
	
	bool check(string name);

	bool check(string name,string psd);

	bool InitServer(int port);

	bool Accept();

	//void *pth_main(void *arg);

	int Send(const void *buf,const int buflen);

	int Recv(void *buf,const int buflen); 

	~qqServer();
};

struct param{
	int fd;
	int id;
	qqServer *serv;
};

void *pth_main(void *arg);
int main(int argc,char *argv[])
{
	qqServer server;
	if(server.InitServer(atoi(argv[1]))==false){
		printf("server initiate fail.\n"); return -1;
	}
	server.Accept();
}

qqServer::qqServer(){
	m_listenfd=0;
	//m_clientfd=0;
	userInfo = fopen("userInfo.txt","a+");
}

qqServer::~qqServer(){
	if(m_listenfd!=0) close(m_listenfd);
	//if(m_clientfd!=0) close(m_clientfd);
}

void qqServer::start(){
	
}

void qqServer::login(int fd){
	char buf[100];
	int cltfd=fd;
	//skip while when login success
	string username,password;
	while(1){
		//recv username and password
		memset(buf,0,sizeof(buf));
		sprintf(buf,"username:");
		send(cltfd,buf,strlen(buf),0);
		memset(buf,0,sizeof(buf));
		recv(cltfd,buf,sizeof(buf),0);
		username=buf;

		memset(buf,0,sizeof(buf));
		sprintf(buf,"password:");
		send(cltfd,buf,strlen(buf),0);
		memset(buf,0,sizeof(buf));
		recv(cltfd,buf,sizeof(buf),0);
		password=buf;

		//create a new user and save it
		if(!check(username)){
			memset(buf,0,sizeof(buf));
			sprintf(buf,"This username not exits!Do you want to build one?[y/n]");
			send(cltfd,buf,strlen(buf),0);

			string choose;
			memset(buf,0,sizeof(buf));
			recv(cltfd,buf,sizeof(buf),0);
			//strcpy(choose.c_str(),buf);
			choose=buf;
			if(choose=="y"||choose=="Y"){
				user_psd newInfo;
				string newUsername,newPassword;

				memset(buf,0,sizeof(buf));
				sprintf(buf,"new username:");
				send(cltfd,buf,strlen(buf),0);

				memset(buf,0,sizeof(buf));
				recv(cltfd,buf,sizeof(buf),0);
				//strcpy(newUsername.c_str(),buf);
				newUsername=buf;

				while(check(newUsername)){
					memset(buf,0,sizeof(buf));
					sprintf(buf,"This name already exits.\nChoose another one:");
					send(cltfd,buf,strlen(buf),0);
					memset(buf,0,sizeof(buf));
					recv(cltfd,buf,sizeof(buf),0);
					//strcpy(newUsername.c_str(),buf);
					newUsername=buf;
				}

				memset(buf,0,sizeof(buf));
				sprintf(buf,"password:");
				send(cltfd,buf,strlen(buf),0);

				memset(buf,0,sizeof(buf));
				recv(cltfd,buf,sizeof(buf),0);
				//strcpy(newPassword.c_str(),buf);
				newPassword=buf;

				//save
				strcpy(newInfo.name,newUsername.c_str());
				strcpy(newInfo.password,newPassword.c_str());
				fseek(userInfo,0,SEEK_END);
				fwrite(&newInfo,1,sizeof(newInfo),userInfo);
				//send create success
				memset(buf,0,sizeof(buf));
				sprintf(buf,"Create success!");
				send(cltfd,buf,strlen(buf),0);
			}else{
				continue;
			}
		}else if(!check(username,password)){
			//start();
			while(!check(username,password)){
				memset(buf,0,sizeof(buf));
				sprintf(buf,"password is wrong\nplease try again:");
				send(cltfd,buf,strlen(buf),0);
				memset(buf,0,sizeof(buf));
				recv(cltfd,buf,sizeof(buf),0);
				//strcpy(password.c_str(),buf);
				password=buf;
			}
			memset(buf,0,sizeof(buf));
			sprintf(buf,"login success");
			send(cltfd,buf,strlen(buf),0);
			//save user to online list
			onlineUser[username]=cltfd;
			cout<<username<<" login"<<endl;
			break;
		}else{
			memset(buf,0,sizeof(buf));
			sprintf(buf,"login success");
			send(cltfd,buf,strlen(buf),0);
			onlineUser[username]=cltfd;
			cout<<username<<" login"<<endl;
			break;
		}
	}
	//after login success
	while(1){
		string cmd;
		memset(buf,0,sizeof(buf));
		recv(cltfd,buf,sizeof(buf),0);
		cmd=buf;
		if(cmd=="list"){
			auto it=onlineUser.begin();
			while(it!=onlineUser.end()){
				memset(buf,0,sizeof(buf));
				if(it->first==username){
					string temp=username+"(you)";
					strcpy(buf,temp.c_str());
				}else{
					strcpy(buf,it->first.c_str());
				}
				send(cltfd,buf,strlen(buf),0);
				it++;
			}
			sleep(1);
			memset(buf,0,sizeof(buf));
			strcpy(buf,"finish");
			send(cltfd,buf,strlen(buf),0);
			continue;
		}
		if(cmd=="logoff"){
			onlineUser.erase(username);
			cout<<username<<"log off"<<endl;
			return;
		}
		if(cmd=="connect"){

		}
		if(cmd=="send"){

		}
	}

}

bool qqServer::check(string name){
	rewind(userInfo);
	user_psd info;
	while(1){
		if(fread(&info,1,sizeof(info),userInfo)==0) break;
		if(strcmp(info.name,name.c_str())==0) return true;
	}
	return false;
}

bool qqServer::check(string name,string psd){
	rewind(userInfo);
	user_psd info;
	while(1){
		if(fread(&info,1,sizeof(info),userInfo)==0) break;
		if(strcmp(info.name,name.c_str())==0&&strcmp(info.password,psd.c_str())==0) return true;
	}
	return false;
}

bool qqServer::InitServer(int port){
	m_listenfd = socket(AF_INET,SOCK_STREAM,0);
	if(m_listenfd==-1){
		m_listenfd=0; return false;
	}
	sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port=htons(port);
	if(bind(m_listenfd,(sockaddr *)&servaddr,sizeof(servaddr))!=0){
		close(m_listenfd); m_listenfd=0; return false;
	}
	if(listen(m_listenfd,5) != 0){
		close(m_listenfd); m_listenfd=0; return false;
	}
	return true;
}

bool qqServer::Accept(){
	int i=0;
	while(1){
		if( (m_clientfd[i]=accept(m_listenfd,0,0))<=0 ) return false;
		pthread_t pthid;
		param  param1;
		param1.fd=m_clientfd[i];
		param1.id=i;
		param1.serv=this;
		if(pthread_create(&pthid,NULL,pth_main,&param1)!=0){
			printf("pthread_crate failed\n"); return false;
		}
		
		
	}
}

void *pth_main(void *arg){
	param tmp=*(param*)arg;
	int clientfd=tmp.fd;
	int client_id=tmp.id;
	qqServer *serv=tmp.serv;
	char cmd_buf[100];
	string cmd;
	while(1){
		memset(cmd_buf,0,sizeof(cmd_buf));
		recv(clientfd,cmd_buf,sizeof(cmd_buf),0);
		//strcpy(cmd,cmd_buf);
		cmd=cmd_buf;
		if(cmd=="login"){
			serv->login(clientfd);
			continue;
		}
	}
}

int qqServer::Send(const void *buf,const int buflen){
	//return send(m_clientfd,buf,buflen,0);
}

int qqServer::Recv(void *buf,const int buflen){
	//return recv(m_clientfd,buf,buflen,0);
}
