#include "MainApplication.h"

int main(int argc, char *argv[])
{
    MainApplication app(argc, argv);
    
    if (!app.initialize()) {
        return -1;
    }
    
    return app.exec();
}
