#ifndef EVENT_TBFQAKTA

#define EVENT_TBFQAKTA

#include "vector/vector.h"

#define MAKE_REGISTER(x) bool register_##x##(callback &cb)\
                         {\
                            if (!event_##x##.find(cb)) {\
                                event_##x##.push_back(cb);\
                                return true;\
                            }\
                            return false;\
                         }

#define MAKE_UNREGISTER(x)  bool unregister_##x##(callback &cb)\
                            {\
                               if (!event_##x##.find(cb)) {\
                                   event_##x##.erase(cb);\
                                   return true;\
                               }\
                               return false;\
                            }


#define MAKE_EVENT(x)   protected:\
                            vector<callback> event_##x##;\
                        public:\
                            MAKE_REGISTER(x)\
                            MAKE_UNREGISTER(x)\
                            virtual execute_##x##() = 0;

class callback {
    friend class event;
public:
    class callback_context {
        friend class callback;
    protected:
        long ctx;
    public:
       ~callback_context();
        callback_context();
    public:
        bool operator bool();
        long operator long();
        void *operator void *();
    };
protected:
    callback_context context;
public:
    virtual ~callback() { }
    callback() { }
public:
    virtual void on_event() = 0;
};

class event {
public:
    virtual ~event();
    event();
public:
    MAKE_EVENT(play_toggle);
    MAKE_EVENT(playback_begin);
    MAKE_EVENT(playback_stopped);
    MAKE_EVENT(shuffle_toggle);
    MAKE_EVENT(repeat_toggle);
    MAKE_EVENT(track_order_changed);
    MAKE_EVENT(priority_change);
    MAKE_EVENT(next_track_started);
protected:
    void execute(const vector<callback> &event_list,
            callback::callback_context context);
};

#endif /* end of include guard: EVENT_TBFQAKTA */
