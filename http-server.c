// Stephanie Pan
// sp3507
// Lab7 Part 2
// http-server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


static void die(const char *s) {
    perror(s);
    exit(1); 
}

int main(int argc, char **argv) {

    if (argc != 5) {
        fprintf(stderr, "usage: %s <server_port> <web_root> <mdb-lookup-host> <mdb-lookup-port>\n", argv[0]); 
        exit(1); 
    }

    unsigned short port = atoi(argv[1]); 
    const char *webroot = argv[2]; 
    const char *mdblookuphost = argv[3]; 
    unsigned short mdbport = atoi(argv[4]);

    // Create a listening socket for client  
    int servsock; 
    if ((servsock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        die("socket failed"); 

    // Construct local address structure for client 
    struct sockaddr_in servaddr; 
    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(port); 

    // Bind to the local address of client 
    if (bind(servsock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        die("bind failed"); 

    // Start listening for incoming connections (client) 
    if (listen(servsock, 5)) 
        die("listen failed"); 
    
    int clntsock;
    socklen_t clntlen;
    struct sockaddr_in clntaddr; 

    // create a socket as a client for mdb
    int mdbsock; // socket descriptor 
    if ((mdbsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        die("socket failed"); 

    // making IP address of local host 
    struct hostent *he; 
    if ((he = gethostbyname(mdblookuphost)) == NULL) {
        die("gethostbyname failed"); 
    }
    char *mdbIP = inet_ntoa(*(struct in_addr *) he->h_addr); 

    // construct a server address structure
    struct sockaddr_in mdbservaddr; 
    memset(&mdbservaddr, 0, sizeof(mdbservaddr));
    mdbservaddr.sin_family = AF_INET; 
    mdbservaddr.sin_addr.s_addr = inet_addr(mdbIP); 
    mdbservaddr.sin_port = htons(mdbport); 

    // establish a TCP connection to the mdb-server 
    if (connect(mdbsock, (struct sockaddr *) &mdbservaddr, sizeof(mdbservaddr)) < 0)
        die("connect to mdb-lookup-server failed"); 

    FILE *mdbinput = fdopen(mdbsock, "r");

    while (1) {
      
        // accept an incoming connection 
        clntlen = sizeof(clntaddr); 
        
        if((clntsock = accept(servsock, (struct sockaddr *) &clntaddr, &clntlen)) < 0)
            die("accept failed"); 

        FILE *input = fdopen(clntsock, "r"); 

        // take in request from client
        int goodMsg = 1; 
        char requestLine[200];
        char *token_separators; 
        char *method; 
        char *requestURI; 
        char *httpVersion;
        if (fgets(requestLine, 200, input) != NULL) {
            token_separators = "\t \r\n"; 
            method = strtok(requestLine, token_separators); 
            requestURI = strtok(NULL, token_separators); 
            httpVersion = strtok(NULL, token_separators);  

            if (strcmp(method, "GET") != 0) { // checking for GET method 
                char *errormsg = "HTTP/1.0 501 Not Implemented \r\n \r\n <html><body><h1>501 Not Implemented</h1></body></html>"; 
                send(clntsock, errormsg, strlen(errormsg), 0);
                goodMsg = 0; 
            }  

            if ((strcmp(httpVersion, "HTTP/1.0") != 0) && (strcmp(httpVersion, "HTTP/1.1") != 0)) { // checking for HTTP/1.0 or HTTP
                char *errormsg = "HTTP/1.0 501 Not Implemented \r\n \r\n <html><body><h1>501 Not Implemented</h1></body></html>"; 
                send(clntsock, errormsg, strlen(errormsg), 0); 
                goodMsg = 0; 
            }

            if (requestURI[0] != '/') { // checking first character of request URI 
                char *errormsg = "HTTP/1.0 400 Bad Request \r\n \r\n <html><body><h1>400 Bad Request</h1></body></html>";
                send(clntsock, errormsg, strlen(errormsg), 0); 
                goodMsg = 0; 
            }

            if (strstr(requestURI, "/../") != NULL || strstr(requestURI, "/..") != NULL) { // checking for "/../" and "/.."
                char *errormsg = "HTTP/1.0 400 Bad Request \r\n \r\n <html><body><h1>400 Bad Request</h1></body></html>";
                send(clntsock, errormsg, strlen(errormsg), 0);  
                goodMsg = 0; 
            } 
        }
        else {
            char *errormsg = "HTTP/1.0 400 Bad Request \r\n \r\n <html><body><h1>400 Bad Request</h1><body><html>";
            send(clntsock, errormsg, strlen(errormsg), 0); 
            goodMsg = 0; 
        }


        // send file back as long as request msg is ok 
        if (goodMsg == 1) {

            const char *form = 
                "<h1>mdb-lookup</h1>\n"
                "<p>\n"
                "<form method=GET action=/mdb-lookup>\n"
                "lookup: <input type=text name=key>\n"
                "<input type=submit>\n"
                "</form>\n"
                "<p>\n"; 
            if (strncmp(requestURI, "/mdb-lookup?key=", strlen("/mdb-lookup?key=")) == 0) {// mdb-lookup w/ search term protocol
            
                // getting search term from requestURI
                char searchterm[50]; 
                strcpy(searchterm, requestURI+16);  
                send(mdbsock, searchterm, strlen(searchterm), 0); 
                send(mdbsock, "\n", strlen("\n"), 0); 

                // send header
                char *okmsg = "HTTP/1.0 200 OK\r\n\r\n";
                send(clntsock, okmsg, strlen(okmsg), 0); 

                // send HTML file manually
                send(clntsock, "<html>", strlen("<html>"), 0); 
                char *colors = "<head><style>tr:nth-child(even){background:pink}</style></head><body>";
                send(clntsock, colors, strlen(colors), 0); 
                send(clntsock, form, strlen(form), 0); 
                send(clntsock, "<table border=1>", strlen("<table border=1>"), 0); 

                char line[100]; 
                while (fgets(line, sizeof(line), mdbinput) != NULL) { // while loop to accept entries from mdb

                    if(line[0] == '\n') 
                        break; 
                    send(clntsock, "<tr><td>", strlen("<tr><td>"), 0); 
                    send(clntsock, line, strlen(line), 0);                
                    send(clntsock, "</td></tr>", strlen("</td></tr>"), 0); 
                }
                send(clntsock, "</table>", strlen("</table>"), 0); 
                send(clntsock, "</body></html>", strlen("</body></html>"), 0); 
                //logging file sent
                fprintf(stderr, "looking up [%s]: %s \"%s %s %s\" 200 OK \r\n", searchterm, inet_ntoa(clntaddr.sin_addr), method, requestURI, httpVersion); 
            }
            else if (strcmp(requestURI, "/mdb-lookup") == 0) { // mdb-lookup protocol

                //send header
                char *okmsg = "HTTP/1.0 200 OK\r\n\r\n";
                send(clntsock, okmsg, strlen(okmsg), 0); 

                // send file
                send(clntsock, "<html><body>", strlen("<html><body>"), 0);
                send(clntsock, form, strlen(form), 0); 
                send(clntsock, "</body></html>", strlen("</body></html>"), 0); 
            }
            else { // regular file protocol
                
                char fullpathtofile[1000];
                char printablepath[200];
                strcpy(fullpathtofile, webroot);
                strcat(fullpathtofile, requestURI); 
                strcpy(printablepath, requestURI); 
  
                // file vs. directory code
                struct stat buffer; 
                stat(fullpathtofile, &buffer); 
                if (!S_ISREG(buffer.st_mode)) {
                    if (fullpathtofile[strlen(fullpathtofile)-1] == '/') {
                        strcat(fullpathtofile, "index.html");
                        strcat(printablepath, "index.html"); 
                    }
                    else {
                        strcat(fullpathtofile, "/index.html"); 
                        strcat(printablepath, "/index.html"); 
                    }
                }
        
                FILE *fp = fopen(fullpathtofile, "rb");
                if (fp == NULL) {
                    char *errormsg = "HTTP/1.0 404 Not Found\r\n\r\n <html><body><h1>404 Not Found\r\n</h1></body></html>";
                    send(clntsock, errormsg, strlen(errormsg), 0); 
                }
                else { // sending file 
                    char *okmsg = "HTTP/1.0 200 OK\r\n\r\n";
                    send(clntsock, okmsg, strlen(okmsg), 0);  

                    size_t c; 
                    char buf[4096]; 
                    memset(buf, 0, sizeof(buf)); 
                    while((c = fread(buf, 1, sizeof(buf), fp)) > 0) { 
                        if (send(clntsock, buf, c, 0) != c)  
                            die("file send1 failed"); 
                    }
                    send(clntsock, "\r\n", sizeof("\r\n"), 0); 

                    if (ferror(fp)) 
                        die("file send failed"); 
                    
                    //logging file sent
                    fprintf(stderr, "%s \"%s %s %s\" 200 OK \r\n", inet_ntoa(clntaddr.sin_addr), method, printablepath, httpVersion); 

                    fclose(fp);  
                }

            }
            
        }
        fclose(input);  
        close(clntsock); 

    } // end while(1) loop

    fclose(mdbinput); 

    return 0; 

} // end main
