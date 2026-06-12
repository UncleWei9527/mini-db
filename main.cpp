#include"value.h"
#include<string>
#include<iostream>
#include"disk_manager.h"
#include"lru_replacer.h"
#include<cassert>
int main() {
    minidb::LRUReplacer lru(3);
    lru.Unpin(1);
    lru.Unpin(2);
    lru.Unpin(0);
    lru.Pin(2);
    auto num=lru.Victim();
    assert(num.has_value()&&num.value()==1);
    num=lru.Victim();
    assert(num.has_value()&&num.value()==0);
    num=lru.Victim();
    assert(!num.has_value());
    return 0;
}