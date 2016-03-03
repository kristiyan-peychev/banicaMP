#include "audio.h"
#include "device_manager.h"

#include <atomic>
#include <mutex>

static std::atomic_flag manager_initialized = ATOMIC_FLAG_INIT;

//================================================
// portaudio_output_device methods

portaudio_output_device::~portaudio_output_device()
{ }

portaudio_output_device::portaudio_output_device()
{ }

void portaudio_output_device::set_device_info(PaDeviceIndex index)
{
    device_id = index;
    device_info = Pa_GetDeviceInfo(index);
}

void portaudio_output_device::set_next(portaudio_output_device *dev)
{
    next = dev;
}

int portaudio_output_device::get_id() const
{
    return (int) device_id;
}

const char *portaudio_output_device::get_name() const
{
    return device_info->name;
}

double portaudio_output_device::get_default_sample_rate() const
{
    return device_info->defaultSampleRate;
}

int portaudio_output_device::get_max_channels() const
{
    return device_info->maxOutputChannels;
}

//===================================================
// portaudio_enumeration_manager methods

portaudio_enumeration_manager::~portaudio_enumeration_manager()
{
    free_enumeration();
}

portaudio_enumeration_manager::portaudio_enumeration_manager()
: enumerating(ATOMIC_FLAG_INIT)
, enumeration(NULL)
{ }

bool portaudio_enumeration_manager::enumerate()
{
    PaDeviceIndex itr;
    int device_count = Pa_GetDeviceCount();
    portaudio_output_device *dev = NULL;

    for (itr = 0; itr < device_count; ++itr) {
        if (dev == NULL) {
            dev = new portaudio_output_device();
            enumeration = dev;
        } else {
            dev->set_next(new portaudio_output_device());
            dev = dynamic_cast<portaudio_output_device *>(dev->next);
        }
        dev->set_device_info(itr);
    }

    return (NULL != enumeration);
}

const output_device *portaudio_enumeration_manager::get_enumeration()
{
    return enumeration;
}

void portaudio_enumeration_manager::free_enumeration()
{
    if (NULL == enumeration)
        return;

    while (enumeration) {
        portaudio_output_device *tmp = enumeration;
        enumeration = dynamic_cast<portaudio_output_device *>(enumeration->next);
        delete tmp;
    }
    enumeration = NULL;
}

//====================================================
// portaudio_initialization_manager methods

portaudio_initialization_manager::~portaudio_initialization_manager()
{
    terminate_portaudio();
}

portaudio_initialization_manager::portaudio_initialization_manager()
: portaudio_initialized(ATOMIC_FLAG_INIT)
, initflag(false)
{
}

bool portaudio_initialization_manager::initialize_portaudio()
{
    if (portaudio_initialized.test_and_set(std::memory_order_relaxed))
        return false; // portaudio is already initialized

    std::lock_guard<std::mutex> guard(portaudio_lock);
    local_error = Pa_Initialize();
    if (local_error != paNoError)
        return false;
    else {
        initflag = true;
        return true;
    }
}

bool portaudio_initialization_manager::terminate_portaudio()
{
    if (!get_initialized())
        return false;

    portaudio_initialized.clear();

    std::lock_guard<std::mutex> guard(portaudio_lock);
    local_error = Pa_Terminate();
    if (local_error != paNoError)
        return false;
    else {
        initflag = false;
        return true;
    }
}

bool portaudio_initialization_manager::get_initialized() const
{
    return initflag;
}

//===================================================
// portaudio_manager methods

portaudio_manager::~portaudio_manager()
{
    manager_initialized.clear(std::memory_order_relaxed);
}

portaudio_manager::portaudio_manager()
{
    if (manager_initialized.test_and_set(std::memory_order_relaxed))
        throw audio::double_initialization();
}

bool portaudio_manager::initialize_portaudio()
{
    bool ret = portaudio_initialization_manager::initialize_portaudio();
    players = std::vector<portaudio_wav_player *>(Pa_GetDeviceCount());

    return ret;
}

bool portaudio_manager::reg(portaudio_wav_player *player,
                                 PaDeviceIndex device)
{
    if (player == NULL || !get_initialized())
        return false;

    if (device > Pa_GetDeviceCount() - 1)
        return false; // invalid device index

    std::lock_guard<std::mutex> guard(players_lock);
    portaudio_wav_player *requested_device_user = players[device];
    if (requested_device_user != NULL) {
        return false; // device is already in use
    } else {
        if (!get_initialized() && !initialize_portaudio())
            return false; // failed to initialize portaudio
        players[device] = player;
        return true;
    }
}

bool portaudio_manager::unreg(portaudio_wav_player *player)
{
    if (player == NULL || !get_initialized())
        return false;

    std::lock_guard<std::mutex> guard(players_lock);
    for (std::vector<portaudio_wav_player *>::iterator i =
            players.begin(); i != players.end(); ++i) {
        if (*i == player) {
            players.erase(i);
            if (players.size() == 0)
                terminate_portaudio();
            return true;
        }
    }

    return false;
}

