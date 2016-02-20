#ifndef DEVICE_MANAGER_CAF0PPUT

#define DEVICE_MANAGER_CAF0PPUT

#include <portaudio.h>
#include <mutex>
#include <atomic>
#include <vector>

#include "../audio.h"

class portaudio_output_device : public output_device {
    PaDeviceIndex       device_id;
    const PaDeviceInfo *device_info;
public:
    ~portaudio_output_device();
    portaudio_output_device();
public:
    void        set_device_info(PaDeviceIndex index);
    void        set_next(portaudio_output_device *dev);
public:
    friend class portaudio_enumeration_manager;
public:
    int         get_id() const;
    const char *get_name() const;
    double      get_default_sample_rate() const;
    int         get_max_channels() const;
};

class portaudio_enumeration_manager {
    std::atomic_flag         enumerating;
    portaudio_output_device *enumeration;
public:
    virtual ~portaudio_enumeration_manager();
    portaudio_enumeration_manager();
public:
    bool                    enumerate();
    const output_device    *get_enumeration();
    void                    free_enumeration();
};

class portaudio_initialization_manager {
    PaError                             local_error;
    std::atomic_flag                    portaudio_initialized;
    std::mutex                          portaudio_lock;
protected:
    bool    initflag;
public:
    virtual ~portaudio_initialization_manager();
            portaudio_initialization_manager();
public:
    bool    initialize_portaudio();
    bool    terminate_portaudio();
    PaError get_last_error() const;
    bool    get_initialized() const;
};

/* class portaudio_manager
 *
 * This class shall be responsible for giving out output devices
 * to portaudio_wav_player objects and managing the portaudio's initialization.
 *
 */

class portaudio_wav_player;

class portaudio_manager : public portaudio_initialization_manager {
    std::mutex                          players_lock;
    std::vector<portaudio_wav_player *> players;
public:
   ~portaudio_manager();
    portaudio_manager();
public:
    bool reg(portaudio_wav_player *player, PaDeviceIndex device);
    bool unreg(portaudio_wav_player *player);
public:
    portaudio_manager(portaudio_manager&)            = delete;
    portaudio_manager& operator=(portaudio_manager&) = delete;
public: // static methods

};


#endif /* end of include guard: DEVICE_MANAGER_CAF0PPUT */
