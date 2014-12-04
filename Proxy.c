#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<ctype.h>

#define IE_REQUEST_LEN 2048
#define MAX_CHARBUFF 20480
#define BACKLOG 10

int sock_client_found, sock_client_SR, sock_server_SR;     /* listen on sock_client_found, send and receive on sock_client_SR */
struct sockaddr_in proxy_addr;
struct sockaddr_in clnt_addr;
struct sockaddr_in server_addr;
struct in_addr **addr_test;
pid_t pid;
int sin_size;
int k=1;
char recvbuffer[MAX_CHARBUFF];
char sendbuffer[MAX_CHARBUFF];
char checkbuffer[MAX_CHARBUFF];
char *targetip;
void clientRecvTest();
void ProxySentTest();
int HostProcess(char * SourceBuf,int DataLen);
int TCPsend(int s,const char *buf,int len,int flags);
int TCPrecv(int s,char *buf,int len,int flags);
void blacklist();
int n=1;
int tcpsendnum=0;
  

int main()
{
           
        int k=0;
        char *check="identification=";
        char *check1="password=";
        char *buffer,*buffer1;
        char *ID,*password;
        
        fputs("Proxy start\n",stdout);
	if((sock_client_found = socket(AF_INET, SOCK_STREAM, 0))==-1)
		{
		    perror("Socket: ");
		    exit(1);
	  }
        memset(&proxy_addr,0,sizeof(proxy_addr));
	proxy_addr.sin_family = AF_INET;
	proxy_addr.sin_addr.s_addr = INADDR_ANY;
        proxy_addr.sin_port = htons(8080);
	if(bind(sock_client_found,(struct sockaddr*)&proxy_addr,sizeof(struct sockaddr)) == -1)
		{
            perror("Bind: ");
            exit(1);
		}
        if(listen(sock_client_found,BACKLOG)== -1)
        {
            perror("Listen: ");
            exit(1);
        }

         

    while(1)
        {
            setsockopt(sock_client_found,SOL_SOCKET,SO_REUSEADDR,&k,sizeof(int));
            sin_size = sizeof(struct sockaddr_in);
            
            if ((sock_client_SR = accept(sock_client_found, (struct sockaddr *)&clnt_addr,&sin_size)) == -1)
                {
                    perror("Accept: ");
                    continue;
                }
            printf("Got connection from %s\n", inet_ntoa(clnt_addr.sin_addr));

            if(k==0)
            {
             memset(checkbuffer,0,MAX_CHARBUFF);
             char reply[]="<html><head><title>check the user ID and password</title><meta http-equiv=Content-Type content=\"text/html;charset=gb2312\"></head><body><form id= \"loginForm\"name=\"login\" action=\"INDOOR_ANY\"method=\"post\"><p><label>User ID:<input type=\"text\"name=\"identification\"/></label></p><p><label>password:<input type=\"password\"name=\"password\"/></label></p><input name= \"login\" type=\"submit\" value=\"login\"class=\"submit\"></form></body></html>";
             tcpsendnum = TCPsend(sock_client_SR,reply,strlen(reply),0);
             int Checkrecv;
             Checkrecv= recv(sock_client_SR,checkbuffer,MAX_CHARBUFF,0);
               if(Checkrecv>0)
               {
                printf("\n\n\nReceive buffer after transmitting the certification page: \n%s %d\n\n",checkbuffer,strlen(checkbuffer));
                //perror("recv");
               }
      
             buffer=(char*)malloc(sizeof(char)*(strlen(checkbuffer)+1));
             strcpy(buffer,checkbuffer);

             buffer1=(char*)malloc(sizeof(char)*(strlen(checkbuffer)+1));
             strcpy(buffer1,checkbuffer);
             
             ID=strtok(strstr(buffer,check),"&");
             password=strtok(strstr(buffer1,check1),"&");

             int isid = 1, ispassword= 1;


             if(ID!= NULL)
             {
              isid = strcmp("identification=test",ID);
             }
             else
             {
	      printf("\nID:\t%s\n",ID); 
             }


             if(password!= NULL)
             {
              ispassword = strcmp("password=test",password); 
             }
             else
             {
	      printf("\npassword:\t%s\n",password); 
             }             


             if(isid!=0 || ispassword !=0)
              {
               printf("wrong ID or password");
              }
             else
              {
               printf("Congratulations!! Enjoy the proxy");
               char* tempsend = "Congratulations!! Enjoy the proxy";
               tcpsendnum = TCPsend(sock_client_SR,tempsend, strlen(tempsend),0);
               k=k+1; 
              }   
             close(sock_client_SR); 
             }


           pid = fork();
            if (pid == 0)
               {
                   clientRecvTest();
                   ProxySentTest();
                   fputs("Child connection closed",stdout);
               }
            else if (pid <0)
                {
                    printf("Fork error!\n");
                    exit(-1);

                }
           else if (pid >0)
               {
                    close(sock_client_SR);
                    printf("Connection closed.\n");
               }
      }
   return 0;

}



void clientRecvTest()
{
  char clntName[INET_ADDRSTRLEN];
  inet_ntop(AF_INET,&clnt_addr.sin_addr.s_addr,clntName,sizeof(clntName));
  int numBytesRcvd;
  int numBytesSent;
  numBytesRcvd = recv(sock_client_SR,recvbuffer,MAX_CHARBUFF,0);

  shutdown(sock_client_SR,SHUT_RD);
 
  // fputs(recvbuffer,stdout);
}

void ProxySentTest()
{
   int flag=0;
   int numBytesRecvS;
   server_addr.sin_family = AF_INET;
   if((sock_server_SR = socket(AF_INET, SOCK_STREAM, 0))==-1)
       {
           perror("Socket: ");
           exit(1);
       }

     if(HostProcess(recvbuffer, strlen(recvbuffer))== 0);
    {
         printf("HostProcess is wrong!\n");

    }
    
    blacklist();

   if (connect(sock_server_SR, (struct sockaddr *)&server_addr, sizeof(server_addr))== -1)
      {
          perror("connect ");
          exit(1);
      }



    //  向webserver转发IE请求
   numBytesRecvS = TCPsend(sock_server_SR,recvbuffer, strlen(recvbuffer),0);
  
   char   dest[1000];  
   numBytesRecvS=1;
   int senlen=0;
   memset(dest,0,1000);
   numBytesRecvS=recv(sock_server_SR,dest,sizeof(dest)-1,0);  // 从webserver获取响应行
   if(numBytesRecvS==0)  
   {
      printf("与webserver(%s)的连接关闭\n",inet_ntoa(clnt_addr.sin_addr));
      shutdown(sock_server_SR,SHUT_RD); //关闭接收操作
      return;
    }
    else if(numBytesRecvS==-1)
   {
    shutdown(sock_server_SR,SHUT_RD); //关闭接收操作
    return;
   }

    // 取出响应行,处理之... 
   char szRespondLine[1000]={0};
   int count = 0;

   char *pch;
   char *pdest;
   pdest = strchr(dest,'\n');  //'\n'第一 次出现的位置
   count = pdest - dest+1;
   pdest++; //越过'\n'
   pch = strchr(pdest,'\n');  // '\n'第二次出现的位置
   count +=(pch -pdest+1);
   strncpy(szRespondLine,dest,count);
   szRespondLine[count]='\0';


   senlen = TCPsend(sock_client_SR,dest,numBytesRecvS,0);  //将数据转发给IE浏览器
   if(senlen==0)
   {
     printf("与浏览器(%s)的一个连接关闭...\n",inet_ntoa(clnt_addr.sin_addr));
     return;
   }
    else if(senlen==-1)
   {
     return;
   }
   

    // 循环接收webserver发来的数据,直至该连接关闭...
   while(numBytesRecvS>0)  
   {  
      memset(dest,0,1000);
      numBytesRecvS=recv(sock_server_SR,dest,sizeof(dest),0);  // 从webserver获取数据
      if(numBytesRecvS==0)  
  {
   printf("与webserver(%s)的连接关闭\n",inet_ntoa(server_addr.sin_addr));
   shutdown(sock_server_SR,SHUT_RD); //关闭接收操作
   break;
  }
      else if(numBytesRecvS==-1)
  {
     break;
  }

  senlen = TCPsend(sock_client_SR,dest,numBytesRecvS,0);  //将数据转发给IE浏览器
  if(senlen==0)
  {
      printf("与浏览器(%s)的一个连接关闭...\n",inet_ntoa(clnt_addr.sin_addr));
      break;
  }
  else if(senlen==-1)
  {    
     break;
  }
 }
 tcpsendnum += senlen;

    close(sock_client_SR);
    close(sock_server_SR);
    printf("***************Total Send: %d bytes****************\n", tcpsendnum);
    fputs("Child inside connection closed\n",stdout);
}


int HostProcess(char * SourceBuf,int DataLen)
{


  char *search="Host";
  char *search1="User-Agent";

  char *targetln,*targetla,*targetan,*targetport,*targeturl;
  char *buffer,*buffer1;
  struct hostent *targetIpByDN;
  char * SourceBufe;

  SourceBufe = SourceBuf;

  server_addr.sin_family = AF_INET;

  buffer=(char*)malloc(sizeof(char)*(strlen(SourceBufe)+1));
  strcpy(buffer,SourceBufe);

  buffer1=(char*)malloc(sizeof(char)*(strlen(SourceBufe)+1));
  strcpy(buffer1,SourceBufe);

  targetln=strtok(strstr(buffer,search),"\r\n");
  printf("\ntargetln:\t%s\n",targetln);

  targetan=strtok(strstr(buffer1,search1),"\r\n");
  printf("User-information:\t%s\n",targetan);

  targeturl=strtok((char*)strcasestr(targetln,":")+2,"\r\n");
  targetip=(char*)malloc(sizeof(char)*(strlen(targeturl)+1));


  printf("targeturl:\t%s\n",targeturl);

  strcpy(targetip,targeturl);

  targetport=strstr(targeturl,":");

  if(targetport!= NULL)
  {
      targetport+=1;
      printf("targetport:\t%s\n",targetport);
      server_addr.sin_port = htons((u_short)atoi(targetport));
  }
  else
  {
      server_addr.sin_port = htons(80);
      printf("no port address\n");
  }

  targetip=strtok(targetip,":");
  if(*targetip >= '0' && *targetip <= '9')
  {
      printf("targetip:\t%s\n",targetip);
      server_addr.sin_addr.s_addr = inet_addr(targetip);
  }
  else
  {
        targetIpByDN =(struct hostent *)gethostbyname(targetip);
        if(!targetIpByDN)
        {
            printf("get sever formation is wrong\n");
            printf("%s\n",targetip);
            return -1;
        }
      //memcpy(&(ServerAddr.sin_addr),pHost->h_addr,pHost->h_length);
      addr_test = ((struct in_addr **)targetIpByDN->h_addr_list);
      printf("Targetip by DNS: %s \n", inet_ntoa(*addr_test[0]));
      server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*addr_test[0]));
  }

  return 1;
}



int TCPrecv(int s,char *buf,int len,int flags)
{
 
 int nRev=0,recvCount=0;
 int length =len;

 if(buf==NULL)
  return 0;

  printf("\nbull error3\n");

 // 循环接收数据
 while(length>0)
 {
  nRev =recv(s,buf+recvCount,length,flags);

  printf("receive number: %d", nRev);

  if(nRev==0)
  {
   printf("\nerror for nRev = 0\n");
   break;
  }
  if(nRev==-1)//网络出现异常
  {
   printf("\nNW error\n");
   break;
  }
  printf("\nNo error\n");
  length-=nRev;
  recvCount+=nRev;
  printf("\n %d %d\n",length,recvCount);
 }
 printf("\nNo error2\n");
 return recvCount; //返回接收到的字节数
}


int TCPsend(int s,const char *buf,int len,int flags)
{
 int n=0,sendCount=0;
 int length =len;
 if(buf==NULL)
  return 0;
 while(length>0)
 {
  n=send(s,buf+sendCount,length,flags); //发送数据，
  if(n==0)
  {
   break;
  }
  if(n==-1)//网络出现异常
  {
   break;
   
  }
  length-=n;
  sendCount+=n;
 }

 return sendCount; // 返回已发送的字节数
}

//黑名单
void blacklist()
{
  char blip[20];
  FILE *inFile;

  inFile = fopen("blacklist.txt","r");

  if(inFile == NULL)
  {
    printf("\n Failed to open blacklist.\n");
    exit(1);
  }
  while (fgets(blip,20,inFile)!=NULL)
  {
    strtok(blip,"\n");

    int isblack;

    if(*targetip >= '0' && *targetip <= '9')
    {
     isblack = strcmp(targetip,blip);
    }
    else
    {
     isblack = strcmp(inet_ntoa(*addr_test[0]),blip);//targetIp解析来的Ip，全局变量
    }

    if(isblack == 0)
    {
        char send[250];
        FILE *caution;
        caution = fopen("blacklisttest.HTML","r");
        char caubuff[20480];
        char picbuff[20480];

        if(caution == NULL)
        {
          printf("\n Failed to open blacklist.\n");
          exit(1);
        }

        while(fgets(send,250,caution)!=NULL)
        {
         strcat(caubuff,send);
        }
        fclose(caution);
        
                

        //char reply[]="<html>You are logining a wesite on the blacklist</html>";
        int numBytesSent = TCPsend(sock_client_SR,caubuff,strlen(caubuff),0);
        int numBytesRecv=  recv(sock_client_SR,picbuff,MAX_CHARBUFF,0);
        printf("Website in BLACK LIST!\n");
        //printf("\n\n\nReceive buffer after black list: \n%s %d\n\n",picbuff,strlen(picbuff));

         
        close(sock_server_SR);
    }
    else
    {
        printf("Website safe!\n");//connection,resv,send
    }
   }
    fclose(inFile);
}
  

