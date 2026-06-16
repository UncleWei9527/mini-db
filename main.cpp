#include <iostream>
#include"shell.h"
using namespace minidb;


int main() {
    Shell shell("test_shell.db");
    shell.Run();
    return 0;
}