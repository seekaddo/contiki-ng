
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

int main(void) {
  int socket_desc;
  struct sockaddr_in6 server_addr;
  struct sockaddr_in6 my_addr1;
  char server_message[2000];
  char client_message[2000] = "Hellow World one here!!";
  socklen_t server_struct_length = sizeof(server_addr);

  // Clean buffers:
  memset(server_message, '\0', sizeof(server_message));
  //memset(client_message, '\0', sizeof(client_message));

  // Create socket:
  socket_desc = socket(PF_INET6, SOCK_DGRAM,  IPPROTO_UDP);// IPPROTO_IPV6 IPPROTO_UDP



  if (socket_desc < 0) {
    printf("Error while creating socket\n");
    return -1;
  }
  // Set port and IP:
  my_addr1.sin6_family = AF_INET6;
  my_addr1.sin6_port = htons(8765);
  inet_pton(AF_INET6, "fd00::03", &my_addr1.sin6_addr);

  if (bind(socket_desc, (struct sockaddr *)&my_addr1, sizeof(my_addr1)) == 0)
  {
    printf("Error while binding socket\n");
    return -1;
  }


  printf("Socket created successfully\n");

  // Set port and IP:
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_port = htons(5678);
  inet_pton(AF_INET6, "fd00::302:304:506:708", &server_addr.sin6_addr);


  // connect to server
  if(connect(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    printf("\n Error : Connect Failed \n");
    exit(11);
  }

  // Send the message to server:
  if (sendto(socket_desc, client_message, strlen(client_message), 0,
             (struct sockaddr*)0, server_struct_length) < 0) {
    printf("Unable to send message\n");
    return -1;
  }

  printf("Waiting for response\n");
  // Receive the server's response:
  if (recvfrom(socket_desc, server_message, sizeof(server_message), 0,
               (struct sockaddr*)0, &server_struct_length) < 0) {
    printf("Error while receiving server's msg\n");
    return -1;
  }

  printf("Server's response: %s\n", server_message);

  // Close the socket:
  close(socket_desc);

  return 0;
}
