//
// Created by okadzaki on 30.06.15.
//

#ifndef SERVER_THREADINFO_H
#define SERVER_THREADINFO_H

#include <pthread.h>
#include <string>

struct THREADINFO {
    pthread_t thread_ID;
    int sockfd;
    std::string nick;
};

#endif //SERVER_THREADINFO_H
