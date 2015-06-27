#include <iostream>
#include <netdb.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

using namespace std;

static const int PORT = 20203;
void *receiver(void *param);

struct PackageInfo{
    char nick[1024];
    char buff[1024];
};

struct User{
    int sockfd;
    string nick;
};
struct ThreadInfo{
    pthread_t thread;
    int sockfd;

};


int isDisconnected,sockfd;


int connectWithServer(string& HOST){
    int newfd, err_ret;
     sockaddr_in serv_addr;
     hostent *to;

    if((to = gethostbyname(HOST.c_str()))==NULL) {
        err_ret = 0;
        cerr<<"HOST ERROR" << endl;
        return err_ret;
    }

    if((newfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        err_ret = 0;
        cerr<<"SOCKET ERROR" << endl;
        return err_ret;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr = *(( in_addr *)to->h_addr);
    memset(&(serv_addr.sin_zero), 0, 8);

    if(connect(newfd, ( sockaddr *)&serv_addr, sizeof( sockaddr)) == -1) {
        err_ret = 0;
        cerr << "CONNECT ERROR"<<endl;
        return err_ret;
    }
    else {
        cout << "CONNECTED!"<<endl;
        return newfd;
    }
}

void login(User* client, string& host){
    if(isDisconnected){
        cerr << "You'are disconnected";
        return;
    }
    sockfd = connectWithServer(host);
    if(sockfd >= 0) {
        isDisconnected = 1;
        client->sockfd = sockfd;
        cout << "Logged in as " << client->nick << endl;
        ThreadInfo threadinfo;
        pthread_create(&threadinfo.thread, NULL, receiver, (void *)&threadinfo);

    }
    else {
        cerr << "Connection rejected..." << endl;
    }


}

void *receiver(void *param){
    int recvd;
    PackageInfo package;

    cout << "Waiting server..." << endl;

    while (isDisconnected){
        recvd = recv(sockfd, (void *)((char *)&package), 1, 0);
        if (!recvd){
            cerr << "Connection lost";
            isDisconnected = 0;
            close(sockfd);
            break;
        }


        for (int i=1;i<sizeof(PackageInfo);i++){
            recvd = recv(sockfd, (void *)((char *)&package+i), 1, 0);
        }




        if(recvd > 0){
            cout << package.nick << " : " << package.buff<<endl;
        }
    }
    return NULL;

}

void sentToServer( User* client,string& message){
    int sent;
    PackageInfo package;

    if(!isDisconnected) {
        cerr << "\"You are not connected..." << endl;
        return;
    }

    strcpy(package.nick,client->nick.c_str());
    
    strcpy(package.buff,message.c_str());

    for (int i = 0; i< sizeof (PackageInfo);i++)
        sent = send(sockfd, (void *)((char*)&package+i), 1, 0);

}

bool validateIpAddress(const string &ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}


int main() {
    User user;
    while(true){
        cout << "Enter nickname: ";
        getline(cin,user.nick);
        if(user.nick.size() > 0){
            break;
        }else{
            cout << "Wrong nickname" << endl;
         }
    }

    string host = "";
    while(true){
        cout << "Server adress: ";
        getline(cin,host);
        if(validateIpAddress(host)){
            break;
        }else{
            cout << "Wrong adress" << endl;
        }
    }


    login(&user,host);
        string message;
    while (getline(cin,message)){
        sentToServer(&user,message);
        if (!message.compare("exit")){
            break;
        }
    }
    close(sockfd);
    return 0;
}