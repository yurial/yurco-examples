#include <yurco/all.hpp>
#include <iostream>
#include <stdlib.h>

void entry(yurco::Coroutine& self, yurco::SimpleScheduler& scheduler, int my_id)
    {
    for (uint32_t i=0; i < 2; ++i)
        {
        std::cout << "my_id" << my_id << std::endl;
        self.yield();
        }
    scheduler.suspend(self);
    for (;;)
        self.yield();
    }

int main()
    {
    std::atomic<bool> terminate_flag = false;
    yurco::SimpleScheduler scheduler(terminate_flag, 16*1024, true);
    scheduler.coroutine(entry, std::ref(scheduler), 1);
    scheduler.coroutine(entry, std::ref(scheduler), 2);
    while (scheduler.try_execute_one())
        ;
    terminate_flag = true;
    scheduler.resume_all();
    while (scheduler.try_execute_one())
        ;
    std::cout << "Coroutines was terminated via yurco::terminate_exception, cause terminate_flag set to true" << std::endl;
    return EXIT_SUCCESS;
    }
