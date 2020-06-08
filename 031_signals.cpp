#include <yurco/all.hpp>
#include <unistd/signalfd.hpp>

#include <iostream>
#include <signal.h>
#include <stdlib.h>

void signal_handler(yurco::Coroutine& coro, yurco::Reactor& reactor, unistd::fd& sigfd)
    {
    reactor.suspend(coro, sigfd, EPOLLIN);
    std::cerr << "we got a signal" << std::endl;
    reactor.terminate();
    }

void register_signal_handler(yurco::Reactor& reactor)
    {
    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGINT);
    sigaddset(&sigmask, SIGTERM);
    sigaddset(&sigmask, SIGQUIT);
    unistd::fd sigfd = unistd::fd::nodup(unistd::signalfd(sigmask));
    reactor.async(signal_handler, std::ref(reactor), std::move(sigfd)); // use std::move to avoid ::dup() file descriptior
    sigaddset(&sigmask, SIGPIPE); // SIGPIPE just ignored and not processed
    sigprocmask(SIG_BLOCK, &sigmask, nullptr);
    }

int main()
    {
    const size_t stack_size = 16*1024; // size less than 16k lead to SIGSEGV cause libunwind require more space
    yurco::Reactor reactor(stack_size);
    register_signal_handler(reactor);
    reactor.run();
    return EXIT_SUCCESS;
    }
