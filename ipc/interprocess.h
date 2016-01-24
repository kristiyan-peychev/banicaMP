#ifndef INTERPROCESS_H_QFGT309Z
#define INTERPROCESS_H_QFGT309Z

#include <string>
#include <exception>
#include <stdexcept>

//don't laugh too much
class interprocess_interface {

public:
    interprocess_interface(std::wstring);
    virtual ~interprocess_interface();
    
protected:
    virtual std::wstring serialize(std::wstring);
    virtual void deserialize(std::wstring);

    void send_msg(std::wstring);

private:

    void run();
    void listen();
};

namespace ipc {
    class exception: public std::exception {};
}




#endif /* end of include guard: INTERPROCESS_H_QFGT309Z */

