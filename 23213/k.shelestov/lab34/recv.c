#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>

#define MAX_CONNECTIONS 125
#define BUFFER_CAPACITY 255

int initialize_address(const char *ip_address, const char *port_number, struct sockaddr_in6 *address) {
    memset(address, 0, sizeof(*address));
    address->sin6_family = AF_INET6;
    address->sin6_port = htons(atoi(port_number));
    return inet_pton(AF_INET6, ip_address, &address->sin6_addr);
}

void send_configuration_message(int socket_fd, uint8_t client_id, uint8_t action_code) {
    char config_message[6];
    config_message[0] = 0x7E;
    config_message[1] = 0;
    config_message[2] = client_id; 
    config_message[3] = action_code; 
    config_message[4] = 0x7E; 
    send(socket_fd, config_message, 5, 0);
}

int create_and_bind_server_socket(struct sockaddr_in6 address) {
    int socket_descriptor = socket(AF_INET6, SOCK_STREAM, 0);
    if (socket_descriptor < 0) {
        perror("socket");
        return -1;
    }
    if (bind(socket_descriptor, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        close(socket_descriptor);
        return -1;
    }
    return socket_descriptor;
}

int encode_client_message(const char *message, ssize_t message_length, char *encoded_message, ssize_t *encoded_length, uint8_t client_id) {
    int index = 0;
    encoded_message[index++] = 0x7E; 
    encoded_message[index++] = client_id;
    for (int i = 0; i < message_length; i++) {
        if (message[i] == 0x7E || message[i] == 0x7D) {
            encoded_message[index++] = 0x7D; 
        }
        encoded_message[index++] = message[i];
    }
    encoded_message[index++] = 0x7E; 
    *encoded_length = index;
    return 0;
}

int decode_received_message(char *message, ssize_t *message_length, const char *encoded_message, ssize_t encoded_length, uint8_t *client_id) {
    *client_id = encoded_message[1];
    int decoded_index = 0;
    int encoded_index = 2;
    while (encoded_index < encoded_length - 1) {
        if (encoded_message[encoded_index] == 0x7D) {
            encoded_index++;
        }
        message[decoded_index++] = encoded_message[encoded_index++];
    }
    *message_length = decoded_index;
    return 0;
}

ssize_t read_encoded_data(int socket_fd, char encoded_message[]) {
    ssize_t total_bytes = 0;
    ssize_t byte = read(socket_fd, encoded_message, 1);
    if (byte == 0) {
        return 0;
    }
    total_bytes++;
    while (1) {
        byte = read(socket_fd, &encoded_message[total_bytes], 1);
        if (byte == 0) {
            break; 
        }
        total_bytes++;
        if (encoded_message[total_bytes - 1] == 0x7D) {
            read(socket_fd, &encoded_message[total_bytes], 1);
            total_bytes++;
            continue;
        }
        if (encoded_message[total_bytes - 1] == 0x7E) {
            break; 
        }
    }
    return total_bytes;
}

int is_ssh_header(const char *buffer, ssize_t length) {
    return (length >= 4 && strncmp(buffer, "SSH-", 4) == 0);
}

void close_all_open_connections(int client_fds[]) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (client_fds[i] != -1) {
            close(client_fds[i]);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "%s transmitter port - transmitter IPv6 address - receiver port - receiver IPv6 address\n", argv[0]);
        return 1;
    }

    struct sockaddr_in6 destination_address;
    struct sockaddr_in6 receiver_address;
    int client_fds[MAX_CONNECTIONS + 2];

    if ((initialize_address(argv[2], argv[1], &receiver_address) < 0) ||
        (initialize_address(argv[4], argv[3], &destination_address) < 0)) {
        fprintf(stderr, "Invalid addresses\n");
        return 1;
    }

    int server_socket = create_and_bind_server_socket(receiver_address);
    
    if (server_socket < 0) {
        fprintf(stderr, "Error during socket initialization and binding\n");
        return 1;
    }
    printf("Receiver successfully bound\n");

    if (listen(server_socket, 20) < 0) {
        perror("Error while listening");
        return 1;
    }
    printf("Receiver is now listening\n");

    int transmitter_socket = accept(server_socket, NULL, NULL);
    if (transmitter_socket < 0) {
        perror("Transmitter connection failed");
        return 1;
    }
    printf("Transmitter connected successfully\n");

    for (int i = 0; i <= MAX_CONNECTIONS; i++) {
        client_fds[i] = -1; 
    }

    while (1) {
        fd_set active_set;
        FD_ZERO(&active_set);
        FD_SET(transmitter_socket, &active_set);
        for (int i = 1; i <= MAX_CONNECTIONS; i++) {
            if (client_fds[i] != -1) {
                FD_SET(client_fds[i], &active_set);
            }
        }

        int activity = select(FD_SETSIZE, &active_set, NULL, NULL, NULL);
        if (activity == -1) {
            perror("Error during select");
            return 1;
        }

        if (FD_ISSET(transmitter_socket, &active_set)) {
            char encoded_message[BUFFER_CAPACITY * 3];
            ssize_t message_length = read_encoded_data(transmitter_socket, encoded_message);
            if (message_length == 0) {
                close_all_open_connections(client_fds);
                return 1;
            }

            char decoded_message[BUFFER_CAPACITY * 3];
            uint8_t client_id;
            decode_received_message(decoded_message, &message_length, encoded_message, message_length, &client_id);
            if (client_id == 0) {
                uint8_t destination_client_id = encoded_message[2];
                printf("Creating client with ID: %hu\n", destination_client_id);
                uint8_t action = encoded_message[3];
                if (action != 1) {
                    printf("Transmitter requested to close connection: %hu\n", destination_client_id);
                    close(client_fds[destination_client_id]);
                    client_fds[destination_client_id] = -1; 
                }
                continue;
            }

            if (is_ssh_header(decoded_message, message_length)) {
                printf("Ignoring SSH header: %.*s\n", (int)message_length, decoded_message);
                continue;
            }

            printf("Message received from transmitter %hu: %.*s\n", client_id, (int)message_length, decoded_message); 
            write(client_fds[client_id], decoded_message, message_length); 
        } else {
            for (int client_id = 1; client_id <= MAX_CONNECTIONS; client_id++) {
                if (client_fds[client_id] != -1 && FD_ISSET(client_fds[client_id], &active_set)) {
                    char buffer[BUFFER_CAPACITY];
                    int client_fd = client_fds[client_id];
                    printf("Message received from client\n");
                    ssize_t message_length = read(client_fd, buffer, BUFFER_CAPACITY);
                    if (message_length == 0) {
                        FD_CLR(client_fd, &active_set);
                        client_fds[client_id] = -1; 
                        printf("Client disconnected\n");
                        send_configuration_message(transmitter_socket, client_id, 2); 
                        close(client_fd);
                        continue;
                    }
                    char encoded_message[BUFFER_CAPACITY * 3];
                    ssize_t encoded_message_length;
                    encode_client_message(buffer, message_length, encoded_message, &encoded_message_length, client_id);
                    write(transmitter_socket, encoded_message, encoded_message_length); 
                }
            }
        }
    }
}

