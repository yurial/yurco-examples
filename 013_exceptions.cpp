#include <yurco/all.hpp>
#include <iostream>
#include <stdlib.h>

void entry(yurco::Coroutine& self)
    {
    std::cout << "Coroutine started" << std::endl;
    throw std::runtime_error("My Exception ;)");
    }

int main()
    {
    yurco::Coroutine coro(yurco::Stack(16*1024), entry);
    try
        {
        coro(std::nothrow); // noexcept version should ignore any returned exception
        std::cout << "1: No exception" << std::endl;
        }
    catch (const std::exception& e)
        {
        std::cout << "1: Exception: " << e.what() << std::endl;
        }

    try
        {
        coro.rethrow();
        std::cout << "2: No exception" << std::endl;
        }
    catch (const std::exception& e)
        {
        std::cout << "2: Exception: " << e.what() << std::endl;
        }

    try
        {
        coro();
        std::cout << "3: No exception" << std::endl;
        }
    catch (const std::exception& e)
        {
        std::cout << "3: Exception: " << e.what() << std::endl;
        }
    return EXIT_SUCCESS;
    }
