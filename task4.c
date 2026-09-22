#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define SIZE 256

typedef struct Node {
    char *string;
    struct Node *next_node;
}Node;

void popNode(Node **tail, Node **head, char *buf) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    int len_buf = strlen(buf) + 1;
    new_node->string = (char *)malloc(len_buf);
    new_node->next_node = NULL;

    memcpy(new_node->string, buf, len_buf);

    if (*tail == NULL) {
        *head = new_node;
        *tail = new_node;
    }
    else {
        (*tail)->next_node = new_node;
        *tail = new_node;
    }
}

void printNodes(Node *head) {
    while (head != NULL) {
        printf("%s", head->string);
        head = head->next_node;
    }
}

void freeNodes(Node *head) {
    while (head != NULL) {
        Node *cur = head->next_node;
        free(head->string);
        free(head);
        head = cur;
    }
}

int main() {
    Node *head = NULL;
    Node *tail = NULL;

    char buf[SIZE];
    while (fgets(buf, SIZE, stdin) != NULL) {
        if (buf[0] == '.') break;

        popNode(&tail, &head, buf);
    } 

    printNodes(head);
    freeNodes(head);

    return 0;
}