#include <yurco/all.hpp>
#include <iostream>
#include <stdlib.h>

void entry(yurco::Coroutine& self)
    {
    try
        {
        for (;;)
            self.yield();
        }
    catch (const yurco::terminate_exception&)
        {
        std::cout << "Coroutine should be terminated" << std::endl;
        }
    }

int main()
    {
    yurco::Coroutine coro(yurco::Stack(16*1024), entry);
    coro();
    try
        {
        coro.set_exception(std::make_exception_ptr(yurco::terminate_exception()));
        coro();
        std::cout << "No exception returned from coroutine" << std::endl;
        }
    catch (const yurco::terminate_exception&)
        {
        std::cout << "Terminate exception was returned" << std::endl;
        }
    return EXIT_SUCCESS;
    }
