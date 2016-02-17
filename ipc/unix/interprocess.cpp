#include "interprocess.h"
#include <sys/poll.h>
#include <cstring>

interprocess::interprocess(std::wstring _filename):filename(_filename.begin(), _filename.end()), quit(false) {
    std::cout<<"filename: "<<filename<<std::endl;
    //try to create the fifo
    int error = mknod(filename.c_str(),S_IFIFO | 0666, 0);
    if(error == -1){
        if(errno != EEXIST){
            perror("mknod(): ");
            throw ipc::fifo_create_error();
        }
    }

}


void interprocess::send_msg(std::string msg){
    std::cout<<"sending msg: "<<msg<<std::endl;
    int fd = open(filename.c_str(), O_WRONLY);
    if(fd == -1){
        close(fd);
        throw ipc::fifo_open_error();
    }
    msg = on_msg_send(msg);
    int size = msg.size();
    write(fd, (const char*)(&size),sizeof(size)); //send size 
    write(fd, msg.c_str(), size); //send msg
    close(fd);
}

void interprocess::listen(){
    /*
    th = std::thread(&interprocess::run, this);
    std::cout<<"end"<<std::endl;
*/
    run();
}

void interprocess::run(){

    int fd = open(filename.c_str(),O_RDWR);
    if(fd == -1){
        throw ipc::fifo_open_error();
    }
    char* buff;
    int size;
    while(true){
        if(read(fd,(char*)(&size),sizeof(size)) == sizeof(size)){ //read size of msg 
            std::cout<<"Size: "<<size<<std::endl;
            buff = new char[size+1];
            if(read(fd, buff, size) != size) {//read msg
                close(fd);
                delete[] buff;
                throw ipc::wrong_msg_size();
            }
            buff[size] = '\0';
            last_msg = buff;
            delete[] buff;

            std::cout<<"Received: "<<last_msg<<std::endl;
            std::cout<<"listen thread: "<<std::this_thread::get_id()<<std::endl;

            th = std::thread(&interprocess::on_msg_receive, this, std::ref(last_msg));
            th.detach();

            if(last_msg == "quit"){
                break;
            }
        }
    }
    close(fd);

}


//broken as fuck
std::string interprocess::wstos(const std::wstring& wstr){
    std::string s(wstr.begin(), wstr.end());
    return s;
}

interprocess::~interprocess(){
    if(th.joinable())
        th.join();
   // unlink(filename.c_str()); //maybe?
}
