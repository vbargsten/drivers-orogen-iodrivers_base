/* Generated from orogen/lib/orogen/templates/tasks/Task.hpp */

#ifndef IODRIVERS_BASE_TASK_TASK_HPP
#define IODRIVERS_BASE_TASK_TASK_HPP

#include "iodrivers_base/TaskBase.hpp"

namespace iodrivers_base {
    class Driver;

    class Task : public TaskBase
    {
	friend class TaskBase;

    protected:
        Driver* mDriver;
        RawPacket mRawPacket;
        base::Time mLastStatus;

        /** Pushes all output data currently queued in the Driver class to the
         * _out_raw port if it is connected
         *
         * This method MUST be called at the end of the updateHook in the
         * subclasses. A check is implemented to warn if the subclass did not do
         * this. It will output warnings looking like:
         *
         *   unwritten data is present in the driver's output buffer. Did you forget to call pushAllData() at the end of the updateHook() ?
         */
        void pushAllData();

    public:
        Task(std::string const& name = "iodrivers_base::Task");
        Task(std::string const& name, RTT::ExecutionEngine* engine);
	~Task();

        /** This hook is called by Orocos when the state machine transitions
         * from PreOperational to Stopped. If it returns false, then the
         * component will stay in PreOperational. Otherwise, it goes into
         * Stopped.
         *
         * It is meaningful only if the #needs_configuration has been specified
         * in the task context definition with (for example):
         *
         *   task_context "TaskName" do
         *     needs_configuration
         *     ...
         *   end
         */
        // bool configureHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Stopped to Running. If it returns false, then the component will
         * stay in Stopped. Otherwise, it goes into Running and updateHook()
         * will be called.
         */
        bool startHook();

        /** This hook is called by Orocos when the component is in the Running
         * state, at each activity step. Here, the activity gives the "ticks"
         * when the hook should be called.
         *
         * The error(), exception() and fatal() calls, when called in this hook,
         * allow to get into the associated RunTimeError, Exception and
         * FatalError states. 
         *
         * In the first case, updateHook() is still called, and recover() allows
         * you to go back into the Running state.  In the second case, the
         * errorHook() will be called instead of updateHook(). In Exception, the
         * component is stopped and recover() needs to be called before starting
         * it again. Finally, FatalError cannot be recovered.
         */
        void updateHook();

        /** This hook is called by Orocos when the component is in the
         * RunTimeError state, at each activity step. See the discussion in
         * updateHook() about triggering options.
         *
         * Call recover() to go back in the Runtime state.
         */
        // void errorHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Running to Stopped after stop() has been called.
         */
        void stopHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Stopped to PreOperational, requiring the call to configureHook()
         * before calling start() again.
         */
        void cleanupHook();
    };
}

#endif

