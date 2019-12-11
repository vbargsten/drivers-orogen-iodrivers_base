Generic oroGen integration for drivers based on [iodrivers_base](https://github.com/rock-core/drivers-iodrivers_base)

Usage
-----

Your component must depend on the `drivers/orogen/iodrivers_base` package, and the driver component must subclass `iodrivers_base::Task`:

In the package's `manifest.xml`, add

~~~ xml
<depend name="drivers/orogen/iodrivers_base" />
~~~

At the top of the oroGen file, add:

~~~ ruby
using_task_library "iodrivers_base"
~~~

Then, make your task a subclass of the iodrivers_base::Task task context:

~~~ ruby
task_context "Task", subclasses: "iodrivers_base::Task" do
end
~~~

There is two things left to do:

 * __configureHook__: create the device driver, open the device and call the
   setDriver() method. The device's "name" (i.e. device file, IP for network-based access, ...) is
   provided in the io_port property. Note that it is legal for this property to
   be empty (see the next section for explanations). To ensure exception-safety of
   the configureHook, one must use iodrivers_base's ConfigureGuard guard class (example below).

~~~ cpp
setDriver(driver)
~~~

 * process the __incoming__ data in a "void processIO()" method that is going to be
   called by the base class from updateHook().

 * __cleanupHook__: the device's close() method is going to be called
   automatically by the base class. The pointer is *not* going to be deleted
   though, so you should take care of it if you want to recreate the object in
   cleanup/configure cycles.

For instance, assuming the class has a `m_driver` attribute of the Driver
subclass

~~~ cpp
std::unique_ptr<Driver> m_driver;
~~~

One could have a setup looking like:

~~~ cpp
bool Task::configureHook()
{
    unique_ptr<Driver> driver(new Driver());
    // Un-configure the device driver if the configure fails.
    // You MUST call guard.commit() once the driver is fully
    // functional (usually before the configureHook's "return true;"
    iodrivers_base::ConfigureGuard guard(this);
    if (!_io_port.get().empty())
        driver->openURI(_io_port.get());
    setDriver(driver.get());

    // This is MANDATORY and MUST be called after the setDriver but before you do
    // anything with the driver
    if (!TaskBase::configureHook())
        return false;

    // If some device configuration was needed, it must be done after the
    // setDriver and call to configureHook on TaskBase (i.e., here)

    m_driver = move(driver);
    guard.commit();
    return true;
}
void Task::processIO()
{
    mDriver->processSamples();
    _samples.write(mDriver->getOrientationSample());
}
void Task::cleanupHook()
{
    // MUST BE done first. It detaches the driver from the task
    TaskBase::cleanupHook();
    // Not strictly necessary, the driver will be deleted on the next
    // successful configureHook anyways. YMMV.
    m_driver.reset();
}
~~~

In addition, you might want to start data acquisition in the startHook and stop
it in the stopHook. Whether the acquisition start/stop should be in
startHook/stopHook or configureHook/cleanupHook is governed by the following
factor:

 * if starting/stopping acquisition is done a lot quicker than the whole device
   configuration, then do it in startHook/stopHook as you will not waste
   ressources doing acquisition while the data is not needed
 * if it is slow (some people would even say: not deterministically fast, but
   YMMV), do it in the configureHook/cleanupHook to ensure the responsiveness of
   the system when start/stop cycles are needed but reconfiguration is not needed.

By default, the standard runtime management of oroGen tasks entails that you
will have a full stop/cleanup/configure/start cycle if reconfiguration is
needed. You should therefore not care about dynamic reconfiguration in first
implementations.

Details about the iodrivers_base::Task interface
------------------------------------------------
This interface provides two means of communication between the device and the
driver.

 * direct I/O access. This is done by setting io_port to a non-empty string.
   Acceptable values for io_port if the driver uses openURI (which it should do)
   are listed in the property's documentation.
 * port-based access. In this mode, the data is flowing through the io_raw_in
   and io_raw_out ports. The transfer of data between the ports and the Driver
   object is made by the iodrivers_base::Task class.

Other properties control the behaviour of the system in both modes (read
timeout) and write statistics about the I/O. Some properties are specific to one
mode, in which case this is documented in the property documentation directly.
