#include "ConfigureGuard.hpp"
#include <rtt/extras/FileDescriptorActivity.hpp>

using namespace iodrivers_base;

ConfigureGuard::ConfigureGuard(iodrivers_base::Task *task)
    : m_task(task)
    , m_committed(false)
{
}

ConfigureGuard::~ConfigureGuard()
{
    if (!m_committed)
    {
        RTT::extras::FileDescriptorActivity* fd_activity =
            m_task->getActivity<RTT::extras::FileDescriptorActivity>();
        if (fd_activity)
        {
            fd_activity->clearAllWatches();
            fd_activity->setTimeout(0);
        }

        if (m_task->getDriver())
        {
            iodrivers_base::Driver *driver = m_task->getDriver();

            m_task->detachDriver();
            driver->close();
        }
    }
}

void ConfigureGuard::commit()
{
    m_committed = true;
}

