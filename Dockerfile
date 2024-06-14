# Use Alpine Linux as the base image
FROM alpine:latest

# Install dependencies
RUN apk add --no-cache \
    git \
    g++ \
    make

# Set the working directory
WORKDIR /app

# Clone the project from GitHub
RUN git clone https://github.com/DeckerSU/komodo-chainparams.git .

# Build the project using Makefile
RUN make Makefile_docker

# Define entrypoint to pass all parameters
ENTRYPOINT ["./komodo-chainparams"]

# Default command if no parameters are passed
CMD ["--help"]
