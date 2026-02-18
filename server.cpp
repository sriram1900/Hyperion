#include<iostream>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<string>
int main(){
    //connection with socket
    //1.socket-buying the phone
    int server_fd=socket(AF_INET,SOCK_STREAM,0);
    if(server_fd<0)
    {
        std::cerr<<" phone store os closed!(socket failed)"<<std::endl;
        return-1;
    }
    //2.bind-setting the phone number
    //computer needs a structured way to store your IP and Port together.
    sockaddr_in address;
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port = htons(8080);       // Room number 8080 [cite: 561]
    if(bind(server_fd,(struct sockaddr*)&address,sizeof(address))<0){
        std::cerr<<"room 8080 is already taken!(bind failed)"<<std::endl;
        return -1;
    }
    //3.listen-turning the ringer on
    if(listen(server_fd,3)<0){
        std::cerr<<"ringer broken!(listen failed)"<<std::endl;
        return -1;
    }
    std::cout<<"hyperion is waiting for piza order on port 8080"<<std::endl; 
    //we start loop here so that program never hits the final return 0
    while(true){
        int new_socket=accept(server_fd,nullptr,nullptr);
        if(new_socket<0){
        std::cerr<<"couldn't answer the phone(accept failed)"<<std::endl;
        continue;
        }
         std::cout << "Connection accepted! A customer has arrived." << std::endl;
         //THE LISTENING
        
        // 1. We create our "Bucket" (The Buffer)
        char buffer[1024] = {0}; 

        // 2. We tell the OS: "Push data from new_socket into our buffer"
        // This is the moment the OS hands the "message" to your code.
        read(new_socket, buffer, 1024);

        // 3. We print the message to see what the browser asked for
        std::cout << "----- BROWSER REQUEST -----\n" << buffer << "\n----------------------" << std::endl;
        // --- STEP 4: THE ROUTER (NEW CODE GOES HERE) ---
        
        // Convert the buffer to a string so we can search it easily
        std::string request(buffer);
        std::string response;

        if (request.find("GET /about") != std::string::npos) {
            // If the browser asked for /about
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>About Hyperion</h1><p>This is the about page!</p>";
        } 
        else if (request.find("GET / ") != std::string::npos) {
            // If the browser asked for the home page (root)
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Hyperion Home</h1><p>Welcome to the main server.</p>";
        } 
        else {
            // If they asked for a page that doesn't exist
            response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1><p>Page does not exist!</p>";
        }
        send(new_socket, response.c_str(), response.size(), 0);

        // 3. Hang up the private line
        close(new_socket); 
        
        std::cout << "Customer served. Waiting for next..." << std::endl;

    }
     // Hang up the phonee
     close(server_fd);
     return 0;
}