#include "pty.hpp"

#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cassert>
#include <iostream>

#include <boost/exception/diagnostic_information.hpp>

Pty::Pty(boost::asio::io_service &io_service) : io_service(io_service), socket(io_service), master_input(io_service), slave_input(io_service)
{
}

bool Pty::open()
{
    assert(master_fd == -1);

    own_master = true;

    master_fd = posix_openpt(O_RDWR | O_NOCTTY);

    if (master_fd >= 0)
    {
        int pty_nbr = 0; // I don't trust ioctl, so initializing
        if (!ioctl(master_fd, TIOCGPTN, &pty_nbr))
        {
            tty_name = "/dev/pts/" + std::to_string(pty_nbr);

            struct stat st;
            if (stat(tty_name.c_str(), &st))
            {
                std::cerr << "pty: stat experienced an error: " << errno << std::endl;
            }
            else
            {
                int flg = 0;
                ioctl(master_fd, TIOCSPTLCK, &flg);
                slave_fd = ::open(tty_name.c_str(), O_RDWR | O_NOCTTY);

                if (slave_fd < 0)
                {
                    std::cerr << "pty: couldn't open slave psuedo terminal" << std::endl;
                }
                else
                {
                    fcntl(master_fd, F_SETFD, FD_CLOEXEC);
                    fcntl(slave_fd, F_SETFD, FD_CLOEXEC);
                    //input.assign(::dup(master_fd));

                    //dup2(slave_fd, 0);
                    //dup2(slave_fd, 1);
                    //dup2(slave_fd, 2);
                    master_input.assign(master_fd);
                    slave_input.assign(slave_fd);
                    master_async_read();
                    slave_async_read();
                    return true;
                }
            }
        }
        else
            std::cerr << "pty: failed to get pty number" << std::endl;
    }
    else
        std::cerr << "pty: failed to open master posix terminal" << std::endl;

    ::close(master_fd);
    ::close(slave_fd);
    master_fd = -1;
    slave_fd = -1;
    return false;
}

void Pty::master_async_read()
{
    boost::asio::async_read_until(master_input, master_input_buffer, "\n", [&](const boost::system::error_code &error, std::size_t bytes_transferred)
                                  {
                                      if (!error)
                                      {
                                          std::cout << "master async reading data" << std::endl;
                                          const char *data = boost::asio::buffer_cast<const char *>(master_input_buffer.data());
                                          std::cout << "data: " << data << std::endl;
                                          master_input_buffer.consume(master_input_buffer.size());
                                          master_async_read();
                                      }
                                      else
                                      {
                                          std::cout << "master error reading data " << error.message() << std::endl;
                                          try
                                          {
                                              slave_input.close();
                                          }
                                          catch (boost::exception const &e)
                                          {
                                              std::cerr << boost::diagnostic_information(e) << std::endl;
                                          }
                                      }
                                  });
}

void Pty::slave_async_read()
{
    boost::asio::async_read_until(slave_input, slave_input_buffer, "\n", [&](const boost::system::error_code &error, std::size_t bytes_transferred)
                                  {
                                      if (!error)
                                      {
                                          std::cout << "slave async reading data" << std::endl;
                                          const char *data = boost::asio::buffer_cast<const char *>(slave_input_buffer.data());
                                          std::cout << "data: " << data << std::endl;
                                          slave_input_buffer.consume(slave_input_buffer.size());
                                          slave_async_read();
                                      }
                                      else
                                      {
                                          std::cout << "slave error reading data " << error.message() << std::endl;
                                          try
                                          {
                                              master_input.close();
                                          }
                                          catch (boost::exception const &e)
                                          {
                                              std::cerr << boost::diagnostic_information(e) << std::endl;
                                          }
                                      }
                                  });
}

void Pty::write()
{
    std::string a = "joe\n";
    boost::system::error_code ec;
    boost::asio::write(master_input, boost::asio::buffer(a.c_str(), a.size()));
    if (ec)
    {
        std::cout << "error: " << ec.message() << std::endl;
    }
}

void Pty::close()
{
    std::cout << "is fd open?" << master_input.is_open() << " " << master_input.is_open() << std::endl;
    //::close(master_fd);
    ::close(slave_fd);
    std::cout << "is fd open?" << master_input.is_open() << " " << slave_input.is_open() << std::endl;
    //input.close();
}