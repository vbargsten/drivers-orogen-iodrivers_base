# frozen_string_literal: true

module IODriversBase
    # Common helpers for the rather complex setup of iodrivers_base::Task tests
    #
    # It relies on the test cases to deploy a task-under-test and store it in the
    # @task instance variable
    module OroGenTestHelpers
        def setup
            super

            @__iodrivers_base_sockets = []
        end

        def teardown
            super

            @__iodrivers_base_sockets.each(&:close)
        end

        # Configure @task to use a file descriptor
        #
        # Internally, it creates a UDP socket and set the io_port property to connect to it
        def setup_iodrivers_base_with_fd(task, io_port_name: :io_port)
            # We currently have no simple way to forward a file descriptor to
            # the component and have it use it. Randomly assign a port locally,
            # close the socket and ask the component to use it to reduce the
            # likelihood of port collision
            local_socket, local_port = create_udp_socket
            remote_socket, remote_port = create_udp_socket

            @__iodrivers_base_sockets << local_socket
            remote_socket.close

            local_socket.connect "127.0.0.1", remote_port
            task.properties[io_port_name]
                .write("udp://127.0.0.1:#{local_port}?local_port=#{remote_port}")

            local_socket
        end

        # @api private
        #
        # Data service used by {#setup_iodrivers_base_with_ports}
        data_service_type "RawIOService" do
            input_port "in", "/iodrivers_base/RawPacket"
            output_port "out", "/iodrivers_base/RawPacket"
        end

        # @api private
        #
        # Composition used by {#setup_iodrivers_base_with_ports}
        class PortIO < Syskit::Composition
            add OroGen.iodrivers_base.Task, as: "test"
            add RawIOService, as: "raw_io"

            test_child.io_raw_out_port.connect_to \
                raw_io_child, type: :buffer, size: 20
            raw_io_child.connect_to \
                test_child.io_raw_in_port, type: :buffer, size: 20
        end

        # Configure task to be connected to another task for its I/O, and
        # return the task
        #
        # The "peering" task has a 'in' port and 'out' port. The direction is
        # relative to the task, so write to `out_port` to send data to the driver,
        # and read from `in_port` to read data from it.
        def setup_iodrivers_base_with_ports(task)
            port_io = syskit_stub_deploy_configure_and_start(
                PortIO.use("test" => task)
            )
            syskit_configure_and_start(port_io, recursive: false)
            syskit_configure_and_start(port_io.raw_io_child, recursive: false)
            port_io.raw_io_child
        end

        # Create a UDP socket on a random port and return the socket and the port
        #
        # @return [UDPSocket, Integer]
        def create_udp_socket
            socket = UDPSocket.new
            socket.bind("127.0.0.1", 0)
            [socket, socket.local_address.ip_port]
        end

        # Create a RawPacket object from a raw data as a string
        #
        # @param [String] data the data
        def raw_packet_from_s(data, time: Time.now)
            packet = Types.iodrivers_base.RawPacket.new
            packet.time = time
            packet.data.from_buffer([data.size].pack("Q<") + data)
            packet
        end

        # Convert a RawPacket's data array into a string
        #
        # @param [Types.iodrivers_base.RawPacket] packet
        # @return [String]
        def raw_packet_to_s(packet)
            packet.data.to_byte_array[8..-1]
        end

        # Read data from a I/O but automatically timing out
        def read_with_timeout(io, size, timeout: 10)
            deadline = Time.now + timeout
            buffer = "".dup

            while buffer.size != size
                begin
                    buffer.concat(io.read_nonblock(size - buffer.size))
                rescue IO::WaitReadable
                    remaining = deadline - Time.now
                    if remaining < 0
                        flunk("failed to read #{size} bytes from #{io} in #{timeout}s. "\
                              "Only got #{buffer.size} so far")
                    end

                    select([io], [], [], remaining)
                end
            end

            buffer
        end
    end
end
