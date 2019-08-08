#include <unistd.h>

#include "base/Logging.h"

int main(int argc, char* argv[])
{
    /* 默认是/dev/stdout */
    //Logger::setLogFile("xxx.log");

    for(int i = 0; i < 100; ++i)
        LOG_DEBUG("hello %d\n", i);

    for(int i = 0; i < 100; ++i)
        LOG_WARNING("hello %d\n", i);
    
    sleep(1);

    for(int i = 0; i < 100; ++i)
        LOG_ERROR("hello %d\n", i);

    while(1)
        sleep(1);

    return 0;
}