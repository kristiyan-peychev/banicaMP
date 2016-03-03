#ifndef INTERPROCESS_H_QFGT309Z
#define INTERPROCESS_H_QFGT309Z

#include <string>
#include <exception>
#include <stdexcept>

class interprocess_interface {

public:
    interprocess_interface(){};
    interprocess_interface(std::wstring);
    virtual ~interprocess_interface(){};
    
protected:
    virtual std::string on_msg_send(std::string&)=0;
    virtual std::string on_msg_receive(std::string&)=0;

public:
    void send_msg(std::string);
    void listen();

private:

    void run();
};

namespace ipc {
    class exception: public std::exception {};

    class fifo_open_error: public ipc::exception {
        public: 
        const char* what() noexcept {
            return "Error while opening fifo";
        }
    };

    class fifo_create_error: public ipc::exception {
        public:
        const char* what() noexcept {
            return "Error while creating fifo";
        }
    };

    class wrong_msg_size: public ipc::exception {
    public:
        const char* what() noexcept {
            return "Message size is wrong";
        }
    };

    class server_process_connected: public ipc::exception {
    public:
        const char* what() noexcept {
            return "There is a process connected as server";
        }
    };
}




#endif /* end of include guard: INTERPROCESS_H_QFGT309Z */

