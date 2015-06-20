#include "../playback/playlist/vector/vector.hpp"

int main(int argc, char *argv[])
{
    vector<int> a;
    for(int i = 0; i < 10; i++)
        a.push_back(i);

    vector<int>::iterator it = a.begin();
    while(it != a.end()){
        printf("%d ",*(it));
        ++it;
    }
    printf("\n");
    it = a.rend();
    while(it != a.rbegin()){
        printf("%d ", *it);
        --it;
    }
    printf("\n");
    return 0;
}
