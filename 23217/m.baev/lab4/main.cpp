#include <iostream>

using namespace std;

typedef struct
{
    char **list;
    int size;
    int capacity;
} string_list;


string_list* init_string_list() {
    
    string_list* list = (string_list *) malloc(sizeof(string_list));
    list->size = 0;
    list->capacity = 0;
    list->list = NULL;

    return list;
}


int add_string(string_list *list, const char *string) {

    char *new_string = (char*) malloc(strlen(string) + 1);
    strcpy(new_string, string);

    if (list->size == list->capacity) {
        if (list->capacity == 0) {
            list->list = (char **) malloc(sizeof(char *));
            list->capacity = 1;
        }

        else {
            list->list = (char **) realloc(list->list, sizeof(char *) * list->capacity * 2);
            list->capacity *= 2;
        }
    }

    list->list[list->size++] = new_string;

    return 0;
}


int print_all_string(string_list *list) {
    
    for (int i = 0; i < list->size; i ++) {
        cout << list->list[i];
    }

    return 0;
}


int free_string_list(string_list *list) {
    for (int i = 0; i < list->size; i++) {
        free(list->list[i]);
    }
    free(list->list);
    free(list);
    return 0;
}


int main() {

    char string[100];
    string_list *list = init_string_list();

    while (true) {
        fgets(string, sizeof(string), stdin);

        if (string[0] == '.') {
            break;
        }

        add_string(list, string);
    }

    print_all_string(list);
    
    free_string_list(list);
    return 0;
}