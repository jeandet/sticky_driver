#include <errno.h>
#include <fcntl.h>
#include <fmt/core.h>
#include <iostream>
#include <sticky.hpp>
#include <string.h>
#include <termios.h>
#include <unistd.h>

bool _configure(int fd)
{
    struct termios tty;

    tcgetattr(fd, &tty);
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    tty.c_lflag = ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);
    tty.c_oflag = ~(OPOST | ONLCR);
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 10;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD); // ignore modem controls,
                                     // enable reading
    tty.c_cflag &= ~(PARENB | PARODD); // shut off parity
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    return -1 != tcsetattr(fd, TCSANOW, &tty);
}

std::vector<uint8_t> Sticky::_read(std::size_t count) const
{
    if (this->_fd != -1)
    {
        {
            std::vector<std::uint8_t> buff(count);
            std::size_t pos = 0;
            while (pos < count)
            {
                const auto cnt = read(this->_fd, buff.data() + pos, std::min(count - pos, 2048UL));
                if (cnt > 0)
                {
                    pos += cnt;
#ifdef STICKY_VERBOSE
                    std::cout << "\r" << fmt::format("Got {} over {}", pos, std::size(buff));
#endif
                }
                else
                {
                    throw std::runtime_error { fmt::format(
                        "error reading port {}", this->_port_name) };
                }
            }
#ifdef STICKY_VERBOSE
            std::cout << std::endl << fmt::format("got all data!") << std::endl;
#endif
            return buff;
        }
    }
    return {};
}

bool Sticky::open(const std::optional<std::string> port_name)
{
    if (port_name)
    {
        this->_port_name = port_name.value();
    }
    this->_fd = ::open(this->_port_name.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (this->_fd != -1)
    {
        this->_configured = _configure(this->_fd);
        return this->_configured;
    }
    return false;
}

bool Sticky::close()
{
    if (this->_fd != -1)
        return -1 != ::close(this->_fd);
    return false;
}

std::vector<uint8_t> Sticky::get_data(std::size_t count) const
{
    this->_read(32768); // flush
    return this->_read(count);
}
