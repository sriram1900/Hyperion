Hyperion: Multi-threaded HTTP Server (C++)

Hyperion is a high-performance HTTP server built from scratch in C++ using the raw POSIX Sockets API — no frameworks, no libraries handling the network or HTTP layer. It handles concurrent client requests using a multi-threaded, thread-per-connection model, and includes a hand-built thread-safe LRU cache to reduce redundant disk reads.

🛠️ Technical Implementation
Socket Architecture: Raw POSIX sockets for low-level TCP/IP communication — connection setup, binding, and listening handled manually.
Multi-threading: Detached thread-per-connection model (std::thread). Every incoming connection spawns its own worker thread, so one slow client never blocks others.
HTTP Parsing: Captures and parses raw request headers to route paths like / and /about to the correct file.
HTTP Response Correctness: Responses include an explicit Content-Length header and Connection: close, so clients know exactly when a response is complete instead of relying on the socket closing as an implicit signal (see Performance Investigation below for why this mattered).
Caching Layer: A thread-safe LRU (Least Recently Used) cache — hash map + doubly linked list for O(1) get/put — sits in front of disk reads, with mutex-guarded access and proper eviction when the cache is full.
Disk I/O: On a cache miss, serves HTML assets from the filesystem via std::ifstream, then populates the cache for future requests.
Socket Options: SO_REUSEADDR for instant server restarts during development.
📸 Output & Proof of Work
Web Interface

The server correctly routes and serves HTML from disk:

![Browser View](https://github.com/sriram1900/Hyperion/raw/main/localhost.png)

LRU Cache in Action

Terminal logs show worker threads handling concurrent requests, with clear CACHE HIT / SUCCESS entries confirming the LRU cache is serving repeat requests without hitting disk:

![LRU Cache Logs](https://github.com/sriram1900/Hyperion/raw/main/lrucache1.png)

Load Test Results

Benchmark run using wrk, confirming throughput and latency numbers discussed below:

![Benchmark Results](https://github.com/sriram1900/Hyperion/raw/main/result.png)

📊 Performance Investigation & Benchmarking

Rather than assuming the server was fast, I benchmarked it under load using wrk and used the results to find and fix a real bug.

Initial benchmark (50 concurrent connections, 10s, 2 threads):

Requests/sec:   6581.86
Socket errors: connect 0, read 65863, write 0, timeout 0

The read-error count almost exactly matched the total request count — a strong signal something was wrong. Investigating with curl -v showed the server's responses had no Content-Length header, so clients had no way to know a response was complete except waiting for the socket to close — adding latency and wasted reads to every single request.

Fix: added an explicit Content-Length header and Connection: close to every response.

Result, re-benchmarked (5 runs for stability):

Run 1:  12,293 req/sec
Run 2:  11,832 req/sec
Run 3:  12,352 req/sec
Run 4:  11,577 req/sec
Run 5:  13,840 req/sec
Average: ~12,380 req/sec

~1.8x throughput improvement, with p50 latency consistently around 0.4ms and p90 around 1.6ms across runs.

On the LRU cache specifically: correctly implemented and verified (confirmed via HIT/MISS logging — first request to a file logs a MISS and reads disk, every subsequent request logs a HIT and skips disk entirely). Benchmarking showed limited additional throughput gain on top of the Content-Length fix for a small, single repeatedly-requested file — most likely because Linux's own OS-level page cache was already caching it in memory. The cache's value would show more clearly under a larger working set that exceeds available OS cache — an honest finding, not a failure of the implementation.

# 🏗️ Build & Run

You can run Hyperion using Docker, or build it locally using CMake or raw `g++`.

### 1. Docker (Recommended)
The easiest way to run the server without worrying about local C++ toolchains.
```bash
docker build -t hyperion-server .
docker run -p 8080:8080 -d hyperion-server
2. Local Build: CMake
If you want to build from source, CMake is the standard approach.

Bash
mkdir build && cd build
cmake ..
make
./hyperion
3. Local Build: Raw g++
For a quick manual compile with pthread support:

Bash
g++ -O3 hyperion.cpp -o hyperion -pthread
./hyperion
Access & Benchmarking
Once the server is up, access it at: http://localhost:8080

Run your own load test using wrk:

Bash
wrk -t2 -c50 -d10s --latency http://localhost:8080/
