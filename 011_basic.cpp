#include <yurco/all.hpp>
#include <iostream>
#include <stdlib.h>

void entry(yurco::Coroutine& self)
    {
    std::cout << "At 3" << std::endl;
    self.yield();
    std::cout << "At 5" << std::endl;
    }

int main()
    {
    std::cout << "At 1" << std::endl;
    yurco::Coroutine coro(yurco::Stack(16*1024), entry);
    std::cout << "At 2" << std::endl;
    coro();
    std::cout << "At 4" << std::endl;
    coro();
    std::cout << "At 6" << std::endl;
    return EXIT_SUCCESS;
    }
