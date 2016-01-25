#include "interprocess.h"

interprocess::interprocess(std::wstring _filename): quit(false), msg_size(10) {
    filename = wstos(_filename);  
}


//better error checking ofc
void interprocess::send_msg(std::string msg){
    std::cout<<"sending msg: "<<msg<<std::endl;
    int error = mknod(filename.c_str(),S_IFIFO | 0666, 0);
    if(error == -1){
        if(errno != EEXIST){
            perror("mknod: ");
            exit(errno);
        }
    }

    int fd = open(filename.c_str(), O_WRONLY);
    if(fd == -1)
        exit(errno);
    msg = serialize(msg);
    std::cout<<msg<<std::endl;
    write(fd, msg.c_str(), msg_size);
    close(fd);
}

void interprocess::listen(){
    std::thread th(&interprocess::run, this);
    th.join();
    std::cout<<"end"<<std::endl;

}

void interprocess::run(){

    int error = mknod(filename.c_str(),S_IFIFO | 0666, 0);
    if(error == -1){
        if(errno != EEXIST){
            perror("mknod(): ");
            exit(errno);
        }
    }
    int fd = open(filename.c_str(),O_RDONLY);
    if(fd == -1){
        perror("open(): ");
        exit(errno);
    }
    //char* buff = new char[msg_size+1];
    char buff[100];
    while(!quit){
        if(read(fd,buff,msg_size) != 0){
            buff[msg_size] = '\0';
            std::cout<<buff<<std::endl;
        }else{
            fd = open(filename.c_str(), O_RDONLY);
        }
    }

    

}

std::string interprocess::wstos(std::wstring wstr){
    std::setlocale(LC_ALL, "");
    char* str = new char[wstr.size()];
    std::wcstombs(str,wstr.c_str(),wstr.size());
    std::string s(str);
    delete[] str;
    return s;
}

interprocess::~interprocess(){}
