debug_flags=-Wall -Wenum-compare -Wenum-conversion -Wpedantic -ggdb -O0

all: debug

unit-test: CXXFLAGS += $(debug_flags)
unit-test: ipk24-chat-tests.out
	./ipk24-chat-tests.out

test: test/system/test_connection.py
	./test/system/test_connection.py ./ipk24-chat.out

gdb: all
	gdb ipk24-chat.out

CXXFLAGS += -std=c++20

debug: CXXFLAGS += $(debug_flags)
debug: ipk24-chat.out

ipk24-chat.out: main.o client.o session.o exception.o message_factory.o message.o\
								udp_receiver.o udp_sender.o tcp_sender.o tcp_receiver.o tcp_message_factory.o\
								tcp_session.o udp_session.o
	$(CXX) $(CXXFLAGS) $^ -o $@

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

client.o: client.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

session.o: session.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

exception.o: exception.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

# server.o: server.cpp
# 	$(CXX) $(CXXFLAGS) -c $^ -o $@

message.o: message.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

message_factory.o: message_factory.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

ipk24-chat-tests.out: test/test_main.o test/message_serialize_tests.o test/tcp_message_factory_tests.o\
											message_factory.o message.o test/message_factory_tests.o tcp_message_factory.o
	$(CXX) $(CXXFLAGS) -lgtest $^ -o $@

test/test_main.o: test/test_main.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

test/message_serialize_tests.o: test/message_serialize_tests.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

test/message_factory_tests.o: test/message_factory_tests.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

test/tcp_message_factory_tests.o: test/tcp_message_factory_tests.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

udp_session.o: udp_session.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

tcp_session.o: tcp_session.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

udp_receiver.o: udp_receiver.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

udp_sender.o: udp_sender.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

tcp_receiver.o: tcp_receiver.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

tcp_sender.o: tcp_sender.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

tcp_message_factory.o: tcp_message_factory.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

clean:
	$(RM) *.o *.out test/*.o

.PHONY: test clean
