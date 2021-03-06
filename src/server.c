/**
 * webserver.c -- A webserver written in C
 * 
 * Test with curl (if you don't have it, install it):
 * 
 *    curl -D - http://localhost:3490/
 *    curl -D - http://localhost:3490/d20
 *    curl -D - http://localhost:3490/date
 * 
 * You can also test the above URLs in your browser! They should work!
 * 
 * Posting Data:
 * 
 *    curl -D - -X POST -H 'Content-Type: text/plain' -d 'Hello, sample data!' http://localhost:3490/save
 * 
 * (Posting data is harder to test from a browser.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>
#include "net.h"
#include "file.h"
#include "mime.h"
#include "cache.h"

#define PORT "3490"  // the port users will be connecting to

#define SERVER_FILES "./serverfiles"
#define SERVER_ROOT "./serverroot"

/**
 * Send an HTTP response
 *
 * header:       "HTTP/1.1 404 NOT FOUND" or "HTTP/1.1 200 OK", etc.
 * content_type: "text/plain", etc.
 * body:         the data to send.
 * 
 * Return the value from the send() function.
 */
int send_response(int fd, char *header, char *content_type, void *body, int content_length)
{
    const int max_response_size = 262144;
    char response[max_response_size];

    // Build HTTP response and store it in response

    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    // Setting up the Time Stamp
    // time_t = https://en.cppreference.com/w/c/chrono/time_t = (in short) = integer value = seconds
    time_t seconds;
    // struct for time information (tm)
    struct tm *info;
    char buffer[80];
    time(&seconds);

    info = localtime(&seconds);

   // header, date, content_length, , connection
    int response_length = sprintf(response, "%s\n Date: %s\n Content Length: %d\n Content Type: %s\n Connection: close\n", "\n", header, asctime(info), content_length, content_type);

    // we need to advance where we are pointing in the response buffer, (to the end of the header)
    // to pass the header and not over write all the header data = (res + res_leng),
    memcpy(response + response_length, body, content_length);

    // Send it all! / Read Request + content_length,
    int rv = send(fd, response, response_length, 0);

    if (rv < 0) {
        perror("send");
    }

    return rv;
}


/**
 * Send a /d20 endpoint response
 */
void get_d20(int fd)
{
    // Generate a random number between 1 and 20 inclusive
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    // Use send_response() to send it back as text/plain data
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    char body[30];
    // getting a random number between 1 and 20
    int random = rand() % 20 + 1;
    // packaging up the body
    int body_length = sprintf(body, "%d\n", random);
    // use send_resp to send it back as text/plain data
    send_response(fd, "HTTP/1.1 200 OK", "text/plain", body, body_length);
}

/**
 * Send a 404 response
 */
void resp_404(int fd)
{
    char filepath[4096];
    struct file_data *filedata; 
    char *mime_type;

    // Fetch the 404.html file
     // snprintf() formats and stores a series of chars and values in the array buffer
    snprintf(filepath, sizeof filepath, "%s/404.html", SERVER_FILES);
    filedata = file_load(filepath);

    if (filedata == NULL) {
        // TODO: make this non-fatal
        fprintf(stderr, "cannot find system 404 file\n");
        exit(3);
    }

    mime_type = mime_type_get(filepath);

    send_response(fd, "HTTP/1.1 404 NOT FOUND", mime_type, filedata->data, filedata->size);

    file_free(filedata);
}

/**
 * Read and return a file from disk or cache
 */
void get_file(int fd, struct cache *cache, char *request_path)
{
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    // pointer to file_data struct
    struct file_data *filedata = NULL;
    // declare an array to store the file path in
    char file_path[4096];
    // pointer to mime/content_type
    char *mime_type;
    // get the entry from the cache
    struct cache_entry *entry = cache_get(cache, request_path);

    // if no entry found in cache
    if (entry == NULL) {
        // build file path to ./serverroot
        sprintf(file_path, "./serverroot %s", request_path);
        filedata = file_load(file_path);
        if (filedata == NULL) {
            // error handling
            resp_404(fd);
            return;
        }
    }
    // checks the file / returns content string
    mime_type = mime_type_get(file_path);
    //
    cache_put(cache, request_path, mime_type, filedata->data, filedata->size);
    send_response(fd, "HTTP/1.1 200 OK", mime_type, filedata->data, filedata->size);
    file_free(filedata);
    return;
}

/**
 * Search for the end of the HTTP header
 * 
 * "Newlines" in HTTP can be \r\n (carriage return followed by newline) or \n
 * (newline) or \r (carriage return).
 */
char *find_start_of_body(char *header)
{
    ///////////////////
    // IMPLEMENT ME! // (Stretch)
    ///////////////////
}

/**
 * Handle HTTP request and send response
 */
void handle_http_request(int fd, struct cache *cache)
{
    const int request_buffer_size = 65536; // 64K
    char request[request_buffer_size];

    // Read request
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

    if (bytes_recvd < 0) {
        perror("recv");
        return;
    }
    
    // testing
    // resp_404(fd);
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    // Read the three components of the first request line
    // If GET, handle the get endpoints
    //   Check if it's /d20 and handle that special case
    //   Otherwise serve the requested file by calling get_file()
    // (Stretch) If POST, handle the post request
    char request_method[20], request_endpoint[20], request_protocol[20];
    sscanf(request, "%s %s %s", request_method, request_endpoint, request_protocol);
    // just checking the request here
    printf("Request: method: %s , endpt: %s , protocol %s\n", request_method, request_endpoint, request_protocol);
    // Handle GET endpts
    if (strcmp(request_method, "GET")) {
        // check if /d20
        if (strcmp(request_endpoint, "/d20")) {
            get_d20(fd);
            return;
        } else {
            get_file(fd, cache, request_endpoint);
            return;
        }
    }
}

/**
 * Main
 */
int main(void)
{
    int newfd;  // listen on sock_fd, new connection on newfd
    struct sockaddr_storage their_addr; // connector's address information
    char s[INET6_ADDRSTRLEN];

    struct cache *cache = cache_create(10, 0);

    // Get a listening socket
    int listenfd = get_listener_socket(PORT);

    if (listenfd < 0) {
        fprintf(stderr, "webserver: fatal error getting listening socket\n");
        exit(1);
    }

    printf("webserver: waiting for connections on port %s...\n", PORT);

    // This is the main loop that accepts incoming connections and
    // forks a handler process to take care of it. The main parent
    // process then goes back to waiting for new connections.
    
    while(1) {
        socklen_t sin_size = sizeof their_addr;

        // Parent process will block on the accept() call until someone
        // makes a new connection:
        newfd = accept(listenfd, (struct sockaddr *)&their_addr, &sin_size);
        if (newfd == -1) {
            perror("accept");
            continue;
        }

        // Print out a message that we got the connection
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);
        
        // newfd is a new socket descriptor for the new connection.
        // listenfd is still listening for new connections.

        handle_http_request(newfd, cache);

        close(newfd);
    }

    // Unreachable code

    return 0;
}

