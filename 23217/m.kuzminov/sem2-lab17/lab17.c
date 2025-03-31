#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>


typedef struct Node {
    struct Node* next;
    char sentence[BUFSIZ];
}Node;

pthread_mutex_t mutex;
Node* dummy_node;
int size;

void add_in_begining(Node* dummy_node, Node* new_node) {
    pthread_mutex_lock(&mutex);
    new_node->next = dummy_node->next;
    dummy_node->next = new_node;
    size++;
    pthread_mutex_unlock(&mutex);
}

void print_list() {
    pthread_mutex_lock(&mutex);
    Node* current = dummy_node->next;
    printf("Nodes in current state:\n");
    while(current) {
        printf("    %s\n", current->sentence);
        current = current->next;
    }
    pthread_mutex_unlock(&mutex);
}

void* sort(void* param){
    while(1) {
        sleep(5);
        pthread_mutex_lock(&mutex);
        printf("Thread started sort\n");
        if(size == 0 || size == 1) {
            printf("Nothing to sort\n");
            pthread_mutex_unlock(&mutex);
            continue;
        }

        for(int i = 0; i < size - 1; i++) {
            Node* current = dummy_node;
            for(int j = 0; j < size - 1 - i; j++) {
                Node* left = current->next;
                Node* right = left->next;
                if (strcmp(left->sentence, right->sentence) > 0) {
                    left->next = right->next;
                    right->next = left;
                    current->next = right;
                }
                current = current->next;
            }
        }
        printf("sorted\n");
        pthread_mutex_unlock(&mutex);

    }
}

int main(int argc, char const *argv[])
{
    pthread_mutex_init(&mutex, NULL);
    dummy_node = (Node*)malloc(sizeof(Node));
    dummy_node->next = NULL;
    size = 0;
    char buffer[BUFSIZ];
    pthread_t thread;
    
    if (pthread_create(&thread, NULL, sort, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }

    while(1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            perror("fgets");
            exit(1);
        }

        if (buffer[0] == '\n') {
            print_list();
            continue;
        }
        
        size_t n = strcspn(buffer, "\n");
        if (n == 0) {
            continue;
        }
        buffer[n] = '\0';
        Node* new_node = (Node*)malloc(sizeof(Node));
        new_node->next = NULL;
        memcpy(&(new_node->sentence), &buffer, BUFSIZ);
        add_in_begining(dummy_node, new_node);
    }
}

