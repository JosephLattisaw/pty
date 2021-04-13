#include <iostream>
#include <memory>
#include <boost/asio.hpp>

#include "pty.hpp"

int main(int, char **)
{
    boost::asio::io_service io_service;
    auto pty = std::make_unique<Pty>(io_service);
    std::cout << "pty opened? " << (pty->open() ? "true" : "false") << std::endl;
    //pty->write();
    //pty->write();
    //pty->close();

    boost::asio::steady_timer timer1(io_service);
    timer1.expires_after(std::chrono::seconds(2));
    timer1.async_wait([&](const boost::system::error_code &error)
                      { pty->write(); });

    boost::asio::steady_timer timer2(io_service);
    timer2.expires_after(std::chrono::seconds(5));
    timer2.async_wait([&](const boost::system::error_code &error)
                      { pty->write(); });

    boost::asio::steady_timer timer3(io_service);
    timer3.expires_after(std::chrono::seconds(8));
    timer3.async_wait([&](const boost::system::error_code &error)
                      { pty->close(); });

    /*boost::asio::steady_timer timer4(io_service);
    timer4.expires_after(std::chrono::seconds(12));
    timer4.async_wait([&](const boost::system::error_code &error)
                      { pty->write(); });*/

    io_service.run();
}
