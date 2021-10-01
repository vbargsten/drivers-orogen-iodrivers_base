#include "PortStream.hpp"
#include <iodrivers_base/Exceptions.hpp>

using namespace iodrivers_base;

PortStream::PortStream(RTT::InputPort<RawPacket>& in, RTT::OutputPort<RawPacket>& out)
    : mIn(in), mOut(out) {}

bool PortStream::hasQueuedData()
{
    if (mPacketRead.data.empty())
        return (mIn.read(mPacketRead, false) == RTT::NewData);
    else
        return true;
}

bool PortStream::waitRead(base::Time const& timeout)
{
    if (!mPacketRead.data.empty())
        return true;

    uint64_t sleep_time;
    int count;
    uint64_t full_timeout = timeout.toMicroseconds();
    if (full_timeout > 10000)
    {
        sleep_time = 10000;
        count = (full_timeout + sleep_time - 1) / sleep_time;
    }
    else
    {
        count = 1;
        sleep_time = full_timeout;
    }

    for (int i = 0; i < count; ++i)
    {
        if (mIn.read(mPacketRead, false) == RTT::NewData)
            return true;
        usleep(sleep_time);
    }
    return false;
}
bool PortStream::waitWrite(base::Time const& timeout)
{
    return true;
}
size_t PortStream::read(uint8_t* buffer, size_t buffer_size)
{
    if (mPacketRead.data.empty())
    {
        if (mIn.read(mPacketRead, false) != RTT::NewData)
            return 0;
    }

    size_t data_size = std::min(buffer_size, mPacketRead.data.size());
    memcpy(&buffer[0], &mPacketRead.data[0], data_size);
    size_t remaining = mPacketRead.data.size() - data_size;
    memmove(&mPacketRead.data[0], &mPacketRead.data[data_size], remaining);
    mPacketRead.data.resize(remaining);
    return data_size;
}
size_t PortStream::write(uint8_t const* buffer, size_t buffer_size)
{
    mPacketWrite.data.clear();
    mPacketWrite.data.insert(mPacketWrite.data.begin(), buffer, buffer + buffer_size);
    mPacketWrite.time = base::Time::now();
    mOut.write(mPacketWrite);
    return buffer_size;
}
void PortStream::clear()
{
    mIn.clear();
}

PortListener::PortListener(RTT::OutputPort<RawPacket>& out_read, RTT::OutputPort<RawPacket>& out_write)
    : mOutRead(out_read), mOutWrite(out_write) {}

void PortListener::writeData(boost::uint8_t const* buffer, size_t buffer_size)
{
    if (mOutWrite.connected())
    {
        mPacket.time = base::Time::now();
        mPacket.data.clear();
        mPacket.data.insert(mPacket.data.begin(), buffer, buffer + buffer_size);
        mOutWrite.write(mPacket);
    }
}
void PortListener::readData(boost::uint8_t const* buffer, size_t buffer_size)
{
    if (mOutRead.connected())
    {
        mPacket.time = base::Time::now();
        mPacket.data.clear();
        mPacket.data.insert(mPacket.data.begin(), buffer, buffer + buffer_size);
        mOutRead.write(mPacket);
    }
}

