#ifndef INTERPROCESS_H_JDAPM9K3
#define INTERPROCESS_H_JDAPM9K3

#include "../interprocess.h"

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <clocale>
#include <cstdlib>

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
    std::string filename;
    std::string last_msg;
    bool quit;
    std::thread th;
    std::mutex read_mutex;


    void run();

    std::string wstos(const std::wstring&);

};

#endif /* end of include guard: INTERPROCESS_H_JDAPM9K3 */
