#include "vector.cpp"
#include <cstdio>

int main(){
    vector<int> v;
    for(int i = 0; i < 100; i++)
        v.push_back(i);

    v.remove(52);
    printf("%d\n", v[52]);

    for(int i = 0; i < 90; i++)
        v.pop_back();

    printf("%d %d\n",v.size(), v.capacity());
    return 0;
}
