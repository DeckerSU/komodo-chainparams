# Use Alpine Linux as the base image for building
FROM alpine:latest AS builder

# Install dependencies for building
RUN apk add --no-cache \
    git \
    g++ \
    make

# Set the working directory
WORKDIR /app

# Clone the project from GitHub
RUN git clone https://github.com/DeckerSU/komodo-chainparams.git .
# COPY . .

# Build the project using Makefile
RUN make -f Makefile_docker all

# Use a minimal base image for the final stage
FROM alpine:latest

# Install runtime dependencies
RUN apk add --no-cache \
    libstdc++

# Set the working directory
WORKDIR /app

# Copy the built files from the builder stage
COPY --from=builder /app/komodo-chainparams /app/

# Define entrypoint to pass all parameters
ENTRYPOINT ["./komodo-chainparams"]

# Default command if no parameters are passed
CMD ["--help"]
