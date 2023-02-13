#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "linked_list.h"

Node* head = NULL;
Node* new_node1 = NULL;
Node* new_node2 = NULL;
Node* new_node3 = NULL;



int main(void) {
	pid_t pid1 = 20303;
    char* path1 = "test/path/to/process";

    pid_t pid2 = 8923;
    char* path2 = "test/path/for/second/process";

    pid_t pid3 = 2;
    char* path3 = "test/path/for/third/process";

    int in_list;

    print_nodes(head);

    printf("Add Node\n");
    head = add_new_node(head, pid1, path1);

    print_nodes(head);

    // printf("head pid %d\n", head -> pid);
    // if (head -> pid == pid1) {
    //     printf("success\n");
    // } else {
    //     printf("failure\n");
    // }

    printf("Add second Node\n");
    head = add_new_node(head, pid2, path2);
    // if (head -> next -> pid == pid2) {
    //     printf("success\n");
    // } else {
    //     printf("failure\n");
    // }

    print_nodes(head);

    printf("Add thrid Node\n");
    head = add_new_node(head, pid3, path3);
    // if (head -> next -> next -> pid == pid3) {
    //     printf("success\n");
    // } else {
    //     printf("failure\n");
    // }

    printf("processInList: %d\n", processInList(head, pid3) == true);
    printf("processInList: %d\n", processInList(head, 27323) == false);

    print_nodes(head);

    head = freeAll(head);

    print_nodes(head);

    in_list = print_if_exists(head, pid1);
    printf("in_list: %d\n", in_list);

    in_list = print_if_exists(head, 92834);
    printf("in_list: %d\n", in_list);

    printf("Delete Node1\n");
    head = delete_node(head, pid1);
    print_nodes(head);

    printf("Delete Node3\n");
    head = delete_node(head, pid3);
    print_nodes(head);

    printf("Delete Node2\n");
    head = delete_node(head, pid2);
    print_nodes(head);

    printf("Delete Node from empty\n");
    head = delete_node(head, pid2);
    print_nodes(head);

    char delim[2] = " ";
    char *list[50];
    char input[100] = "this is a bunch of stuff that...\nI need to tokenize.";
    token_ize(input, list, delim);

    int index = 0;
    while(list[index] != NULL) {
        printf("index%d: %s\n", index, list[index]);
        index++;
    }


    printf("finished\n");
	return 0;
}