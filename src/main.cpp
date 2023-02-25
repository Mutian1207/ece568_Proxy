#include"proxy.hpp"
#include<iostream>
#include<cstdlib>
#include<cstdio>
#include<signal.h>

static void workAsDaemon(){
    pid_t pid;
    
    //step1 fork a child process
    pid = fork();
    //exit parent process
    if(pid<0){
        exit(EXIT_FAILURE);
    }else if(pid>0){
        exit(EXIT_SUCCESS);
    }

    //step2 detach from the terminal
    //dessociate from controlling tty
    if(setsid()<0) exit(EXIT_FAILURE);

    //step3 point stdin/stdout/stderr at /dev/null
    if( freopen("/dev/null","w",stderr)==NULL || 
        freopen("/dev/null","w",stdin)==NULL || 
        freopen("/dev/null","w",stdout)==NULL){
        exit(EXIT_FAILURE);
    }
    //step4 change directory
    chdir("/");

    //step5 set umask to 0
    umask(0);

    //step6 fork again to make the process not a session leader
    pid = fork();
     if(pid<0){
        exit(EXIT_FAILURE);
    }else if(pid>0){
        exit(EXIT_SUCCESS);
    }
    //step7 close signals
    //SIG_IGN -> ignore
    struct sigaction action = {SIG_IGN};
    //ignore SIGPIPE
    sigaction(SIGPIPE, &action, NULL);

    //save this process id to /var/run/mydaemon.pid
    //....

}
int main(){
    //become a daemon
    workAsDaemon();
    
    Proxy proxy("12345","100");
    proxy.start_proxy();

    exit(EXIT_SUCCESS);
}