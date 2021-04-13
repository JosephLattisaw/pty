#ifndef PTY__HPP
#define PTY__HPP

#include <string>
#include <boost/asio.hpp>

class Pty
{
public:
    Pty(boost::asio::io_service &io_service);

    bool open();
    void write();
    void close();

private:
    void master_async_read();
    void slave_async_read();

    bool own_master = true; //not just 1 bit
    int master_fd = -1;
    int slave_fd = -1;

    std::string tty_name;

    boost::asio::io_service &io_service;
    boost::asio::ip::tcp::socket socket;

    boost::asio::posix::stream_descriptor master_input;
    boost::asio::streambuf master_input_buffer;

    boost::asio::posix::stream_descriptor slave_input;
    boost::asio::streambuf slave_input_buffer;
};

#endif