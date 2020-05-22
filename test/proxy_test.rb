# frozen_string_literal: true

using_task_library "iodrivers_base"

describe OroGen.iodrivers_base.Proxy do
    run_live

    before do
        @task = syskit_deploy(
            OroGen.iodrivers_base.Proxy
                  .deployed_as("task_under_test")
        )
    end

    describe "file-descriptor based I/O" do
        before do
            # We currently have no simple way to forward a file descriptor to
            # the component and have it use it. Randomly assign a port locally,
            # close the socket and ask the component to use it to reduce the
            # likelihood of port collision
            @local_socket = UDPSocket.new
            @local_socket.bind("localhost", 0)
            local_port = @local_socket.local_address.ip_port
            component_socket = UDPSocket.new
            component_socket.bind("localhost", 0)
            component_port = component_socket.local_address.ip_port
            component_socket.close
            @local_socket.connect "localhost", component_port

            @task.properties.io_port = "udp://localhost:#{local_port}:#{component_port}"
            syskit_configure_and_start(@task)
        end

        after do
            @local_socket&.close
        end

        it "forwards the data received on the FD to rx" do
            expect_receives "\x1\x2\x3\x4" do
                @local_socket.write "\x1\x2\x3\x4"
            end
        end
        it "forwards the data received on tx to the FD" do
            syskit_write @task.tx_port, make_packet("\x1\x2\x3\x4")
            assert_equal "\x1\x2\x3\x4", @local_socket.read(4)
        end
    end

    describe "using the io_raw_in and io_raw_out instead of the file descriptor" do
        before do
            raw_io_m = Syskit::DataService.new_submodel do
                input_port "in", "/iodrivers_base/RawPacket"
                output_port "out", "/iodrivers_base/RawPacket"
            end

            port_io_m = Syskit::Composition.new_submodel do
                add OroGen.iodrivers_base.Proxy, as: "test"
                add raw_io_m, as: "raw_io"

                test_child.io_raw_out_port.connect_to \
                    raw_io_child, type: :buffer, size: 20
                raw_io_child.connect_to \
                    test_child.io_raw_in_port, type: :buffer, size: 20
            end

            port_io = syskit_stub_deploy_configure_and_start(
                port_io_m.use("test" => @task)
            )

            syskit_configure_and_start(port_io)
        end

        it "forwards the data received on io_raw_in to rx" do
            expect_receives "\x1\x2\x3\x4" do
                syskit_write @task.io_raw_in_port, make_packet("\x1\x2\x3\x4")
            end
        end
        it "forwards the data received on tx to io_raw_out" do
            expect_receives "\x1\x2\x3\x4", port: @task.io_raw_out_port do
                syskit_write @task.tx_port, make_packet("\x1\x2\x3\x4")
            end
        end
    end

    def expect_receives(expected_data, port: @task.rx_port, &block)
        buffer = "".dup
        expect_execution(&block)
            .to do
                have_one_new_sample(port)
                    .matching do |s|
                        buffer.concat(s.data.to_byte_array[8..-1])
                        buffer.size == expected_data.size
                    end
            end

        assert_equal expected_data, buffer
    end

    def make_packet(data)
        packet = Types.iodrivers_base.RawPacket.new
        packet.time = Time.now
        packet.data.from_buffer([data.size].pack("Q<") + data)
        packet
    end
end
