#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string>
#include <fstream>
#include <sstream>
#include <thread>

// THE WORKER: This function runs on its own separate thread for every visitor
void handle_client(int client_socket) {
    // 1. Announce the start (The "Numbers" you are looking for)
    std::cout << "\n[NEW REQUEST] Worker Thread " << std::this_thread::get_id() << " is starting..." << std::endl;

    char buffer[1024] = {0};
    read(client_socket, buffer, 1024);

    std::string request(buffer);
    std::string filename = "index.html"; // Default page

    // 2. THE BRAIN (Routing)
    if (request.find("GET /about") != std::string::npos) {
        filename = "about.html";
    }

    // 3. THE LIBRARIAN (File Serving)
    std::ifstream file(filename);
    std::string response;

    if (file.is_open()) {
        std::stringstream ss;
        ss << file.rdbuf(); // Pour the file content into the stream
        std::string content = ss.str();
        
        // Build the HTTP response
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + content;
    } else {
        response = "HTTP/1.1 404 Not Found\r\n\r\n<h1>404: File Not Found</h1>";
    }

    // 4. THE HANDSHAKE (Send and Close)
    send(client_socket, response.c_str(), response.size(), 0);
    
    // Final Announcement
    std::cout << "[SUCCESS] Thread " << std::this_thread::get_id() << " served " << filename << std::endl;

    close(client_socket); 
}

int main() {
    // 1. Create the Socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    // 2. Setup Address Reuse (Allows you to restart the server quickly)
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080); // Listening on Room 8080

    // 3. Bind and Listen
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed! Try a different port or wait a minute." << std::endl;
        return -1;
    }

    listen(server_fd, 10); 

    std::cout << "------------------------------------------" << std::endl;
    std::cout << " Hyperion ULTIMATE (Multi-threaded) Online " << std::endl;
    std::cout << " Listening on http://localhost:8080        " << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    while(true) {
        // MANAGER: Wait for a connection
        int client_socket = accept(server_fd, nullptr, nullptr);
        
        if (client_socket >= 0) {
            // HIRE WORKER: Pass the socket to a new thread
            std::thread worker(handle_client, client_socket);
            
            // DETACH: Let the worker finish independently
            worker.detach(); 
        }
    }

    close(server_fd);
    return 0;
}