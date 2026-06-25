#include <iostream>
#include"shell.h"
using namespace minidb;
//create table user(name varchar,age int,sex bool );
//insert into user values('wjh',123,true);
//insert into user values('cmq',456,false);
//insert into user values('sunzi',789,false);
//select*from user where name='sunzi';
//select*from user where age >=456;
//select*from user where age !=456;
//create table student(id int,score int,name varchar);
//insert into student values(0,100,'wzf');
int main() {
    Shell shell("test_filter_plan.db");
    shell.Run();
    return 0;
}
