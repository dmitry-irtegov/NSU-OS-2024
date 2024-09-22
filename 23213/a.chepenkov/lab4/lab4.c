#define _CRT_SECURE_NO_WARNINGS
#define BUFFER_SIZE 5

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Node {
	char* str;
	struct Node* next;
} Node;

Node* addNode(char* str, Node* previos_node) {
	Node* node = (Node*)malloc(sizeof(Node));
	if (node == NULL) {
		fprintf(stderr, "Failed to malloc new node\n");
	}

	node->str = (char*)malloc(sizeof(char) * (BUFFER_SIZE+1));
	if (str == NULL) {
		fprintf(stderr, "Failed to malloc space for string\n");
	}

	strcpy(node->str, str);
	node->next = NULL;
	if (previos_node != NULL) {
		previos_node->next = node;
	}

	return node;
}

void freeList(Node* head) {
	Node* current_node = head;
	Node* next_node;
	while (current_node != NULL) {
		next_node = current_node->next;
		free(current_node->str);
		free(current_node);
		current_node = next_node;

	}
}

int main() {
	char buffer[BUFFER_SIZE];
	Node* current_node = NULL;
	Node* head = NULL;
	int flag = 0;
	while (1) {
		if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
			break;
		}

		if (buffer[0] == '.' && flag == 0) {
			break;
		}

		if (buffer[strlen(buffer)-1] != '\n') {
			flag = 1;
		}
		else {
			flag = 0;
		}

		current_node = addNode(buffer, current_node);
		if (head == NULL) {
			head = current_node;
		}
	}

	current_node = head;
	while (current_node != NULL) {
		printf("%s", current_node->str);
		current_node = current_node->next;
	}

	freeList(head);

	return 0;
}
