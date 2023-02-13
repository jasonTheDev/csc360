#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "linked_list.h"

/*
 * Adds a new node to the given list
 * Returns: the new head
 */
Node * add_new_node(Node* head, pid_t new_pid, char * new_path) {
	Node* new_node = malloc( sizeof(*new_node) );

	if (new_node == NULL) {
		perror("Unable to allocate memory\n");
		exit(1);
	}
	new_node -> pid = new_pid;
	new_node -> path = new_path;
	new_node -> next = head;

	return new_node; // the new head
}

/*
 * Deletes a node, identified by the pid, from a given list
 * Returns: the new head
 */
Node * delete_node(Node* head, pid_t pid) {
	Node* current = head;
	Node* prev = NULL;

	while(current != NULL) {
		if(current -> pid == pid){
			if(prev == NULL) {
				head = current -> next;
				free(current);
			} else {
				prev -> next = current -> next;
				free(current);
			}
			break;
		}
		prev = current;
		current = current -> next;
	}
	return head;
}

/*
 * Prints pid and path for all nodes in given list
 * Returns: void
 */
void print_nodes(Node *node) {
	while(node != NULL) {
		printf("%d:\t%s\n", node -> pid, node -> path);
		node = node -> next;
	}
}

/*
 * Gets the length of given list
 * Returns: length
 */
int get_length(Node *head) {
	int count = 0;
	Node *current = head;

	while(current != NULL) {
		count = count + 1;
		current = current -> next;
	}
	return count;
}

/*
 * Checks if node with given pid exists in list
 * Returns: true if exists, false otherwise
 */
bool pid_exists(Node *head, pid_t pid) {
	Node *current = head;
	bool in_list = false;

	while(current != NULL) {
		if(current -> pid == pid) {
			in_list = true;
			break;
		}
		current = current -> next;
	}
	return in_list;
}

/*
 * Frees all allocated memory for given list
 * Returns: the new head
 */
Node * free_all_nodes(Node *node) {
	Node *next = NULL;
	while(node != NULL){
		next = node -> next;
		free(node);
		node = next;
	}
	return NULL;
}

/*
 * Calls kill with given signal for all processes in list
 * Returns: void
 */
void kill_all_nodes(Node *head, int SIGNAL) {
	Node *current = head;
	while(current != NULL) {
		kill(current -> pid, SIGNAL);
		current = current -> next;
	}
}
