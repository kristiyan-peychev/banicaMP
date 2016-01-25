#ifndef INTERPROCESS_H_JDAPM9K3
#define INTERPROCESS_H_JDAPM9K3

#include "../interprocess.h"

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <clocale>
#include <cstdlib>
#include <iostream> //remove later
class interprocess: public interprocess_interface {

public:
    interprocess(std::wstring);
    virtual ~interprocess();
    
protected:
    virtual std::string serialize(std::string str){ return str;}
    virtual std::string deserialize(std::string str){ return str;}
public:
    void send_msg(std::string);
    void listen();

    void set_msg_size(size_t size){ msg_size = size;}

private:
    std::string filename;
    bool quit;
    size_t msg_size;
    std::thread thread;

    void run();

    std::string wstos(std::wstring);
};

#endif /* end of include guard: INTERPROCESS_H_JDAPM9K3 */
