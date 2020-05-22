/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "PeriodicTask.hpp"
#include "../ConfigureGuard.hpp"

using namespace iodrivers_base;
using namespace iodrivers_base::test;
using namespace std;

static const int DUMMY_BUFFER_SIZE = 1024;

namespace
{
    class DummyDriver : public iodrivers_base::Driver {
    public:
        DummyDriver()
            : iodrivers_base::Driver(DUMMY_BUFFER_SIZE) {}

        int extractPacket(boost::uint8_t const* buffer, size_t size) const
        {
            return size;
        }
    };
}

PeriodicTask::PeriodicTask(std::string const& name)
    : PeriodicTaskBase(name)
{
}

PeriodicTask::~PeriodicTask()
{
}

bool PeriodicTask::configureHook()
{
    unique_ptr<Driver> driver(new DummyDriver());
    // Un-configure the device driver if the configure fails.
    // You MUST call guard.commit() once the driver is fully
    // functional (usually before the configureHook's "return true;"
    iodrivers_base::ConfigureGuard guard(this);
    if (!_io_port.get().empty())
        driver->openURI(_io_port.get());
    setDriver(driver.get());

    // This is MANDATORY and MUST be called after the setDriver but before you do
    // anything with the driver
    if (!PeriodicTaskBase::configureHook())
        return false;

    // If some device configuration was needed, it must be done after the
    // setDriver and call to configureHook on TaskBase (i.e., here)

    mDriver = move(driver);
    guard.commit();
    return true;

}
bool PeriodicTask::startHook()
{
    if (! PeriodicTaskBase::startHook())
        return false;
    return true;
}
void PeriodicTask::updateHook()
{
    PeriodicTaskBase::updateHook();
}
void PeriodicTask::processIO()
{
    uint8_t buffer[DUMMY_BUFFER_SIZE];
    int packet_size = mDriver->readPacket(buffer, DUMMY_BUFFER_SIZE);

    RawPacket packet;
    packet.time = base::Time::now();
    packet.data = std::vector<uint8_t>(buffer, buffer + packet_size);
    _rx.write(packet);
}
void PeriodicTask::errorHook()
{
    PeriodicTaskBase::errorHook();
}
void PeriodicTask::stopHook()
{
    PeriodicTaskBase::stopHook();
}
void PeriodicTask::cleanupHook()
{
    PeriodicTaskBase::cleanupHook();
}
