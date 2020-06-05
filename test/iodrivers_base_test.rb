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

    describe "the behavior on non FD-driven activities" do
        before do
            @subject_task = syskit_deploy(
                OroGen.iodrivers_base.test.PeriodicTask.deployed_as("task_under_test")
            )
        end

        describe "with a file descriptor" do
            before do
                @subject_task.properties.io_read_timeout = Time.at(0.1)
                @local_socket = setup_iodrivers_base_with_fd(@subject_task)
                syskit_configure_and_start(@subject_task)
            end

            it "does nothing if there is no input" do
                sleep 1
                execute_one_cycle # will raise if the task stops
                assert subject_task.running?
            end

            it "does call processIO if there is data" do
                sample = expect_execution { @local_socket.write("\x1\x2\x3\x4") }
                         .to { have_one_new_sample subject_task.rx_port }

                assert_equal "\x1\x2\x3\x4", raw_packet_to_s(sample)
            end
        end

        describe "with the port-based communication" do
            before do
                @subject_task.properties.io_read_timeout = Time.at(0.1)
                @raw_io = setup_iodrivers_base_with_ports(@subject_task)
                syskit_configure_and_start(@subject_task)
            end

            it "does nothing if there is no input" do
                sleep 1
                execute_one_cycle # will raise if the task stops
                assert subject_task.running?
            end

            it "does call processIO if there is data" do
                packet = raw_packet_from_s("\x1\x2\x3\x4")
                sample = expect_execution { syskit_write @raw_io.out_port, packet }
                         .to { have_one_new_sample subject_task.rx_port }

                assert_equal "\x1\x2\x3\x4", raw_packet_to_s(sample)
            end
        end
    end
end
