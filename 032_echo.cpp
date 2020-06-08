#include <yurco/all.hpp>
#include <unistd/signalfd.hpp>
#include <unistd/addrinfo.hpp>
#include <unistd/netdb.hpp>
#include <iostream>
#include <signal.h>
#include <stdlib.h>

void process_connection(yurco::Coroutine& coro, yurco::Reactor& reactor, int& clientfd)
    {
    (void)coro;
    char buf[1024];
    try
        {
        const size_t nread = yurco::read(reactor, coro, clientfd, buf, sizeof(buf));
        if (0 == nread)
            return; // terminate coroutine, close connection
        for (size_t nwrite = 0; nwrite < nread;)
            nwrite += yurco::write(reactor, coro, clientfd, buf+nwrite, nread-nwrite);
        }
    catch (const yurco::terminate_exception&)
        {
        std::cerr << "terminate connection coroutine" << std::endl;
        }
    catch (...)
        {
        std::cerr << "got unknown exception while read/write" << std::endl;
        }
    reactor.close(std::nothrow, clientfd);
    }

void listener(yurco::Coroutine& coro, yurco::Reactor& reactor, int serverfd)
    {
    try
        {
        for (;;)
            {
            for (size_t i = 0; i < 32; ++i) // sometimes we should yield() to process accepted connections
                {
                int clientfd = yurco::accept(reactor, coro, serverfd, nullptr, nullptr, SOCK_NONBLOCK);
                reactor.async(process_connection, std::ref(reactor), clientfd);
                }
            coro.yield();
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
    reactor.close(std::nothrow, serverfd);
    }

void signal_handler(yurco::Coroutine& coro, yurco::Reactor& reactor, int sigfd)
    {
    reactor.suspend(coro, sigfd, EPOLLIN);
    std::cerr << "we got a signal" << std::endl;
    // use unistd::read(sigfd) to get a signals
    reactor.terminate();
    reactor.close(std::nothrow, sigfd);
    }

void register_signal_handler(yurco::Reactor& reactor)
    {
    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGINT);
    sigaddset(&sigmask, SIGTERM);
    sigaddset(&sigmask, SIGQUIT);
    int sigfd = unistd::signalfd(sigmask);
    reactor.async(signal_handler, std::ref(reactor), sigfd); // use std::move to avoid ::dup() file descriptior
    sigaddset(&sigmask, SIGPIPE); // SIGPIPE just ignored and not processed
    sigprocmask(SIG_BLOCK, &sigmask, nullptr);
    }

void register_listener(yurco::Reactor& reactor)
    {
    const std::vector<unistd::addrinfo> addr = unistd::getaddrinfo("localhost:31337"); // or [::]:31337 or other valid variants
    int serverfd = unistd::socket(addr.at(0), SOCK_NONBLOCK);
    unistd::setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, 1);
    unistd::bind(serverfd, addr.at(0));
    unistd::listen(serverfd, 8192/*backlog*/);
    reactor.async(listener, std::ref(reactor), serverfd); // use std::move to avoid ::dup() file descriptior
    }

int main()
    {
    const size_t stack_size = 16*1024; // size less than 16k lead to SIGSEGV cause libunwind require more space
    yurco::Reactor reactor(stack_size);
    register_listener(reactor);
    register_signal_handler(reactor);
    reactor.run();
    return EXIT_SUCCESS;
    }

