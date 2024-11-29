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
#define BUFFER_SIZE 255

int initialize_address(const char *ip_address, const char *port_number, struct sockaddr_in6 *address) {
    memset(address, 0, sizeof(*address));
    address->sin6_family = AF_INET6;
    address->sin6_port = htons(atoi(port_number));
    return inet_pton(AF_INET6, ip_address, &address->sin6_addr);
}

void send_configuration_message(int socket_fd, uint8_t client_identifier, uint8_t action_code) {
    char config_message[6];
    config_message[0] = 0x7E; 
    config_message[1] = 0; 
    config_message[2] = client_identifier; 
    config_message[3] = action_code; 
    config_message[4] = 0x7E; 
    send(socket_fd, config_message, 5, 0);
}

int create_and_bind_server_socket(struct sockaddr_in6 address) {
    int server_socket = socket(AF_INET6, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        return -1;
    }
    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        close(server_socket);
        return -1;
    }
    return server_socket;
}

int encode_client_message(const char *message, ssize_t message_length, char *encoded_message, ssize_t *encoded_length, uint8_t client_identifier) {
    int index = 0;
    encoded_message[index++] = 0x7E; 
    encoded_message[index++] = client_identifier; 
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

int decode_received_message(char *message, ssize_t *message_length, const char *encoded_message, ssize_t encoded_length, uint8_t *client_identifier) {
    *client_identifier = encoded_message[1];
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

    struct sockaddr_in6 receiver_address;
    struct sockaddr_in6 transmitter_address;
    int client_fds[MAX_CONNECTIONS + 2];

    if ((initialize_address(argv[2], argv[1], &transmitter_address) < 0) ||
        (initialize_address(argv[4], argv[3], &receiver_address) < 0)) {
        fprintf(stderr, "Invalid addresses\n");
        return 1;
    }

    int server_socket = create_and_bind_server_socket(transmitter_address);
    if (server_socket < 0) {
        fprintf(stderr, "Failed to bind socket\n");
        return 1;
    }
    printf("Transmitter successfully bound\n");

    if (listen(server_socket, MAX_CONNECTIONS) < 0) {
        perror("Error while listening");
        return 1;
    }
    printf("Transmitter is now listening\n");

    int receiver_socket = socket(AF_INET6, SOCK_STREAM, 0);
    if (connect(receiver_socket, (const struct sockaddr *)&receiver_address, sizeof(receiver_address)) < 0) {
        perror("Error connecting to receiver");
        return 1;
    }
    printf("Connected to receiver successfully\n");


    memset(client_fds, -1, sizeof(client_fds));
    uint8_t active_clients_count = 0;

    while (1) {
        fd_set active_set;
        FD_ZERO(&active_set);
        FD_SET(server_socket, &active_set);
        FD_SET(receiver_socket, &active_set);
        for (int i = 1; i <= active_clients_count; i++) {
            if (client_fds[i] != -1) {
                FD_SET(client_fds[i], &active_set);
            }
        }

        int activity = select(FD_SETSIZE, &active_set, NULL, NULL, NULL);
        if (activity == -1) {
            perror("Error during select");
            continue; 
        }

        if (FD_ISSET(server_socket, &active_set)) {
            int new_client_socket = accept(server_socket, NULL, NULL);
            if (new_client_socket < 0) {
                perror("Error accepting new client");
                continue; 
            }
            printf("New client connected\n");

            if (active_clients_count < MAX_CONNECTIONS) {
                for (int i = 1; i <= MAX_CONNECTIONS; i++) {
                    if (client_fds[i] == -1) {
                        client_fds[i] = new_client_socket;
                        active_clients_count++;
                        send_configuration_message(receiver_socket, i, 1);
                        break;
                    }
                }
            } else {
                fprintf(stderr, "Maximum number of clients reached\n");
                close(new_client_socket);
            }
        }

        if (FD_ISSET(receiver_socket, &active_set)) {
            char encoded_message[BUFFER_SIZE * 3];
            ssize_t message_length = read_encoded_data(receiver_socket, encoded_message);
            if (message_length == 0) {
                close_all_open_connections(client_fds);
                close(receiver_socket);
                return 0;
            }
            char decoded_message[BUFFER_SIZE * 3];
            uint8_t client_identifier;
            decode_received_message(decoded_message, &message_length, encoded_message, message_length, &client_identifier);
            if (client_identifier == 0) {
                uint8_t client_to_remove = encoded_message[2];
                close(client_fds[client_to_remove]);
                client_fds[client_to_remove] = -1;
                active_clients_count--; 
                continue;
            }
            printf("Forwarding message to client %hu\n", client_identifier);
            write(client_fds[client_identifier], decoded_message, message_length);
        } else {
            for (uint8_t i = 1; i <= active_clients_count; i++) {
                if (client_fds[i] != -1 && FD_ISSET(client_fds[i], &active_set)) {
                    char buffer[BUFFER_SIZE] = {0};
                    ssize_t message_length = read(client_fds[i], buffer, BUFFER_SIZE);
                    if (message_length <= 0) {
                        printf("Client disconnected\n");
                        send_configuration_message(receiver_socket, i, 2);
                        close(client_fds[i]);
                        client_fds[i] = -1;
                        active_clients_count--; 
                        continue;
                    }
                    char encoded_message[BUFFER_SIZE * 5] = {0};
                    ssize_t encoded_message_length;
                    encode_client_message(buffer, message_length, encoded_message, &encoded_message_length, i);
                    write(receiver_socket, encoded_message, encoded_message_length);
                }
            }
        }
    }
}


