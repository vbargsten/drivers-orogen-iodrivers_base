#include "PortStream.hpp"
#include <iodrivers_base/Exceptions.hpp>

using namespace iodrivers_base;

PortStream::PortStream(RTT::InputPort<RawPacket>& in, RTT::OutputPort<RawPacket>& out)
    : mIn(in), mOut(out), mHasRead(false) {}

bool PortStream::hasQueuedData()
{
    if (!mHasRead && mIn.read(mPacketRead) == RTT::NewData)
        mHasRead = true;
    return mHasRead;
}

void PortStream::waitRead(base::Time const& timeout)
{
    if (mHasRead)
        return;

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
        if (mIn.read(mPacketRead) == RTT::NewData)
        {
            mHasRead = true;
            return;
        }
        usleep(sleep_time);
    }
    throw TimeoutError(TimeoutError::NONE, "waitRead(): timeout");
}
void PortStream::waitWrite(base::Time const& timeout)
{
    return;
}
size_t PortStream::read(uint8_t* buffer, size_t buffer_size)
{
    if (!mHasRead)
    {
        if (mIn.read(mPacketRead) != RTT::NewData)
            return 0;
    }

    if (buffer_size < mPacketRead.data.size())
        throw std::runtime_error("in PortStream::read(): buffer too small, it should be sized bigger than the biggest packet that could be received on the port");

    mHasRead = false;
    memcpy(&buffer[0], &mPacketRead.data[0], mPacketRead.data.size());
    return mPacketRead.data.size();
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

