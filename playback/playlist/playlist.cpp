#include "playlist.h"

Observer observer;

playlist::decoded_song_handler::decoded_song_handler(void) :
       configuration(), decoded_songs_list(configuration.get_max_songs())
{ }

playlist::decoded_song_handler::decoded_song_handler(const vector<song*> &c) :
        configuration(), decoded_songs_list(c)
{
    for (int i = 0; i < configuration.get_min_songs() &&
            i < decoded_songs_list.size(); i++)
        decoded_songs_list[i]->load_song();
}

void playlist::decoded_song_handler::push(song* s) noexcept
{
    if (decoded_songs_list.size() >= configuration.get_max_songs()) {
        decoded_songs_list[0]->clear_song();
        decoded_songs_list.remove(0);
    }

    decoded_songs_list.push_back(s);
    s->load_song();
}

playlist::playlist(const char* path, bool is_playlist): list(), size(0), curr_song(0),
    queue_pos(0), playing_now(false), repeat(false), shuffle(false)
{
    audio::initialize();

    if(path){
        if(is_playlist)
            load(path);
        else
            generate(path);
    }
}

playlist::playlist(): playing_now(false), repeat(false), shuffle(false),
        paused(true), queue_pos(0), pq_pos(0)
{
    audio::initialize();
}

playlist::~playlist()
{
    size_t size = list.size();
    for(int i = 0; i < size; i++)
        delete list[i];

    audio::terminate();
}

void playlist::generate(const char* path)
{
    //NEEDS MORE ERROR HANDLING
    tinydir_dir dir;
    tinydir_open(&dir, path);
    while(dir.has_next){
        tinydir_file file;

        if(tinydir_readfile(&dir, &file) == -1){
            tinydir_next(&dir);
            continue;
        }

        //if file is supported add to playlist
        if(!file.is_dir && get_file_encoding(file.path) != ENC_UNK) {
            add_song(file.path);
        }

        //recursively search all dirs
        if(file.is_dir && strcmp(file.name, ".") && strcmp(file.name, ".."))
            generate(file.path);

        tinydir_next(&dir);
    }
    tinydir_close(&dir);
}

void playlist::print_songs()
{
    for(int i = 0; i < list.size(); i++)
        printf("%d: %s %d\n",i, list[i]->get_info().title.toCString(true), list[i]->get_info().length);

    //for(int i = 0; i < queue.size(); i++)
    //    printf("%d ", queue[i]);
    printf("\n");
}

//load from playlist
void playlist::load(const char* path)
{

}

//save to playlist
void playlist::save(const char* path)
{
}

void playlist::play_song(size_t idx)
{
    //set current queue pos
    queue_pos = idx;
    play_song(queue[idx]);
}

void playlist::play_song(song *s)
{
    if (s == NULL)
        throw std::out_of_range("index is out of range");

    //unpause when clicked on paused song
    if(paused && s == curr_song){
        pause_song();
        return;
    }

    //stop previous song
    if (playing_now) {
        stop_song();
        playing_now = false;
    }
    
    curr_song = s;
    curr_song->start();
    playing_now = true;
}

void playlist::play_next_song()
{
    song *to_play;
    if (playing_now) {
        stop_song();
        playing_now = false;
    }

    if (!priority_queue.empty() && pq_pos < priority_queue.size()) {
        to_play = priority_queue[++pq_pos]; // FIXME
    } else {
        if (pq_pos >= priority_queue.size()) {
            priority_queue.clear();
            pq_pos = 0;
        }

        to_play = queue[(++queue_pos %= size)];
    }

    if (!repeat && queue_pos >= queue.size() - 1){
        printf("end of playlist\n");
        return;
    }

    curr_song->stop();
    curr_song = to_play;
    curr_song->start();
}

void playlist::pause_song()
{
    if(playing_now){
        curr_song->pause();
        paused = true;
    }
}

void playlist::stop_song()
{
    if(playing_now){
        curr_song->stop();
        playing_now = false;
    }
}

void playlist::seek(int secs)
{
    if(playing_now)
        curr_song->seek(secs);
}

void playlist::remove_song(size_t idx)
{
    remove_song(queue[idx]);
}

void playlist::remove_song(song *s)
{
    ssize_t idx;

    idx = list.find(s);
    if (idx > 0) {
        list.remove(idx);
        return;
    }

    idx = queue.find(s);
    if (idx > 0)
        queue.remove(idx);


    idx = priority_queue.find(s);
    if (idx > 0)
        priority_queue.remove(idx);
}

void playlist::add_song(const char* path)
{
    song* new_song = new song(path);
    list.push_back(new_song);
    queue.push_back(list.last());
}

void playlist::enqueue_prioriy(song *s)
{
    if (priority_queue.empty())
        pq_pos = 0;

    priority_queue.push_back(s);
}

void playlist::remove_priority(size_t idx)
{
    priority_queue.remove(idx);
}

void playlist::remove_priority(song *s)
{
    size_t idx = priority_queue.find(s);
    priority_queue.remove(idx);
}

void playlist::clear_priority()
{
    priority_queue.clear();
}

// we're going to try and make list a set
// of the songs within the playlist, since
// they are going to be freed at the destructor
// call, and if two pointers point to the same
// song we're going to have a bad time, thus
// a linear-time enqueueing of a song
void playlist::add_song(song* s)
{
    size_t idx;
    if ((idx = list.find(s)) < 0) {
        list.push_back(s);
        queue.push_back(list.last());
    } else {
        queue.push_back(list[idx]);
    }
}

void playlist::shuffle_list()
{
    priority_queue.clear();
    srand(time(NULL));
    for (size_t i = queue.size() - 1; i > 0; i--) {
        size_t r = rand() % queue.size();
        std::swap(queue[i], queue[r]);
    }
}

void playlist::toggle_repeat()
{
    repeat = !repeat;
}

static std::function<bool(song*&, song*&)> comp_song(char ch)
{
    switch (ch){
    case 't' : return [](song *&s1, song *&s2) {return s1->get_info().title < s2->get_info().title;};
    case 'a' : return [](song *&s1, song *&s2) {return s1->get_info().artist < s2->get_info().artist;};
    case 'l' : return [](song *&s1, song *&s2) {return s1->get_info().length < s2->get_info().length;};
    default : return  [](song *&s1, song *&s2) {return s1->get_info().title < s2->get_info().title;};
    }

}

void playlist::toggle_shuffle()
{
    if (shuffle) {
        auto compar = comp_song('t');
        shuffle = false;
        queue.sort(compar);
    } else {
        shuffle = true;
        shuffle_list();
    }
}


