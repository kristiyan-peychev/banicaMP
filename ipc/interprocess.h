#ifndef INTERPROCESS_H_QFGT309Z
#define INTERPROCESS_H_QFGT309Z

#include <string>
#include <exception>
#include <stdexcept>

//don't laugh too much
class interprocess_interface {

public:
    interprocess_interface(){};
    interprocess_interface(std::wstring);
    virtual ~interprocess_interface(){};
    
protected:
    virtual std::string serialize(std::string)=0;
    virtual std::string deserialize(std::string)=0;

    void send_msg(std::string);
    void listen();

private:

    void run();
};

namespace ipc {
    class exception: public std::exception {};
}




#endif /* end of include guard: INTERPROCESS_H_QFGT309Z */

