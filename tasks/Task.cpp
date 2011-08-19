/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"
#include <iodrivers_base/Driver.hpp>
#include <rtt/extras/FileDescriptorActivity.hpp>

using namespace iodrivers_base;

Task::Task(std::string const& name)
    : TaskBase(name)
{
}

Task::Task(std::string const& name, RTT::ExecutionEngine* engine)
    : TaskBase(name, engine)
{
    _io_write_timeout.write(base::Time::fromSeconds(1));
    _io_read_timeout.write(base::Time::fromSeconds(1));
    _io_status_interval.write(base::Time::fromSeconds(1));
}

Task::~Task()
{
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Task.hpp for more detailed
// documentation about them.

// bool Task::configureHook()
// {
//     if (! TaskBase::configureHook())
//         return false;
//     return true;
// }

bool Task::startHook()
{
    if (!mDriver)
    {
        log(RTT::Error) << "call setDriver(driver) before TaskBase::startHook()" << RTT::endlog();
        return false;
    }

    if (! TaskBase::startHook())
        return false;

    if (mDriver->isValid())
    {
        RTT::extras::FileDescriptorActivity* fd_activity =
            getActivity<RTT::extras::FileDescriptorActivity>();
        if (fd_activity)
        {
            fd_activity->watch(mDriver->getFileDescriptor());
            fd_activity->setTimeout(_io_read_timeout.get().toMilliseconds() / 1000);
        }
        mDriver->setReadTimeout(_io_read_timeout.get().toMilliseconds());
        mDriver->setWriteTimeout(_io_write_timeout.get().toMilliseconds());
    }
    return true;
}

void Task::updateHook()
{
    mDriver->setOutBufferEnabled(_out_raw.connected() || !mDriver->isValid());

    if (mDriver->isOutBufferEnabled() && mDriver->getOutBufferSize())
    {
        // Check if there is some data in the driver's output buffer. If it is
        // the case, then the user most likely forgot to call pushAllData() at
        // the end of his updateHook()
        log(RTT::Warn) << "unwritten data is present in the driver's output buffer. Did you forget to call pushAllData() at the end of the updateHook() ?" << RTT::endlog();
        pushAllData();
    }

    TaskBase::updateHook();

    if (!mDriver->isValid())
    {
        while (_raw_in.read(mRawPacket, false) == RTT::NewData)
            mDriver->pushRawData(mRawPacket);
    }
}

void Task::pushAllData()
{
    if (_raw_out.connected())
    {
        mRawPacket.time = base::Time::now();
        mDriver->flushOutBuffer(mRawPacket.data);
        _out_raw.write(mRawPacket);
    }

    base::Time now = base::Time::now();
    if ((now - mLastStatus) > _io_status_interval.get())
    {
        _io_status.write(mDriver->getStatus());
        mLastStatus = now;
    }
}

// void Task::errorHook()
// {
//     TaskBase::errorHook();
// }

void Task::stopHook()
{
    RTT::extras::FileDescriptorActivity* fd_activity =
        getActivity<RTT::extras::FileDescriptorActivity>();
    if (fd_activity)
    {
        fd_activity->clearAllWatches();
        //set timeout back so we don't timeout on the rtt's pipe
        fd_activity->setTimeout(0);
    }
    TaskBase::stopHook();
}

void Task::cleanupHook()
{
    TaskBase::cleanupHook();
    mDriver->close();
}

