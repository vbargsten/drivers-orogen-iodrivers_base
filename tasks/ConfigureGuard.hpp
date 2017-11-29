#ifndef IODRIVERS_BASE_OROGEN_CONFIGUREGUARD_HPP
#define IODRIVERS_BASE_OROGEN_CONFIGUREGUARD_HPP

#include <iodrivers_base/Task.hpp>
#include <iodrivers_base/Driver.hpp>

namespace iodrivers_base {

class ConfigureGuard
{
private:
    iodrivers_base::Task *m_task;
    bool m_committed;

public:
    explicit ConfigureGuard(iodrivers_base::Task *task);
    ~ConfigureGuard();
    void commit();
};

}

#endif
