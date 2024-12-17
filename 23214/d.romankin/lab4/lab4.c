#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct Node {
	char* value;
	struct Node* next;
} Node;


Node* add(Node* nodeList, char* string) {
	Node* newNode = (Node*)malloc(sizeof(Node));
	if (newNode == NULL) {
		perror("node malloc error");
		exit(EXIT_FAILURE);
	}
	int len = strlen(string) + 1;
	newNode->value = (char*)malloc((len));
	
	if (newNode->value == NULL) {
		perror("value malloc error");
		exit(EXIT_FAILURE);
	}
	
	strncpy(newNode->value, string, len);
	
	if (nodeList == NULL) {
		return newNode;
	}
	Node* curNode = nodeList;
	while (curNode->next != NULL) {
		
		curNode = curNode->next;
	}
	
	curNode->next = newNode;
	return nodeList;
}

void freeNodes(Node* nodeList){
	Node* curNode = nodeList;
	while (curNode != NULL) {
		Node* nextNode = curNode->next;
		free(curNode->value);
		free(curNode);
		curNode = nextNode;
	}
}

void printNodes(Node* nodeList){
	Node* curNode = nodeList;
	while (curNode != NULL) {
		
		printf("%s", curNode->value);
		curNode = curNode->next;
	}
}

char buffer[5];
int flag = 0;
int main() {
	
	Node* nodeList = NULL;
	
	while(1) {
		if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
			if (ferror(stdin)) {
				exit(EXIT_FAILURE);
			}
			if (feof(stdin)) {
				break;
			}
			
		}
		
		if (buffer[0] == '.' && !flag) {
			break;
		}

		if (buffer[strlen(buffer) - 1] != '\n') {
			flag = 1;
		}
		else {
			flag = 0;
		}
		nodeList = add(nodeList, buffer);
	}
	printNodes(nodeList);
	freeNodes(nodeList);
	exit(EXIT_SUCCESS);
}
