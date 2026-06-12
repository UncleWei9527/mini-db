#include"value.h"
#include<string>
#include<iostream>
#include"disk_manager.h"
int main() {
    minidb::DiskManager disk_mgr("test.db");
    std::string buffer(minidb::PAGE_SIZE, 'A');
    disk_mgr.WritePage(0, buffer);
    for (int i=0;i<buffer.size();i++) {
       buffer[i]=rand()%26+'a';
    }
    disk_mgr.ReadPage(0,buffer);
    for (int i=0;i<buffer.size();i++) {
        if (buffer[i]!='A')
            throw std::logic_error("read error");
    }
    return 0;
}