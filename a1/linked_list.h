#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

typedef struct Node Node;

struct Node{
    pid_t pid;
    char * path;
    Node * next;
};


Node * add_new_node(Node* head, pid_t new_pid, char * new_path);
Node * delete_node(Node* head, pid_t pid);
int print_if_exists(Node *node, pid_t pid);
void print_nodes(Node *node);
Node * free_all_nodes(Node *head);
int get_length(Node *head);
bool pid_exists(Node *head, pid_t pid);
void kill_all_nodes(Node *head, int SIGNAL);


#endif
