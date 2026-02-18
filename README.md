# Hyperion: Multi-threaded Static Web Server (C++)

Hyperion is a high-performance web server built from the ground up using C++ and the Linux Socket API. This project demonstrates deep-level systems programming, focusing on how the operating system handles networking, concurrency, and file I/O.

## 🛠️ Technical Implementation
The project implements the full lifecycle of a web request at the systems level:
- **Socket Architecture:** Utilizes POSIX sockets for low-level network communication over TCP/IP.
- **Multi-threading:** Implements a detached thread model (`std::thread`). When a connection is accepted, a worker thread is spawned to process the HTTP request, allowing the main listener to remain available for new incoming connections.
- **HTTP Header Parsing:** Captures and parses raw browser buffers to identify the requested path (e.g., `/`, `/about`, or `/favicon.ico`) as seen in the server logs.
- **Disk I/O:** Integrates `std::ifstream` and `std::stringstream` to serve physical HTML assets from the local filesystem based on the parsed request.
- **Socket Options:** Implements `SO_REUSEADDR` to bypass the TCP `TIME_WAIT` state, allowing for immediate server restarts.

## 📸 Output & Proof of Work

### Real-time Request Handling & Multi-threading
The following log shows the server successfully capturing raw HTTP requests from Google Chrome and assigning unique **Thread IDs** to handle each request concurrently:

![Terminal Logs](Screenshot_20260218_110619.png)

### Web Interface
The server successfully routes the browser to the correct HTML documents based on the URL path:

![Browser View](Screenshot_20260218_104210.png)

## 🏗️ Build & Run
1. **Compile with Pthread support:**
   ```bash
   g++ server.cpp -o Hyperion -lpthread
   Execute:
./Hyperion
Access: Open http://localhost:8080 in any web browser.
