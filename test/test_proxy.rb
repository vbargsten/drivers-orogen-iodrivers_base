require 'minitest/spec'
require 'orocos/test/component'
require 'minitest/autorun'

describe 'iodrivers_base::Proxy' do
    include Orocos::Test::Component

    describe "file-descriptor base I/O" do
        start 'proxy', 'iodrivers_base::Proxy' => 'proxy'
        writer 'proxy', 'tx', attr_name: 'tx'
        reader 'proxy', 'rx', attr_name: 'rx'
    
        before do
            # We currently have no simple way to forward a file descriptor to
            # the component and have it use it. Randomly assign a port locally,
            # close the socket and ask the component to use it to reduce the
            # likelihood of port collision
            @local_socket = UDPSocket.new
            @local_socket.bind('localhost', 0)
            local_port = @local_socket.local_address.ip_port
            component_socket = UDPSocket.new
            component_socket.bind('localhost', 0)
            component_port = component_socket.local_address.ip_port
            component_socket.close
            @local_socket.connect 'localhost', component_port

            proxy.io_port = "udp://localhost:#{local_port}:#{component_port}"
            proxy.configure
            proxy.start
        end

        it "forwards the data received on io_raw_in to rx" do
            @local_socket.write "\x1\x2\x3\x4"
            assert_receives "\x1\x2\x3\x4", rx
        end
        it "forwards the data received on tx to io_raw_out" do
            tx.write make_packet("\x1\x2\x3\x4")
            assert_equal "\x1\x2\x3\x4", @local_socket.read(4)
        end
    end

    describe "using the io_raw_in and io_raw_out instead of the file descriptor" do
        start 'proxy', 'iodrivers_base::Proxy' => 'proxy'
        writer 'proxy', 'tx', attr_name: 'tx'
        reader 'proxy', 'rx', attr_name: 'rx'
    
        before do
            @io_raw_in  = data_writer proxy.io_raw_in
            @io_raw_out = data_reader proxy.io_raw_out
            proxy.configure
            proxy.start
        end

        it "forwards the data received on io_raw_in to rx" do
            @io_raw_in.write(make_packet("\x1\x2\x3\x4"))
            assert_receives "\x1\x2\x3\x4", rx
        end
        it "forwards the data received on tx to io_raw_out" do
            tx.write make_packet("\x1\x2\x3\x4")
            assert_receives "\x1\x2\x3\x4", @io_raw_out
        end
    end

    def assert_receives(expected_data, reader)
        data = ''
        while data.size < expected_data.size
            new_packet = assert_has_one_new_sample reader
            data.concat(new_packet.data.to_byte_array[8..-1])
        end
        assert_equal expected_data, data[0, expected_data.size]
    end

    def make_packet(data)
        packet = tx.new_sample
        packet.time = Time.now
        packet.data.from_buffer([data.size].pack("Q<") + data)
        packet
    end
end

