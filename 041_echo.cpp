#include <yurco/all.hpp>
#include <unistd/signalfd.hpp>
#include <unistd/addrinfo.hpp>
#include <unistd/netdb.hpp>
#include <iostream>
#include <signal.h>
#include <stdlib.h>

void process_connection(unistd::fd& clientfd)
    {
    char buf[1024];
    try
        {
        const size_t nread = unistd::read(clientfd, buf, sizeof(buf));
        if (0 == nread)
            return; // terminate coroutine, close connection
        unistd::write_all(clientfd, buf, nread);
        }
    catch (const yurco::terminate_exception&)
        {
        std::cerr << "terminate connection coroutine" << std::endl;
        }
    catch (...)
        {
        std::cerr << "got unknown exception while read/write" << std::endl;
        }
    }

void listener(unistd::fd& serverfd)
    {
    try
        {
        for (;;)
            {
            for (size_t i = 0; i < 32; ++i) // sometimes we should yield() to process accepted connections
                {
                unistd::fd clientfd = unistd::fd::nodup(unistd::accept(serverfd, SOCK_NONBLOCK));
                yurco::async(process_connection, std::move(clientfd));
                }
        yurco::yield();
            }
        }
    catch (const yurco::terminate_exception&)
        {
        std::cerr << "terminate listener coroutine" << std::endl;
        }
    catch (...)
        {
        std::cerr << "unknwon exception while accept" << std::endl;
        }
    }

void signal_handler(unistd::fd& sigfd)
    {
    yurco::suspend(sigfd, EPOLLIN);
    std::cerr << "we got a signal" << std::endl;
    // use unistd::read(sigfd) to get a signals
    yurco::terminate();
    }

void register_signal_handler()
    {
    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGINT);
    sigaddset(&sigmask, SIGTERM);
    sigaddset(&sigmask, SIGQUIT);
    unistd::fd sigfd = unistd::fd::nodup(unistd::signalfd(sigmask));
    yurco::async(signal_handler, std::move(sigfd)); // use std::move to avoid ::dup() file descriptior
    sigaddset(&sigmask, SIGPIPE); // SIGPIPE just ignored and not processed
    sigprocmask(SIG_BLOCK, &sigmask, nullptr);
    }

void register_listener()
    {
    const std::vector<unistd::addrinfo> addr = unistd::getaddrinfo("localhost:31337"); // or [::]:31337 or other valid variants
    unistd::fd serverfd = unistd::fd::nodup(unistd::socket(addr.at(0), SOCK_NONBLOCK));
    unistd::setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, 1);
    unistd::bind(serverfd, addr.at(0));
    unistd::listen(serverfd, 8192/*backlog*/);
    yurco::async(listener, std::move(serverfd)); // use std::move to avoid ::dup() file descriptior
    }

int main()
    {
    const size_t stack_size = 16*1024; // size less than 16k lead to SIGSEGV cause libunwind require more space
    yurco::init(stack_size);
    register_signal_handler();
    register_listener();
    yurco::run();
    return EXIT_SUCCESS;
    }

