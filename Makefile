objects = pipe1 socpair ns_server ns_client list_adapters http_time_server server_udp client_udp server client
all: $(objects)
CPP=g++
CPPFLAGS=-g --std=c++17

$(objects): %: %.cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

clean:
	rm -f $(objects)
