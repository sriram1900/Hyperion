FROM ubuntu:22.04
RUN apt-get update && apt-get install -y build-essential
WORKDIR /app
COPY . /app
RUN g++ hyperion.cpp -o hyperion -pthread
CMD ["./hyperion"]