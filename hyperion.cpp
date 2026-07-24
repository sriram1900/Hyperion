#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string>
#include <fstream>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <list>
#include <mutex>
// ==========================================
// 1. THE CACHE SKELETON (Placed at the very top!)
// ==========================================
class LRUCache {
private:
    int capacity; 
    std::list<std::string> order; 
    std::unordered_map<std::string, std::pair<std::string, std::list<std::string>::iterator>> cache_map; 
    std::mutex cache_lock; 

public:
    // Constructor
    LRUCache(int cap) {
        capacity = cap;
    }

    // --- PASTE THIS NEW GET FUNCTION HERE ---
    std::string get(const std::string& filename) {
        // 1. Lock the cache so threads don't crash
        std::lock_guard<std::mutex> lock(cache_lock); 
        
        // 2. Look inside the Hash Map. If we reach the end, it means it's not here.
        if (cache_map.find(filename) == cache_map.end()) {
            return ""; // Cache Miss! 
        }
        
        // 3. Cache Hit! We found it. 
        // Erase it from its old spot in the linked list...
        order.erase(cache_map[filename].second);
        
        // ...and push it to the very front of the line (Most Recently Used!)
        order.push_front(filename);
        
        // Update the dictionary so it knows the file's new spot at the front
        cache_map[filename].second = order.begin();
        
        // Return the actual HTML text content
        return cache_map[filename].first; 
    }
        // --- NEW: put() actually adds/updates entries and evicts when full ---
    void put(const std::string& filename, const std::string& content) {
        std::lock_guard<std::mutex> lock(cache_lock);

        if (cache_map.find(filename) != cache_map.end()) {
            // Already cached — just remove old position, we'll re-insert at front below
            order.erase(cache_map[filename].second);
        } else if ((int)order.size() >= capacity) {
            // Cache full and this is a new file — evict least recently used (back of list)
            std::string lru_key = order.back();
            order.pop_back();
            cache_map.erase(lru_key);
        }

        order.push_front(filename);
        cache_map[filename] = {content, order.begin()};
    }
};
    
// --- NEW: one shared cache instance, used by every thread ---

LRUCache page_cache(10); // holds up to 10 files at once

// THE WORKER: This function runs on its own separate thread for every visitor

void handle_client(int client_socket) {
    // 1. Announce the start
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
std::string response;
   // --- NEW: check the cache first ---
    std::string cached_content = page_cache.get(filename);

    if (!cached_content.empty()) {
        // Cache HIT — skip disk entirely
        response = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html\r\n"
                   "Content-Length: " + std::to_string(cached_content.size()) + "\r\n"
                   "Connection: close\r\n"
                   "\r\n" +
                   cached_content;
        std::cout << "[CACHE HIT] " << filename << std::endl;
    } else {
        // Cache MISS — read from disk like before
        std::ifstream file(filename);

if (file.is_open()) {
    std::stringstream ss;
    ss << file.rdbuf(); // Pour the file content into the stream
    std::string content = ss.str();
    
    // Build the HTTP response WITH Content-Length
    response = "HTTP/1.1 200 OK\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: " + std::to_string(content.size()) + "\r\n"
               "Connection: close\r\n"
               "\r\n" + 
               content;
    page_cache.put(filename, content); // --- NEW: store it in the cache for next time
     std::cout << "[CACHE MISS] " << filename << " read from disk and cached" << std::endl;
} else {
    std::string not_found = "404: File Not Found";
    response = "HTTP/1.1 404 Not Found\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: " + std::to_string(not_found.size()) + "\r\n"
               "Connection: close\r\n"
               "\r\n" + 
               not_found;
}
    }

    // 4. THE HANDSHAKE (Send and Close)
    send(client_socket, response.c_str(), response.size(), 0);
    
    // Final Announcement
    std::cout << "[SUCCESS] Thread " << std::this_thread::get_id() << " served " << filename << std::endl;

    close(client_socket); 
}

// THE MANAGER: Sets up the server and listens
int main() {
    // =================================================================
    // 1. CREATE THE SOCKET (This is the exact line your compiler missed!)
    // =================================================================
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed!" << std::endl;
        return -1;
    }

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
    std::cout << "[SUCCESS] Receptionist locked to Port 8080 successfully." << std::endl;

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