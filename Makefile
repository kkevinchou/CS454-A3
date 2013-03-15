CXX = g++					# compiler
CXXFLAGS = -g -Wall -Wno-unused-label -MMD -DTYPE="${TYPE}"  # compiler flags
MAKEFILE_NAME = ${firstword ${MAKEFILE_LIST}}	# makefile name
SOURCES = $(wildcard *.cpp)

OBJECTS1 = binder.o sender.o helpers.o receiver.o constants.o rwbuffer.o
EXEC1 = binder				# 1st executable name

OBJECTS = ${OBJECTS1}
DEPENDS = ${OBJECTS:.o=.d}			# substitute ".o" with ".d"
EXECS = ${EXEC1}

#############################################################

.PHONY : all clean

all : ${EXECS}					# build all executables
	${CXX} -c rpcserver.cpp rpcclient.cpp server.cpp client.cpp server_functions.cpp server_function_skels.cpp
	ar rcs librpc.a sender.o receiver.o constants.o helpers.o rwbuffer.o rpcserver.o rpcclient.o

${EXEC1} : ${OBJECTS1}				# link step 1st executable
	${CXX} $^ -o $@ -lpthread
#############################################################

${OBJECTS} : ${MAKEFILE_NAME}			# OPTIONAL : changes to this file => recompile

-include ${DEPENDS}				# include *.d files containing program dependences

clean :						# remove files that can be regenerated
	rm -f *.d *.o ${EXECS}
