CXX = g++					# compiler
CXXFLAGS = -g -Wall -Wno-unused-label -MMD -DTYPE="${TYPE}"  # compiler flags
MAKEFILE_NAME = ${firstword ${MAKEFILE_LIST}}	# makefile name

BLAH = ar rcs librpc.a sender.o receiver.o constants.o helpers.o rwbuffer.o rpcserver.o rpcclient.o

OBJECTS1 = binder.o sender.o helpers.o receiver.o constants.o rwbuffer.o
EXEC1 = binder				# 1st executable name

OBJECTS2 = server.o rpcserver.o helpers.o server_function_skels.o server_functions.o sender.o constants.o receiver.o rwbuffer.o
EXEC2 = server				# 2st executable name

OBJECTS3 = client.o helpers.o sender.o receiver.o rpcclient.o constants.o rwbuffer.o
EXEC3 = client				# 3rd executable name

OBJECTS4 = client1.o helpers.o sender.o receiver.o rpcclient.o constants.o rwbuffer.o
EXEC4 = client1				# 4th executable name

OBJECTS5 = client2.o helpers.o sender.o receiver.o rpcclient.o constants.o rwbuffer.o
EXEC5 = client2				# 5th executable name

OBJECTS6 = server2.o rpcserver.o helpers.o server_function_skels.o server_functions.o sender.o constants.o receiver.o rwbuffer.o
EXEC6 = server2				# 6th executable name

OBJECTS = ${OBJECTS1} ${OBJECTS2} ${OBJECTS3}	${OBJECTS4}	${OBJECTS5}	 ${OBJECTS6}		# all object files
DEPENDS = ${OBJECTS:.o=.d}			# substitute ".o" with ".d"
EXECS = ${EXEC1} ${EXEC2} ${EXEC3}	${EXEC4} ${EXEC5} ${EXEC6}			# all executables

#############################################################

.PHONY : all clean

all : ${EXECS}					# build all executables
	ar rcs librpc.a sender.o receiver.o constants.o helpers.o rwbuffer.o rpcserver.o rpcclient.o

${EXEC1} : ${OBJECTS1}				# link step 1st executable
	${CXX} $^ -o $@ -lpthread

${EXEC2} : ${OBJECTS2}				# link step 1st executable
	${CXX} $^ -o $@ -lpthread

${EXEC3} : ${OBJECTS3}				# link step 1st executable
	${CXX} $^ -o $@ -lpthread

${EXEC4} : ${OBJECTS4}				# link step 1st executable
	${CXX} $^ -o $@ -lpthread

${EXEC5} : ${OBJECTS5}				# link step 1st executable
	${CXX} $^ -o $@ -lpthread

${EXEC6} : ${OBJECTS6}				# link step 1st executable
	${CXX} $^ -o $@ -lpthread
#############################################################

${OBJECTS} : ${MAKEFILE_NAME}			# OPTIONAL : changes to this file => recompile

-include ${DEPENDS}				# include *.d files containing program dependences

clean :						# remove files that can be regenerated
	rm -f *.d *.o ${EXECS}
