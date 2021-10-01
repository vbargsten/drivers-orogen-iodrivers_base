/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"
#include <iodrivers_base/Driver.hpp>
#include <rtt/extras/FileDescriptorActivity.hpp>
#include "PortStream.hpp"

#include <base-logging/Logging.hpp>

using namespace iodrivers_base;


Task::Task(std::string const& name)
    : TaskBase(name)
    , mDriver(0), mStream(0), mListener(0)
{
    _io_write_timeout.set(base::Time::fromSeconds(1));
    _io_read_timeout.set(base::Time::fromSeconds(1));
    _io_status_interval.set(base::Time::fromSeconds(1));
}

Task::Task(std::string const& name, RTT::ExecutionEngine* engine)
    : TaskBase(name, engine)
    , mDriver(0), mStream(0), mListener(0)
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
    if (driver == mDriver)
        return;

    if (driver)
    {
        mListener = new PortListener(_io_read_listener, _io_write_listener);
        driver->addListener(mListener);
    }

    mDriver = driver;
    mStream = 0;
}

void Task::detachDriver()
{
    if (mDriver)
    {
        mDriver->removeListener(mListener);
        mDriver = 0;
        delete mListener;
        mListener = 0;
    }
}

Driver* Task::getDriver() const
{
    return mDriver;
}

/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Task.hpp for more detailed
// documentation about them.

bool Task::configureHook()
{
    if (!_io_port.get().empty() && _io_raw_in.connected())
        throw std::runtime_error("cannot use the io_raw_in port and a normal I/O mechanism at the same time");
    else if (_io_port.get().empty() && !_io_raw_in.connected())
        throw std::runtime_error("either the io_port property must be set to a valid URL or the io_raw_in port must be connected");
    if (!mDriver)
        throw std::runtime_error("call setDriver(driver) before configureHook()");

    if (! TaskBase::configureHook())
        return false;

    mIOWaitTimeout = _io_wait_timeout.get();

    if (mDriver->getFileDescriptor() != Driver::INVALID_FD)
    {
        if (_io_raw_in.connected())
            throw std::runtime_error("cannot use the io_raw_in port and a normal I/O mechanism at the same time");

        RTT::extras::FileDescriptorActivity* fd_activity =
            getActivity<RTT::extras::FileDescriptorActivity>();
        if (fd_activity)
        {
            fd_activity->watch(mDriver->getFileDescriptor());

            if (mIOWaitTimeout.isNull()) {
                fd_activity->setTimeout(_io_read_timeout.get().toMilliseconds());
            }
            else {
                fd_activity->setTimeout(mIOWaitTimeout.toMilliseconds() / 10);
            }
        }
    }
    else if (_io_raw_in.connected())
    {
        mStream = new PortStream(_io_raw_in, _io_raw_out);
        mDriver->setMainStream(mStream);
    }
    mDriver->setReadTimeout(_io_read_timeout.get());
    mDriver->setWriteTimeout(_io_write_timeout.get());

    return true;
}

bool Task::startHook()
{
    if (!_io_port.get().empty() && _io_raw_in.connected())
        throw std::runtime_error("cannot use the io_raw_in port and a normal I/O mechanism at the same time");

    if (! TaskBase::startHook())
        return false;

    mLastStatus = base::Time::now();

    if (mIOWaitTimeout.isNull()) {
        mIOWaitDeadline = base::Time();
    }
    else {
        mIOWaitDeadline = base::Time::now() + mIOWaitTimeout;
    }
    return true;
}

bool Task::hasIO()
{
    if (mDriver->getFileDescriptor() == Driver::INVALID_FD) {
        return mStream->hasQueuedData();
    }

    return mDriver->getMainStream()->hasIO();
}

void Task::updateHook()
{
    TaskBase::updateHook();

    if (mDriver->getFileDescriptor() != Driver::INVALID_FD)
    {
        RTT::extras::FileDescriptorActivity* fd_activity =
            getActivity<RTT::extras::FileDescriptorActivity>();
        if (fd_activity)
        {
            if (fd_activity->hasError())
                return exception(IO_ERROR);
        }
    }

    if ((base::Time::now() - mLastStatus) > _io_status_interval.get())
        updateIOStatus();

    while (hasIO()) {
        do {
            if (!mIOWaitDeadline.isNull()) {
                mIOWaitDeadline = base::Time::now() + mIOWaitTimeout;
            }
            try {
                processIO();
            }
            catch(iodrivers_base::TimeoutError& e) {
                if (!_handle_iodrivers_base_timeout.get()) {
                    throw;
                }

                LOG_ERROR_S
                    << "Received TimeoutError exception from processIO(), "
                    << "transitioning to IO_TIMEOUT: "
                    << e.what();
                exception(IO_TIMEOUT);
                return;
            }

            if (getTaskState() != TaskCore::Running) {
                // processIO transitioned to a stop state
                return;
            }
        }
        while (mDriver->hasPacket());
    }

    if (!mIOWaitDeadline.isNull() && base::Time::now() > mIOWaitDeadline) {
        processIOTimeout();
    }
}

void Task::processIOTimeout()
{
    updateIOStatus();
    exception(IO_TIMEOUT);
}

void Task::updateIOStatus()
{
    mLastStatus = base::Time::now();
    _io_status.write(mDriver->getStatus());
}

void Task::pushAllData()
{
}

// void Task::errorHook()
// {
//     TaskBase::errorHook();
// }

void Task::exceptionHook()
{
    // No guarantee that the driver is initialized or not yet deleted here
    if (mDriver)
        updateIOStatus();
    return TaskBase::exceptionHook();
}

void Task::stopHook()
{
    updateIOStatus();
    TaskBase::stopHook();
}

void Task::cleanupHook()
{
    RTT::extras::FileDescriptorActivity* fd_activity =
        getActivity<RTT::extras::FileDescriptorActivity>();
    if (fd_activity)
    {
        fd_activity->clearAllWatches();
        //set timeout back so we don't timeout on the rtt's pipe
        fd_activity->setTimeout(0);
    }

    if (mDriver) // the subclass could have decided to delete the driver before calling us
        mDriver->close();

    TaskBase::cleanupHook();
    mStream = 0;
}

