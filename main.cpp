#include"value.h"
#include<string>
#include<iostream>
int main() {
    minidb::Value bool_v(false);
    bool_v.GetAsBool();

    //bool_v.GetAsInt();assert
    //bool_v.GetAsString();assert
    minidb::Value str_v(std::string("hello world"));
    //str_v.GetAsBool();
    //str_v.GetAsInt();assert
    str_v.GetAsString();
    minidb::Value  int_v(12345);
    //int_v.GetAsBool();
    int_v.GetAsInt();
    //int_v.GetAsString();assert
    std::cout<<bool_v.ToString()<<std::endl;
    std::cout<<str_v.ToString()<<std::endl;
    std::cout<<int_v.ToString()<<std::endl;

    return 0;
}