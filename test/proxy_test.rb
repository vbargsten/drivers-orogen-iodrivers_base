# frozen_string_literal: true

require_relative "./orogen_test_helpers"

using_task_library "iodrivers_base"

describe OroGen.iodrivers_base.Proxy do
    include IODriversBase::OroGenTestHelpers

    attr_reader :subject_task

    run_live

    before do
        @subject_task = syskit_deploy(
            OroGen.iodrivers_base.Proxy.deployed_as("task_under_test")
        )
    end

    describe "file-descriptor based I/O" do
        before do
            @local_socket = setup_iodrivers_base_with_fd(@subject_task)
            syskit_configure_and_start(@subject_task)
        end

        it "forwards the data received on the FD to rx" do
            sample = expect_execution { @local_socket.write "\x1\x2\x3\x4" }
                     .to { have_one_new_sample subject_task.rx_port }

            assert_equal "\x1\x2\x3\x4", raw_packet_to_s(sample)
        end

        it "forwards the data received on tx to the FD" do
            syskit_write @subject_task.tx_port, raw_packet_from_s("\x1\x2\x3\x4")
            assert_equal "\x1\x2\x3\x4", @local_socket.read(4)
        end
    end

    describe "using the io_raw_in and io_raw_out instead of the file descriptor" do
        attr_reader :raw_io

        before do
            @raw_io = setup_iodrivers_base_with_ports(@subject_task)
            syskit_configure_and_start(@subject_task)
        end

        it "forwards the data received on io_raw_in to rx" do
            packet = raw_packet_from_s("\x1\x2\x3\x4")
            sample = expect_execution { syskit_write @raw_io.out_port, packet }
                     .to { have_one_new_sample subject_task.rx_port }
            assert_equal packet.data, sample.data
        end

        it "forwards the data received on tx to io_raw_out" do
            packet = raw_packet_from_s("\x1\x2\x3\x4")
            sample = expect_execution { syskit_write @subject_task.tx_port, packet }
                     .to { have_one_new_sample raw_io.in_port }
            assert_equal packet.data, sample.data
        end
    end
end
