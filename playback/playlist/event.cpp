#include "event.h"

callback::~callback_context()
{ }

callback::callback_context()
{ }

bool callback::callback_context::operator bool()
{
    return static_cast<bool>(ctx);
}

long callback::callback_context::operator long()
{
    return static_cast<long>(ctx);
}

void *callback::callback_context::operator void *()
{
    return static_cast<void *>(ctx);
}

event::event()
{ }

void event::execute(const vector<callback> &event_list,
        callback::callback_context context)
{
    if (event_list.size() == 0)
        return;

    std::thread th;
    for (size_t i = 0; i < event_list.size(); ++i) {
        event_list[i].context = context;
        th = std::thread(event_list[i].on_event);
        th.detach();
    }
}
