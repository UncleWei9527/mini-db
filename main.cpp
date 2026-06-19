#include <iostream>
#include"shell.h"
using namespace minidb;
//create table user(name varchar,age int,sex bool );
//insert into user values('wjh',123,true);
//create table student(id int,score int,name varchar);
//insert into student values(0,100,'wzf');
int main() {
  Shell shell("test_catalog_serialize.db");
   shell.Run();
    printf("hello");
    return 0;
}