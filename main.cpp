#include "mainwindow.h"

#include <iostream>
#include <fstream>

#pragma comment(lib, "user32.lib")

int main(int argc, char *argv[])
{
    try
    {
        QApplication a(argc, argv);
        mainwindow w;
        w.show();
        int ret = a.exec();
        return ret;
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
    }
    return 0;
}
