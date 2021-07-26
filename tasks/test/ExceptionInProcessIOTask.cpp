/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "ExceptionInProcessIOTask.hpp"

using namespace iodrivers_base::test;

ExceptionInProcessIOTask::ExceptionInProcessIOTask(std::string const& name)
    : ExceptionInProcessIOTaskBase(name)
{
}

ExceptionInProcessIOTask::~ExceptionInProcessIOTask()
{
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See ExceptionInProcessIOTask.hpp for more detailed
// documentation about them.

bool ExceptionInProcessIOTask::configureHook()
{
    if (! ExceptionInProcessIOTaskBase::configureHook())
        return false;
    return true;
}
bool ExceptionInProcessIOTask::startHook()
{
    if (! ExceptionInProcessIOTaskBase::startHook())
        return false;
    return true;
}
void ExceptionInProcessIOTask::updateHook()
{
    ExceptionInProcessIOTaskBase::updateHook();
}
void ExceptionInProcessIOTask::processIO()
{
    return exception(EXCEPTION);
}
void ExceptionInProcessIOTask::errorHook()
{
    ExceptionInProcessIOTaskBase::errorHook();
}
void ExceptionInProcessIOTask::stopHook()
{
    ExceptionInProcessIOTaskBase::stopHook();
}
void ExceptionInProcessIOTask::cleanupHook()
{
    ExceptionInProcessIOTaskBase::cleanupHook();
}
