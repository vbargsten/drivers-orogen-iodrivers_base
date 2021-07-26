# frozen_string_literal: true

using_task_library "iodrivers_base"

require_relative "./orogen_test_helpers"

describe OroGen.iodrivers_base.Task do
    include IODriversBase::OroGenTestHelpers

    attr_reader :subject_task

    run_live

    it "fails to configure if neither the io_port property is set "\
       "nor the io_raw_* ports are connected" do
        @subject_task = syskit_deploy(
            OroGen.iodrivers_base.test.PeriodicTask.deployed_as("task_under_test")
        )

        expect_execution
            .scheduler(true)
            .to { fail_to_start subject_task }
    end

    describe "the behavior of FD-driven activities" do
        attr_reader :subject_task

        before do
            @subject_task = syskit_deploy(
                OroGen.iodrivers_base.test.FDTask.deployed_as("task_under_test")
            )

            @subject_task.properties.io_read_timeout = Time.at(0.1)
            @local_socket = setup_iodrivers_base_with_fd(@subject_task)
        end

        it "does nothing if there is no input" do
            syskit_configure_and_start(@subject_task)
            sleep 1
            execute_one_cycle # will raise if the task stops
            assert subject_task.running?
        end

        it "times out if io_wait_timeout is configured" do
            @subject_task.properties.io_wait_timeout = Time.at(0.5)
            syskit_configure_and_start(@subject_task)
            expect_execution.to { emit subject_task.io_timeout_event }
        end

        it "does call processIO if there is data" do
            syskit_configure_and_start(@subject_task)
            sample = expect_execution { @local_socket.write("\x1\x2\x3\x4") }
                     .to { have_one_new_sample subject_task.rx_port }

            assert_equal "\x1\x2\x3\x4", raw_packet_to_s(sample)
        end

        it "calls processIO repeatedly until all data has been read" do
            syskit_configure_and_start(@subject_task)
            buffers = (0...24).to_a.each_slice(2).map { |b| b.pack("c*") }
            samples = expect_execution { buffers.each { |b| @local_socket.write(b) } }
                      .to { have_new_samples(subject_task.rx_port, 6) }

            expected = (0...24).each_slice(4).map { |array| array.pack("c*") }
            assert_equal(expected, samples.map { |s| raw_packet_to_s(s) })

            # Make sure the UDP packets were processed within less than two periods
            assert (samples[-1].time - samples[0].time) < 0.15
        end
    end

    describe "the behavior on non FD-driven activities" do
        before do
            @subject_task = syskit_deploy(
                OroGen.iodrivers_base.test.PeriodicTask
                      .deployed_as("#{Process.pid}-#{name}")
            )
        end

        describe "with a file descriptor" do
            before do
                @subject_task.properties.io_read_timeout = Time.at(0.1)
                @local_socket = setup_iodrivers_base_with_fd(@subject_task)
            end

            it "does nothing if there is no input" do
                syskit_configure_and_start(@subject_task)
                sleep 1
                execute_one_cycle # will raise if the task stops
                assert subject_task.running?
            end

            it "times out if io_wait_timeout is configured" do
                @subject_task.properties.io_wait_timeout = Time.at(0.5)
                syskit_configure_and_start(@subject_task)
                expect_execution.to { emit subject_task.io_timeout_event }
            end

            it "does call processIO if there is data" do
                syskit_configure_and_start(@subject_task)
                sample = expect_execution { @local_socket.write("\x1\x2\x3\x4") }
                         .to { have_one_new_sample subject_task.rx_port }

                assert_equal "\x1\x2\x3\x4", raw_packet_to_s(sample)
            end

            it "calls processIO repeatedly until all data has been read" do
                syskit_configure_and_start(@subject_task)
                buffers = (0...24).to_a.each_slice(2).map { |b| b.pack("c*") }
                samples = expect_execution { buffers.each { |b| @local_socket.write(b) } }
                          .to { have_new_samples(subject_task.rx_port, 6) }

                expected = (0...24).each_slice(4).map { |array| array.pack("c*") }
                assert_equal(expected, samples.map { |s| raw_packet_to_s(s) })

                # Make sure the UDP packets were processed within less than two periods
                assert (samples[-1].time - samples[0].time) < 0.15
            end
        end

        describe "with the port-based communication" do
            before do
                @subject_task.properties.io_read_timeout = Time.at(0.1)
            end

            it "does nothing if there is no input" do
                setup_iodrivers_base_with_ports(@subject_task)
                sleep 1
                execute_one_cycle # will raise if the task stops
                assert subject_task.running?
            end

            it "times out if io_wait_timeout is configured" do
                @subject_task.properties.io_wait_timeout = Time.at(0.5)
                setup_iodrivers_base_with_ports(@subject_task)
                syskit_configure_and_start(@subject_task)
                expect_execution.to { emit subject_task.io_timeout_event }
            end

            it "does call processIO if there is data" do
                raw_io = setup_iodrivers_base_with_ports(@subject_task)
                packet = raw_packet_from_s("\x1\x2\x3\x4")
                sample = expect_execution { syskit_write raw_io.out_port, packet }
                         .to { have_one_new_sample subject_task.rx_port }

                assert_equal "\x1\x2\x3\x4", raw_packet_to_s(sample)
            end

            it "calls processIO repeatedly until all data has been read" do
                raw_io = setup_iodrivers_base_with_ports(@subject_task)
                packets = (0...24).to_a.each_slice(3)
                                  .map { |bytes| raw_packet_from_s(bytes.pack("c*")) }
                samples = expect_execution { syskit_write raw_io.out_port, *packets }
                          .to { have_new_samples(subject_task.rx_port, 6) }

                expected = (0...24).each_slice(4).map { |array| array.pack("c*") }
                assert_equal(expected, samples.map { |s| raw_packet_to_s(s) })

                # Make sure the incoming samples were processed within less
                # than two periods
                assert (samples[-1].time - samples[0].time) < 0.15
            end
        end
    end
end
