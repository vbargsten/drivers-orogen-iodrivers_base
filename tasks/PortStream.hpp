#ifndef IODRIVERS_BASE_OROGEN_STREAM_HPP
#define IODRIVERS_BASE_OROGEN_STREAM_HPP

#include <iodrivers_base/IOStream.hpp>
#include <iodrivers_base/IOListener.hpp>
#include <iodrivers_base/RawPacket.hpp>
#include <rtt/InputPort.hpp>
#include <rtt/OutputPort.hpp>

namespace iodrivers_base {

class PortStream : public IOStream
{
    RTT::InputPort<RawPacket>& mIn;
    RTT::OutputPort<RawPacket>& mOut;

    RawPacket mPacketRead;
    RawPacket mPacketWrite;

public:
    PortStream(RTT::InputPort<RawPacket>& in,
            RTT::OutputPort<RawPacket>& out);

    bool hasQueuedData();
    bool waitRead(base::Time const& timeout);
    bool waitWrite(base::Time const& timeout);
    size_t read(uint8_t* buffer, size_t buffer_size);
    size_t write(uint8_t const* buffer, size_t buffer_size);
    void clear();
};

class PortListener : public IOListener
{
    RTT::OutputPort<RawPacket>& mOutRead;
    RTT::OutputPort<RawPacket>& mOutWrite;
    RawPacket mPacket;

public:
    PortListener(RTT::OutputPort<RawPacket>& out_read,
            RTT::OutputPort<RawPacket>& out_write);

    void writeData(boost::uint8_t const* buffer, size_t buffer_size);
    void readData(boost::uint8_t const* buffer, size_t buffer_size);
};

}

#endif
