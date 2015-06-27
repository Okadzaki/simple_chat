#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <vector>
#include <arpa/inet.h>
#include <string.h>
#include <algorithm>
#include <unistd.h>
#include <pthread.h>

using namespace std;

static const int PORT= 20203;
static const string HOST = "127.0.0.1";
static const int MAX_CLIENTS = 10;


struct THREADINFO {
    pthread_t thread_ID;
    int sockfd;
    string nick;
};

struct PACKAGE{
    char nick[1024];
    char buff[1024];
};


int compare( THREADINFO *a,  THREADINFO *b) {
    return a->sockfd - b->sockfd;
}



int sockfd, newfd;
THREADINFO thread_info[10];
vector<THREADINFO> client_list;
pthread_mutex_t clientlist_mutex;
void *client_handler(void *fd);
int main(){
    int err_ret, sin_size;
    sockaddr_in serv_addr, client_addr;
    pthread_mutex_init(&clientlist_mutex, NULL);

    /* open a socket */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        err_ret = errno;
        cerr << "socket() failed..." <<endl;
        return err_ret;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(HOST.c_str());
    memset(&(serv_addr.sin_zero), 0, 8);

    if(bind(sockfd, (sockaddr *)&serv_addr, sizeof(sockaddr)) == -1) {
        err_ret = errno;
        cerr << "bind() failed..." <<endl;
        return err_ret;
    }

    if(listen(sockfd, MAX_CLIENTS) == -1) {
        err_ret = errno;
        cerr << "listen() failed...." <<endl;

        return err_ret;
    }

    cout << "Starting socket listener.." << endl;
    while(1) {
        sin_size = sizeof( sockaddr_in);
        if((newfd = accept(sockfd, ( sockaddr *)&client_addr, (socklen_t*)&sin_size)) == -1) {
            err_ret = errno;
            cerr << "accept() failed..." <<endl;
            return err_ret;
        }
        else {
            if(client_list.size() == MAX_CLIENTS) {
                cerr << "Connection full, request rejected..." <<endl;
                continue;
            }
            cout << "Connection requested received"<<endl;
            THREADINFO threadinfo;
            threadinfo.sockfd = newfd;
            threadinfo.nick = "Anonymous";
            pthread_mutex_lock(&clientlist_mutex);
            client_list.push_back(threadinfo);
            pthread_mutex_unlock(&clientlist_mutex);
            pthread_create(&threadinfo.thread_ID, NULL, client_handler, (void *)&threadinfo);
        }
    }
    return 0;
}

vector<THREADINFO>::iterator findThread(vector<THREADINFO>& vector1,THREADINFO& threadInfo){

    for (auto item = vector1.begin();item != vector1.end();++item){
        if (compare(&(*item),&threadInfo) == 0){
            return item;
        }
    }
}

void *client_handler(void *fd) {
    THREADINFO threadinfo = *(THREADINFO *)fd;
    PACKAGE package;
    int bytes, sent;
    while(1) {

        bytes = recv(threadinfo.sockfd, (void *)((char *)&package), 1, 0);
        if(!bytes) {
            cerr << "Connection lost from " << threadinfo.nick;
            pthread_mutex_lock(&clientlist_mutex);
            if (!client_list.empty()) {
                client_list.erase(findThread(client_list,threadinfo));
            }
            pthread_mutex_unlock(&clientlist_mutex);
            break;
        }
        for (int i=1;i<sizeof(PACKAGE);i++){
            bytes = recv(threadinfo.sockfd, (void *)((char *)&package+i), 1, 0);
        }
        if(strlen(package.nick) == 0 || strlen(package.buff) == 0)
            continue;

        if (!strcmp(package.buff,"exit")){
            cout << "User " << package.nick << " logout" << endl;
            pthread_mutex_lock(&clientlist_mutex);
            if (!client_list.empty())
                client_list.erase(findThread(client_list,threadinfo));
            pthread_mutex_unlock(&clientlist_mutex);
            break;
        }


        
        cout<< package.nick << " : " << package.buff << endl;
        pthread_mutex_lock(&clientlist_mutex);
        for (size_t i=0;i<client_list.size();i++){
            PACKAGE spacket;
            if(!compare(&client_list[i], &threadinfo)) continue;

            strcpy(spacket.nick,package.nick);
            strcpy(spacket.buff,package.buff);


            for (int j = 0; j< sizeof (PACKAGE);j++)
                sent = send(client_list[i].sockfd, (void *)((char*)&spacket+j), 1, 0);

        }
        pthread_mutex_unlock(&clientlist_mutex);

    }

    /* clean up */
    close(threadinfo.sockfd);

    return NULL;
}