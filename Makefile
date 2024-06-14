# Compiler
CXX = g++

# Flags
CXXFLAGS = -std=c++11 -fdiagnostics-color=always -g -O2 -I. \
           -DHAVE_DECL_LE64TOH=1 \
           -DHAVE_DECL_HTOBE16=1 \
           -DHAVE_DECL_HTOLE16=1 \
           -DHAVE_DECL_BE16TOH=1 \
           -DHAVE_DECL_LE16TOH=1 \
           -DHAVE_DECL_HTOBE32=1 \
           -DHAVE_DECL_HTOLE32=1 \
           -DHAVE_DECL_BE32TOH=1 \
           -DHAVE_DECL_LE32TOH=1 \
           -DHAVE_DECL_HTOBE64=1 \
           -DHAVE_DECL_HTOLE64=1 \
           -DHAVE_DECL_BE64TOH=1 \
           -DHAVE_DECL_STRNLEN=1

# Output binary
TARGET = komodo-chainparams

# Source files
SRCS = main.cpp \
       komodo_globals.cpp \
       komodo_utils.cpp \
       crypto/sha256.cpp \
       crypto/ripemd160.cpp \
       util.cpp \
       hex.c

# Object files
OBJS = $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)

# Default rule
all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to compile C++ source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to compile C source files
%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
