debug_flags=-Wall -Wenum-compare -Wenum-conversion -Wpedantic -ggdb -O0

test: CXXFLAGS += $(debug_flags)
test: ipk24-chat-tests.out

all: debug

gdb: all
	gdb ipk24-chat.out

CXXFLAGS += -std=c++20

debug: CXXFLAGS += $(debug_flags)
debug: ipk24-chat.out

ipk24-chat.out: main.o client.o session.o exception.o server.o message_factory.o message.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

client.o: client.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

session.o: session.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

exception.o: exception.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

server.o: server.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

message.o: message.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

message_factory.o: message_factory.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

ipk24-chat-tests.out: test/test_main.o test/message_serialize_tests.o message_factory.o message.o 
	$(CXX) $(CXXFLAGS) -lgtest $^ -o $@

test/test_main.o: test/test_main.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

test/message_serialize_tests.o: test/message_serialize_tests.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

clean:
	$(RM) *.o *.out
