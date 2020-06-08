#include <yurco/all.hpp>
#include <stdlib.h>

void entry(yurco::Coroutine& self)
    {
    for (;;)
        self.yield();
    }

int main()
    {
    yurco::Coroutine coro(yurco::Stack(16*1024), entry);
    for (uint32_t i = 0; i < 5000000; ++i)
        coro();
    return EXIT_SUCCESS;
    }
