# Use an official C++ image with CMake
FROM ubuntu:latest

# Install dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    make \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# Set working directory inside the container
WORKDIR /app

# Copy the project files
COPY . .

# Create a build directory
RUN mkdir -p build

# Run CMake (remove old cache before configuring)
RUN cd build && rm -rf CMakeCache.txt CMakeFiles && cmake .. && make

# Default command
CMD ["./build/SimDB","bash"]
