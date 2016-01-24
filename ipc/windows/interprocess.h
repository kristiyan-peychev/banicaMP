#ifndef INTERPROCESS_H_61QS3BL0
#define INTERPROCESS_H_61QS3BL0

#include "../interprocess.h"

class interprocess: public interprocess_interface {
public:
    interprocess(std::wstring);
    virtual ~interprocess();
    
protected:
    virtual std::wstring serialize(std::wstring);
    virtual void deserialize(std::wstring);

    void send_msg(std::wstring);

private:

    void run();
    void listen();
};

#endif /* end of include guard: INTERPROCESS_H_61QS3BL0 */

