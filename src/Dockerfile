FROM ubuntu:latest
# Install any necessary dependencies for proxy
ENV DEBIAN_FRONTEND=noninteractive
# Use an appropriate base image with the necessary build tools and libraries installed
# Install any necessary dependencies
RUN apt-get update && \
    apt-get install -y g++ && \
    apt-get install -y make && \
    apt-get install -y libboost-all-dev

# Set the working directory
WORKDIR /src

# Copy the necessary files to the container

COPY main /src/main

# Build the application using the makefile
#RUN make

# Set the default command to start the application
#CMD ["./main"]
