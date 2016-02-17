#include "interprocess.h"
#include <sys/poll.h>
#include <cstring>

interprocess::interprocess(std::wstring _filename):filename(_filename.begin(), _filename.end()), quit(false) 
{
    //try to create the fifo
    int error = mknod(filename.c_str(),S_IFIFO | 0666, 0);
    if (error == -1) {
        if (errno != EEXIST) {
            perror("mknod(): "); 
            throw ipc::fifo_create_error();
        }
    }

}


void interprocess::send_msg(std::string msg)
{
    int fd = open(filename.c_str(), O_WRONLY);
    if (fd == -1) {
        close(fd);
        perror("open(): ");
        throw ipc::fifo_open_error();
    }
    msg = on_msg_send(msg);
    int size = msg.size();
    write(fd, (const char*)(&size),sizeof(size)); //send size 
    write(fd, msg.c_str(), size); //send msg
    close(fd);
}

void interprocess::listen()
{
    /*
    th = std::thread(&interprocess::run, this);
    std::cout<<"end"<<std::endl;
*/
    run();
}

void interprocess::run()
{

    int fd = open(filename.c_str(),O_RDWR);
    if (fd == -1) {
        throw ipc::fifo_open_error();
    }
    char* buff;
    int size;
    while(true){
        if (read(fd,(char*)(&size),sizeof(size)) == sizeof(size)) { //read size of msg 
            buff = new char[size+1];
            if (read(fd, buff, size) != size) {//read msg
                close(fd);
                delete[] buff;
                throw ipc::wrong_msg_size();
            }
            buff[size] = '\0';
            last_msg = std::string(buff);
            delete[] buff;

            th = std::thread(&interprocess::on_msg_receive, this, std::ref(last_msg));
            th.detach();

            if (last_msg == "quit") {
                break;
            }
        }
    }
    close(fd);

}


//broken as fuck
std::string interprocess::wstos(const std::wstring& wstr)
{
    std::string s(wstr.begin(), wstr.end());
    return s;
}

interprocess::~interprocess()
{
    if (th.joinable())
        th.join();
   // unlink(filename.c_str()); //maybe?
}
