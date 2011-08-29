/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"
#include <iodrivers_base/Driver.hpp>
#include <rtt/extras/FileDescriptorActivity.hpp>

using namespace iodrivers_base;

Task::Task(std::string const& name)
    : TaskBase(name)
    , mHasIO(false)
{
    _io_write_timeout.set(base::Time::fromSeconds(1));
    _io_read_timeout.set(base::Time::fromSeconds(1));
    _io_status_interval.set(base::Time::fromSeconds(1));
}

Task::Task(std::string const& name, RTT::ExecutionEngine* engine)
    : TaskBase(name, engine)
{
    _io_write_timeout.set(base::Time::fromSeconds(1));
    _io_read_timeout.set(base::Time::fromSeconds(1));
    _io_status_interval.set(base::Time::fromSeconds(1));
}

Task::~Task()
{
}

void Task::setDriver(Driver* driver)
{
    mDriver = driver;
}

bool Task::hasIO() const
{
    return mHasIO;
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
    mHasIO = false;
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
            fd_activity->setTimeout(_io_read_timeout.get().toMilliseconds());
        }
        mDriver->setReadTimeout(_io_read_timeout.get());
        mDriver->setWriteTimeout(_io_write_timeout.get());
    }
    return true;
}

void Task::updateHook()
{
    mHasIO = false;
    mDriver->setOutputBufferEnabled(_io_raw_out.connected() || !mDriver->isValid());

    if (mDriver->isOutputBufferEnabled() && mDriver->getOutputBufferSize())
    {
        // Check if there is some data in the driver's output buffer. If it is
        // the case, then the user most likely forgot to call pushAllData() at
        // the end of his updateHook()
        log(RTT::Warning) << "unwritten data is present in the driver's output buffer. Did you forget to call pushAllData() at the end of the updateHook() ?" << RTT::endlog();
        pushAllData();
    }

    TaskBase::updateHook();

    if (mDriver->isValid())
    {
        RTT::extras::FileDescriptorActivity* fd_activity =
            getActivity<RTT::extras::FileDescriptorActivity>();
        if (fd_activity)
        {
            mHasIO = fd_activity->isUpdated(mDriver->getFileDescriptor());
            if (fd_activity->hasError())
                return exception(IO_ERROR);
        }
        else
        {
            std::cout << "activity: not a FDActivity" << std::endl;
            mHasIO = true;
        }
    }
    else
    {
        while (_io_raw_in.read(mRawPacket, false) == RTT::NewData)
        {
            std::cout << "got raw input" << std::endl;
            mHasIO = true;
            mDriver->pushInputRaw(mRawPacket.data);
        }
    }

    if (mHasIO)
        processIO();
}

void Task::processIO()
{
}

void Task::pushAllData()
{
    if (_io_raw_out.connected())
    {
        mRawPacket.time = base::Time::now();
        mDriver->pullOutputRaw(mRawPacket.data);
        _io_raw_out.write(mRawPacket);
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

