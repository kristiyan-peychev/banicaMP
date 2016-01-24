#ifndef INTERPROCESS_H_JDAPM9K3
#define INTERPROCESS_H_JDAPM9K3

#include "../interprocess.h"

#include <unistd.h>

class interprocess: public interprocess_interface {

public:
    interprocess(std::wstring);
    virtual ~interprocess();
    
protected:
    virtual std::wstring serialize(std::wstring);
    virtual void deserialize(std::wstring);

    void send_msg(std::wstring);

private:
    int fd;
    bool quit;
    static const int MSG_SIZE=10;

    void run();
    void listen();
};

#endif /* end of include guard: INTERPROCESS_H_JDAPM9K3 */
