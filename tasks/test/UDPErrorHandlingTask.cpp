/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "UDPErrorHandlingTask.hpp"
#include <iodrivers_base/Driver.hpp>

using namespace iodrivers_base::test;

UDPErrorHandlingTask::UDPErrorHandlingTask(std::string const& name)
    : UDPErrorHandlingTaskBase(name)
{
}

UDPErrorHandlingTask::~UDPErrorHandlingTask()
{
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See UDPErrorHandlingTask.hpp for more detailed
// documentation about them.

bool UDPErrorHandlingTask::configureHook()
{
    if (! UDPErrorHandlingTaskBase::configureHook())
        return false;
    return true;
}
bool UDPErrorHandlingTask::startHook()
{
    if (! UDPErrorHandlingTaskBase::startHook())
        return false;
    return true;
}
void UDPErrorHandlingTask::updateHook()
{
    iodrivers_base::RawPacket packet;
    while (_tx.read(packet) == RTT::NewData) {
        mDriver->writePacket(packet.data.data(), packet.data.size());
    }
    UDPErrorHandlingTaskBase::updateHook();
}
void UDPErrorHandlingTask::errorHook()
{
    UDPErrorHandlingTaskBase::errorHook();
}
void UDPErrorHandlingTask::stopHook()
{
    UDPErrorHandlingTaskBase::stopHook();
}
void UDPErrorHandlingTask::cleanupHook()
{
    UDPErrorHandlingTaskBase::cleanupHook();
}
