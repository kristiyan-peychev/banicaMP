#ifndef CONFIG_P7SQO4MR

#define CONFIG_P7SQO4MR

#include <cstdio>
#include <ostream>
#include <exception>

#define CURRENT_CONFIG_VERSION 1L
#define FILTERS_SIZE 10

class fopen_except : public std::exception {
public:
    const char *what(void) const noexcept override {
        return "Failed to open file";
    }
};

class config {
    long version; // Confirming to; flags?
    unsigned long max_decoded_songs; // At any time
    unsigned long min_decoded_songs; // ^
    long last_volume; // Not yet implemented; in dB?
    float filters[FILTERS_SIZE];
public:
    config(void);
    config(const char *);
public:
    void set_max_songs(long v) {
        max_decoded_songs = v >= min_decoded_songs ? v : min_decoded_songs + 1;
    }
    void set_min_songs(long v) {
        min_decoded_songs = v <= max_decoded_songs ? v : 1;
    }
    void set_last_vol(long v) { last_volume = v; }
public:
    long get_version(void) const { return version; }
    unsigned long get_max_songs(void) const { return max_decoded_songs; }
    unsigned long get_min_songs(void) const { return min_decoded_songs; }
    long get_last_vol(void) const { return last_volume; }
public:
    void flush_conf_in_file(const char *) const throw();
    friend std::ostream &operator<<(std::ostream &, const config &);
};

#endif /* end of include guard: CONFIG_P7SQO4MR */
