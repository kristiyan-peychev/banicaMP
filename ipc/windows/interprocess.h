#ifndef INTERPROCESS_H_61QS3BL0
#define INTERPROCESS_H_61QS3BL0

#include "../interprocess.h"

#include <Windows.h>
#include <thread>
#include <mutex>
#include <sstream>


#define PIPE_TIMEOUT 5000
#define BUFSIZE 4096

class interprocess: public interprocess_interface {
public:
    interprocess(std::wstring);
    virtual ~interprocess();
    
protected:
    virtual std::string on_msg_send(std::string& str)=0;
    virtual std::string on_msg_receive(std::string& str)=0;
public:
    void send_msg(std::string);
    void listen();

private:
    void server_create();
    void client_create();

    std::string filename;
    std::string last_msg;
    std::thread th;
    std::mutex read_mutex;
    HANDLE pipe_handle;

    void run();


};


#endif /* end of include guard: INTERPROCESS_H_61QS3BL0 */

