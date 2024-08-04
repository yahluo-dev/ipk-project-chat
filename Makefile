CXXFLAGS += -std=c++20
debug_flags=-Wall -Wenum-compare -Wenum-conversion -Wpedantic -ggdb -O0
release_flags=-DNDEBUG
EXE=ipk24-chat-client
TESTEXE=ipk24-chat-tests

all: release
zip: xvasil10.zip

unit-test: CXXFLAGS += $(debug_flags)
unit-test: $(TESTEXE)
	./$(TESTEXE)

test: test/system/test_connection.py
	./test/system/test_connection.py ./$(EXE)


debug: CXXFLAGS += $(debug_flags)
debug: $(EXE)

release: CXXFLAGS += $(release_flags)
release: $(EXE)

$(EXE): main.o client.o session.o exception.o udp_message_factory.o message.o\
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

message.o: message.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

udp_message_factory.o: udp_message_factory.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

$(TESTEXE): test/test_main.o test/udp_message_serialize_tests.o test/tcp_message_factory_tests.o\
											udp_message_factory.o message.o test/udp_message_factory_tests.o tcp_message_factory.o
	$(CXX) $(CXXFLAGS) -lgtest $^ -o $@

test/test_main.o: test/test_main.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

test/udp_message_serialize_tests.o: test/udp_message_serialize_tests.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

test/udp_message_factory_tests.o: test/udp_message_factory_tests.cpp
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

xvasil10.zip:
	7za a $@ *.cpp *.h test *.md LICENSE Makefile

clean:
	$(RM) *.o $(EXE) test/*.o

.PHONY: test clean
