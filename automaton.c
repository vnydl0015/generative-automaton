/* Program to generate text based on the context provided by input prompts.

  Skeleton program written by Artem Polyvyanyy, http://polyvyanyy.com/,
  September 2023, with the intention that it be modified by me
  to add functionality, as required by the assignment specification.
  All included code is (c) Copyright University of Melbourne, 2023.

*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define SDELIM "==STAGE %d============================\n" 
#define MDELIM "-------------------------------------\n"  
#define THEEND "==THE END============================\n"
#define NOSFMT "Number of statements: %d\n" 
#define NOCFMT "Number of characters: %d\n"
#define NPSFMT "Number of states: %d\n"
#define TFQFMT "Total frequency: %d\n"

#define CRTRNC          '\r'      /* carriage return character */
#define NEWLIN          '\n'      /* newline character */
#define NUL_CH          '\0'      /* null character */
#define ELLIPSE         '.'       /* ellipse character */

#define STAGE_0          0        /* stage number 0 */
#define STAGE_1          1        /* stage number 1 */
#define STAGE_2          2        /* stage number 2 */
#define STMNT_END        1        /* end of statement */
#define STAGE_END        2        /* end of stage */

#define INT_ZER          0        /* integer 0 */
#define INT_ONE          1        /* integer one */
#define INT_TWO          2        /* integer two */
#define INT_THR          3        /* integer three */
#define INT_TEN          10       /* integer ten */
#define FALSE            0        /* integer 0 for FALSE */
#define TRUE             1        /* integer 1 for TRUE */
#define LEFT             1        /* integer 1 for left */
#define RIGHT            2        /* integer 2 for right */

#define OUTPUT_MAX       37       /* max number of output characters */
#define ASCII_MAX        128      /* max range for ASCII character */

/* Data structure to record information about automaton ***********************/
typedef struct state state_t;     /* a state for each node */
typedef struct node  node_t;      /* a node in a linked list */

struct node {
    char*           str;          /* a transition string */
    node_t*         down;         /* a link to node below */
    node_t*         right;        /* a link to node on right */
    node_t*         up;           /* a link to node above */
    node_t*         left;         /* a link to node on left */
    state_t*        state;        /* a state of this node */
};

typedef struct { 
    node_t*         head;         /* a pointer to root node */    
    node_t*         tail;         /* a pointer to latest node */
} list_t;

struct state {
    int             freq;         /* frequency of node */
    int             visited;      /* visited state of node */
};

typedef struct {
    int             state;        /* total state in automaton */
    int             freq;         /* total frequency in automaton */ 
    int             statement;    /* total statement in automaton */
    int             character;    /* total character in automaton */
} total_t;

typedef struct {       
    list_t*         outputs;      /* a list of output nodes */
    total_t*        total;        /* state of automaton */         
} automaton_t;  

/* Function prototypes ********************************************************/
node_t *get_new_node(void);
list_t *get_new_list(void);
state_t *get_new_state(void);
total_t *get_new_totals(void);
char *get_string(char *c);
char *combine_str(char *p1, char *p2);
int mygetchar(void); 
int get_num_compress(void);
int find_matching_char(automaton_t *automaton, node_t *curr, char c, int*index);
int find_x_node_left(automaton_t *automaton, node_t *curr, int *highest_freq);
int find_x_node_right(automaton_t *automaton, node_t *curr, int *highest_freq, 
                    char *highest_ascii);
automaton_t *get_new_automaton(void);
automaton_t *construct_automaton(void);
automaton_t *add_root(automaton_t *automaton);  
automaton_t *compress_automaton(automaton_t *automaton);
automaton_t *insert_vertically(automaton_t *automaton, char c);
automaton_t *insert_horizontally(automaton_t *automaton, char c, 
                                int *compare_root, int *invert_vertical);
node_t *traverse_automaton(node_t *x_node, int direction, int *new);
void process_stage_0(automaton_t *automaton);
void process_prompt(automaton_t *automaton, int stage_num);
void print_prefix(automaton_t *automaton, char c, int *char_count,
                    int *first_input, int *terminate, int *index);
void print_suffix(automaton_t *automaton, int *char_count, int *index);
void print_stage_2_header(automaton_t *automaton);
void insert_equal_result(automaton_t *automaton, node_t *curr, node_t *new);
void insert_unequal_result(automaton_t *automaton, int result, node_t *curr, 
                        node_t *new, int *compare_root, int *insert_vertical);
void insert_inbetween_left(node_t *new_node, node_t *curr_node);
void insert_inbetween_right(node_t *new_node, node_t *curr_node);
void delete_node(automaton_t *automaton, node_t *x_node);
void check_visited(node_t *node, int *possible_visit);
void free_automaton(automaton_t *automaton);
void free_node(node_t *p1, int free_surround);
void null_surrounding_nodes(node_t *p1);
void print_char(node_t *node, int *char_count, int index);
void print_ellipses(int *char_count);   
static void recursive_free_left(automaton_t *automaton, node_t *root);
static void recursive_free_right(automaton_t *automaton, node_t *root);

/* Main program controls all the action ***************************************/
int main(int argc, char *argv[]) {
    automaton_t *automaton = construct_automaton();
    process_stage_0(automaton);
    process_prompt(automaton, STAGE_1);
    automaton = compress_automaton(automaton);
    print_stage_2_header(automaton); 
    process_prompt(automaton, STAGE_2);
    printf(THEEND);
    free_automaton(automaton);
    return EXIT_SUCCESS; 
}

/* Functions that trigger each stages *****************************************/
/* Read input from STDIN, skip carriage return */
int mygetchar() {
    int c;
    static int previously_newline = FALSE, stage_num = 0;
    while ((c = getchar()) == CRTRNC) {  
    }
    /* Keep track of stage changes based on input file format */
    if (c == NEWLIN && previously_newline) {
        stage_num++;
        return STAGE_END;
    } else if (c == NEWLIN) {
        previously_newline = TRUE;
        return STMNT_END;
    } else {
        previously_newline = FALSE;
    }
    /* Ensure characters are within ASCII range and a valid input file format */
    if (((c != EOF) && (c < INT_ZER || c >= ASCII_MAX))
       || (c == EOF && (stage_num != STAGE_2))) { 
        printf("Invalid test file, program terminated\n");
        exit(EXIT_FAILURE);
    } 
    return c;
}

/* Build automaton using input statements in stage 0 */
automaton_t *construct_automaton(void) {
    automaton_t *automaton = get_new_automaton();  
    int c, insert_vertical = TRUE, compare_root = TRUE;

    while ((c = mygetchar()) != STAGE_END && c != EOF) {
        if (c != STMNT_END) {
            /* Insert new nodes vertically if its string is not in automaton */
            if (insert_vertical) {
                automaton = insert_vertically(automaton, c);
            } else {
                /* Insert horizonally if the same transition string is found */
                automaton = insert_horizontally(automaton, c, 
                                &compare_root, &insert_vertical);
            }   
            automaton->total->character++;  
        } else {
            /* Reset variables for next new statement */
            insert_vertical = FALSE;  
            compare_root = TRUE;
            automaton->total->statement++; 
            automaton->total->freq++;          
        }    
    } 
    return automaton;
}

/* Print all information in stage 0 */
void process_stage_0(automaton_t *automaton) {
    assert(automaton);  
    printf(SDELIM, INT_ZER);
    printf(NOSFMT, automaton->total->statement);
    printf(NOCFMT, automaton->total->character);
    printf(NPSFMT, automaton->total->state);
}

/* Calling functions to print output strings in stages 1 and 2 */
void process_prompt(automaton_t *automaton, int stage_num) {
    assert(automaton);  
    if (stage_num == STAGE_1) printf(SDELIM, STAGE_1);
    
    int c, char_count = 0, index = 0;
    int first_input = TRUE, terminate = FALSE;
    /* Ensure last 2 characters of test file are not '\n' followed by EOF */
    int previously_newline = FALSE;

    while ((c = mygetchar()) != STAGE_END) { 
        /* Add suffix as provided in automaton to a given input prompt, 
        following newline character */
        if (c == STMNT_END || c == EOF) {
            if (!terminate && !previously_newline) {
                print_suffix(automaton, &char_count, &index);
            }
            if (c == EOF) return;
            char_count = index = 0;
            first_input = TRUE;
            terminate = FALSE; 
            previously_newline = TRUE;
        /* Print input prompts (prefix) after every newline */
        } else if (!terminate && char_count < OUTPUT_MAX) {
            previously_newline = FALSE;
            print_prefix(automaton, c, &char_count, &first_input, &terminate,
                         &index);
        }
    }
}

/* Print all information in stage 2 */
void print_stage_2_header(automaton_t *automaton) {
    assert(automaton);
    printf(SDELIM, STAGE_2);
    printf(NPSFMT, automaton->total->state);
    printf(TFQFMT, automaton->total->freq);
    printf(MDELIM);
}

/* Functions to get new memory spaces *****************************************/
/* Create new node */
node_t *get_new_node(void) {
    node_t *new = (node_t *)malloc(sizeof(*new));
    assert(new);
    new->right = new->down = new->up = new->left = NULL;
    new->str = NULL;
    new->state = get_new_state();   
    return new;
}

/* Create new state */
state_t *get_new_state(void) {
    state_t *new = (state_t *)malloc(sizeof(*new));
    assert(new);
    new->visited = new->freq = INT_ZER;
    return new;
}

/* Create new automaton */
automaton_t *get_new_automaton(void) {
    automaton_t *automaton = (automaton_t *)malloc(sizeof(*automaton));
    assert(automaton);
    automaton->outputs = get_new_list();
    automaton->total = get_new_totals(); 
    return automaton;
}

/* Create new list */
list_t *get_new_list(void) {
    list_t *new = (list_t *)malloc(sizeof(*new));
    assert(new);
    new->head = new->tail = NULL;
    return new;
}

/* Create new totals */
total_t *get_new_totals(void) {
    total_t *new = (total_t *)malloc(sizeof(*new));
    assert(new);
    new->statement = new->freq = new->character = INT_ZER;
    new->state = INT_ONE;
    return new;
}

/* Convert single character to a string */
char *get_string(char *c) {
    char *str = (char *)malloc(INT_TWO * (sizeof(char)));
    assert(str);
    str[INT_ZER] = *c;
    str[INT_ONE] = NUL_CH;
    return str;
}

/* Combine two strings to a new memory space, re-order string1 before string2 */
char *combine_str(char *p1, char *p2) {
    char *str = (char *)malloc((strlen(p1) + strlen(p2) + 1) * sizeof(char));
    strcpy(str, p1);
    strcat(str, p2);
    free(p2);
    return str;
}

/* Insertion functions *******************************************************/
/* Insert new node directly below previous node pointed by tail pointer */
automaton_t *insert_vertically(automaton_t *automaton, char c) {
    assert(automaton);
    node_t *newnode = get_new_node();
    newnode->str = get_string(&c);
    automaton->total->state++; 
    /* Update frequency of previous node if traversed pass */
    if (automaton->outputs->tail) {
        automaton->outputs->tail->state->freq++;
        automaton->total->freq++;
    } 
    /* Assign tail pointer to new node. If root node, also assign head */ 
    if (!automaton->outputs->head) {
        automaton->outputs->head = automaton->outputs->tail = newnode;
    } else {
        automaton->outputs->tail->down = newnode;
        newnode->up = automaton->outputs->tail;
        automaton->outputs->tail = newnode;
    } 
    return automaton;
}

/* Insert new node to left or right of previous node pointed by tail pointer */
automaton_t *insert_horizontally(automaton_t *automaton, char c, 
                                   int *compare_root, int *invert_vertical) {
    assert(automaton);
    node_t *new_node = get_new_node(), *curr_node;
    new_node->str = get_string(&c);
    automaton->total->state++; 
    
    /* Reassign curr_node to root node for every new statement */
    if (*compare_root) {
        curr_node = automaton->outputs->head;
    } else {
        /* Ensure valid addition of new node below leaf node */
        if (!automaton->outputs->tail->down) {
            automaton->outputs->tail->down = new_node;
            new_node->up = automaton->outputs->tail;
            automaton->outputs->tail = new_node;
            /* From now on, insert vertically */
            *invert_vertical = TRUE;
            return automaton;
        } 
        curr_node = automaton->outputs->tail->down; 
    } 
    
    /* During processing of each prompt, update freq of previous node */
    if (curr_node != automaton->outputs->head) {
        automaton->outputs->tail->state->freq++;
        automaton->total->freq++;
    } 
    int result = strcmp(new_node->str, curr_node->str);
    /* If equal, reassign tail to current node */
    if (result == 0) {
        insert_equal_result(automaton, curr_node, new_node);
        *compare_root = FALSE;
    } else {
        /* Insert new node in ascending ASCII order for left-hand side */
        if (result < 0) {
            insert_unequal_result(automaton, LEFT, curr_node, new_node, 
                                  compare_root, invert_vertical);
        /* Insert new node in descending ASCII order for right-hand side */
        } else if (result > 0) {
            insert_unequal_result(automaton, RIGHT, curr_node, new_node, 
                                  compare_root, invert_vertical);
        }
    }
    return automaton;
}

/* Remove any already existing state for equal result */
void insert_equal_result(automaton_t *automaton, node_t *curr, node_t *new) {
    assert(automaton && curr && new);
    automaton->total->state--; 
    automaton->outputs->tail = curr; 
    free_node(new, INT_ZER);
}

/* Compare all nodes for further equal result, otherwise insert left or right */
void insert_unequal_result(automaton_t *automaton, int result, node_t *curr, 
                         node_t *new, int *compare_root, int *insert_vertical) {
    while (curr) {
        /* Insert new node to the edge of linked list (left/right) */
        if ((result == LEFT && !curr->left) || 
            (result == RIGHT && !curr->right)){
            if (result == LEFT) {
                new->right = curr;
                curr->left = new; 
            } else {
                new->left = curr;
                curr->right = new;
            }
            automaton->outputs->tail = new;
            *insert_vertical = TRUE;
            return;
        }
        /* Search through all left/right nodes */
        if (result == LEFT) curr = curr->left;
        if (result == RIGHT) curr = curr->right;

        /* If the same string is found, reassign tail */
        int new_result = strcmp(new->str, curr->str);
        if (new_result == 0) {
            insert_equal_result(automaton, curr, new);
            *compare_root = FALSE;
            return;

        /* If insertion in-between nodes is possible due to ASCII ordering */
        } else if (new_result > 0) {
            if (result == LEFT) insert_inbetween_left(new, curr);
            if (result == RIGHT) insert_inbetween_right(new, curr);
            automaton->outputs->tail = new;
            *insert_vertical = TRUE;
            return;
        }
    } 
}

/* Insert new node in-between 2 existing nodes (left side) */
void insert_inbetween_left(node_t *new_node, node_t *curr_node) {
    assert(new_node && curr_node);
    new_node->left = curr_node; 
    new_node->right = curr_node->right; 
    curr_node->right->left = new_node;
    curr_node->right = new_node; 
}

/* Insert new node in-between 2 existing nodes (right side) */
void insert_inbetween_right(node_t *new_node, node_t *curr_node) {
    assert(new_node && curr_node);
    new_node->left = curr_node->left; 
    new_node->right = curr_node; 
    curr_node->left->right = new_node;
    curr_node->left = new_node; 
}

/* Printing functions ********************************************************/
/* Process first-half of stages 1 and 2 input prompts and print to STDOUT */
void print_prefix(automaton_t *automaton, char c, int *char_count,
                    int *first_input, int *terminate, int *index) {
    node_t *curr_node;
    static int str_len = 0; 
    putchar(c);
    (*char_count)++;

    /* Reset curr_node to head for every new input prompt */
    if (*first_input) {
        curr_node = automaton->outputs->head;
        *first_input = FALSE;
        *index = 0;
        str_len = strlen(curr_node->str);
    } else {
        /* If leaf node is reached, terminate the searching */
        if (!automaton->outputs->tail->down && 
            !automaton->outputs->tail->str[*index]) {   
            *terminate = TRUE;
            *index = str_len = 0;
            print_ellipses(char_count);
            putchar(NEWLIN);
            return;
        /* If the entire transition string has been searched, reassign tail */
        } else if (*index >= str_len) {
            curr_node = automaton->outputs->tail->down;
            str_len = strlen(curr_node->str);
            *index = 0;
        /* Otherwise, search through the same string of previous node */
        } else {
            curr_node = automaton->outputs->tail;
        }
    }
    /* Handle unmatched character */
    if (!find_matching_char(automaton, curr_node, c, index)) {
        *terminate = TRUE;
        print_ellipses(char_count);
        putchar(NEWLIN);
    }
}

/* Compare character using string indexing then search for same left or right */
int find_matching_char(automaton_t *automaton, node_t *curr, char c, int*index){
    int fixed = FALSE;
    while (curr) {
        /* If character matches, record its index */
        if (c == curr->str[*index]) {
            automaton->outputs->tail = curr;
            (*index)++;
            return TRUE;           
        /* Otherwise, loop through one side only (fixed) */
        } else if (c < curr->str[*index] && !fixed) {
            fixed = LEFT;
        } else if (c > curr->str[*index] && !fixed) {
            fixed = RIGHT;
        }
        if (fixed == LEFT) curr = curr->left;
        if (fixed == RIGHT) curr = curr->right;
    } 
    return FALSE;
}

/* Process second-half of stages 1 and 2 input prompts and print to STDOUT */
void print_suffix(automaton_t *automaton, int *char_count, int *index) {
    assert(automaton);
    print_ellipses(char_count);

    /* If string of node pointed by tail was not printed out completely */
    int remain = FALSE;
    if (*index < ((int)strlen(automaton->outputs->tail->str))) remain = TRUE;
    /* Assign current node to either the node at same level or below it */
    node_t *curr_node;
    if (remain) curr_node = automaton->outputs->tail;
    if (!remain) curr_node = automaton->outputs->tail->down;

    /* Search for node with higher freq. If equal, search for higher ASCII */
    while (*char_count < OUTPUT_MAX && curr_node) {
        int highest_freq = curr_node->state->freq;
        char *highest_ascii = curr_node->str;

        int left = find_x_node_left(automaton, curr_node, &highest_freq);
        int right = find_x_node_right(automaton, curr_node, &highest_freq, 
                                      highest_ascii);
        if (!left && !right) automaton->outputs->tail = curr_node;

        /* As pointed by tail, if remain, print only remaining suffix */
        int starting_index = INT_ZER;
        if (remain) {
            starting_index = *index;
            remain = FALSE;
        } 
        /* Otherwise, print entire string */
        print_char(automaton->outputs->tail, char_count, starting_index);
        curr_node = automaton->outputs->tail->down;
    }
    putchar(NEWLIN);
}

/* Loop through left nodes to find nodes with highest freq only */
int find_x_node_left(automaton_t *automaton, node_t *curr, int *highest_freq) {
    node_t *left_node = curr->left;   
    int left = FALSE;
    while (left_node) {
        if (left_node->state->freq > *highest_freq) {
            automaton->outputs->tail = left_node;
            *highest_freq = left_node->state->freq;
            left = TRUE;
        }
        left_node = left_node->left;
    }
    return left;
}

/* Loop through right nodes to find nodes with highest freq and ascii */
int find_x_node_right(automaton_t *automaton, node_t *curr, int *highest_freq, 
                      char *highest_ascii) {
    node_t *right_node = curr->right;
    int right = FALSE;
    while (right_node) {
        if ((right_node->state->freq > *highest_freq) || 
            (right_node->state->freq == *highest_freq && 
                strcmp(right_node->str, highest_ascii) > 0)) {
            automaton->outputs->tail = right_node;
            *highest_freq = right_node->state->freq;
            highest_ascii = right_node->str;
            right = TRUE;
        }
        right_node = right_node->right;
    }
    return right;
}

/* print ellipses under 37 character limit*/
void print_ellipses(int *char_count) {
    for (int i = 0; *char_count < OUTPUT_MAX && i < INT_THR; i++, 
         (*char_count)++) {
        putchar(ELLIPSE);
    }
}

/* print characters under 37 character limit */
void print_char(node_t *node, int *char_count, int index) {
    int str_len = strlen(node->str);
    for (; index < str_len && *char_count < OUTPUT_MAX; index++, 
         (*char_count)++) {
        putchar(node->str[index]);
    }
}

/* Compression functions *****************************************************/
/* Compress automaton for num_compress times */
automaton_t *compress_automaton(automaton_t *automaton) {
    assert(automaton);
    int num_compress = get_num_compress();
    automaton = add_root(automaton);

    /* Search for node to compress based on lower ASCII order */
    for (int i = 0; i < num_compress; i++) {
        /* Reallocate tail to root node after each compression */
        node_t *x_node = automaton->outputs->head;
        while (!x_node->state->visited) {
            /* Perform compression if all conditions are met */
            if (x_node->down && !x_node->down->left && !x_node->down->right 
                && x_node->down->down) {
                delete_node(automaton, x_node);
                break;
            } 
            int new_left = FALSE, new_right = FALSE, new_down = FALSE;
            /* Otherwise, search left side first due to ascii ordering */
            if (x_node->down && x_node->down->left && 
                !x_node->down->left->state->visited) {
                x_node = traverse_automaton(x_node, LEFT, &new_left);
            } 
            /* If no potential x_node on left side, search downwards */ 
            if (!new_left && x_node->down && !x_node->down->state->visited) {
                check_visited(x_node->down, &new_down);
                if (new_down) x_node = x_node->down;
            }
            /* If still no potential x_node, search right side */ 
            if (!new_down && !new_left && x_node->down->right && 
                !x_node->down->right->state->visited) {  
                x_node = traverse_automaton(x_node, RIGHT, &new_right);  
            }
            /* If leaf node is reached but no compression has occurred, block 
            entire branch from future visit */
            if (!new_down && !new_left && !new_right) { 
                x_node->state->visited = TRUE;
                x_node = automaton->outputs->head;
            }
        }
    }
    automaton->outputs->head = automaton->outputs->head->down;
    return automaton;
}

/* Get number of compression for stage 2 using string array */
int get_num_compress(void) {
    int c, str_len = 0;   
    char num_compress_str[INT_TEN];  
    
    while ((c = mygetchar()) != STMNT_END) {
        if (c == EOF) exit(EXIT_FAILURE);
        num_compress_str[str_len++] = c; 
    }
    num_compress_str[str_len] = NUL_CH;
    return atoi(num_compress_str);
}

/* Add a string-less root node at the top of automaton */
automaton_t *add_root(automaton_t *automaton) {
    assert(automaton);
    node_t *newnode = get_new_node();
    newnode->state->freq = automaton->total->statement;
    newnode->down = automaton->outputs->head;
    automaton->outputs->head->up = newnode;
    automaton->outputs->head = newnode;
    return automaton;
}

/* Delete 'y' node */
void delete_node(automaton_t *automaton, node_t *x_node) {
    node_t *y_node = x_node->down;
    
    /* Combine y's string with strings of its outgoing arcs on left side */ 
    node_t *left_node = y_node->down->left;
    while (left_node) {
        left_node->str = combine_str(y_node->str, left_node->str); 
        left_node = left_node->left;
    }  
    /* Combine y's string with strings of its outgoing arcs on right side */
    node_t *right_node = y_node->down->right;
    while (right_node) {
        right_node->str = combine_str(y_node->str, right_node->str);
        right_node = right_node->right;
    }                
    /* Reassign pointers after deleting 'y' node */
    y_node->down->str = combine_str(y_node->str, y_node->down->str);
    y_node->down->up = x_node;
    x_node->down = y_node->down;
    automaton->total->freq -= y_node->state->freq;
    automaton->total->state--;
    free_node(y_node, INT_ZER);
    check_visited(x_node, NULL);
}

/* Check for future possible and impossible visits */
void check_visited(node_t *node, int *possible_visit) { 
    /* Already visited node won't be traversed pass again */
    if (!node->down) {
        node->state->visited = TRUE; 
    } else if (!node->down->down) {
        /* Visit is still possible if current node has a side-way node */
        if ((node->down->left && !node->down->left->state->visited) || 
           (node->down->right && !node->down->right->state->visited)) {
            if (possible_visit) *possible_visit = TRUE;
        } else {
            node->state->visited = TRUE; 
        } 
        node->down->state->visited = TRUE;
    /* Visit is still possible if current node has a below node */
    } else {
        if (possible_visit) *possible_visit = TRUE;
    }
}

/* Check for potential visit/compression while traversing automaton */
node_t *traverse_automaton(node_t *x_node, int direction, int *new_node) {
    node_t *curr_node = NULL;

    /* Loop through un-visited nodes in either left or right direction 
    to find node with smallest string in terms of ascii order */
    if (direction == LEFT) {
        curr_node = x_node->down->left;
        while (curr_node->left && !curr_node->left->state->visited){
            curr_node = curr_node->left;
        }
    } else if (direction == RIGHT) {
        curr_node = x_node->down->right;
        while (curr_node->right && !curr_node->right->state->visited){
            curr_node = curr_node->right;
        }
    }
    check_visited(curr_node, new_node);

    /* Assign x_node to potential current node */
    if (*new_node) x_node = curr_node;
    /* If all current nodes has been visited, traverse to opposite side */
    while (!(*new_node)) {
        if (direction == LEFT) curr_node = curr_node->right;
        if (direction == RIGHT) curr_node = curr_node->left;
        /* If still no potential x_node found, break */
        if (curr_node == x_node->down) break;
        check_visited(curr_node, new_node);
    }
    return x_node;
}

/* Freeing memory space functions ********************************************/
/* Free automaton */
void free_automaton(automaton_t *automaton) {
    assert(automaton);
    automaton->outputs->head = automaton->outputs->head->up;
    while (automaton->outputs->head) {
        recursive_free_left(automaton, automaton->outputs->head);
    }
    free(automaton->total);
    free(automaton->outputs);
    free(automaton);
}

/* Free each node recursively, starting at left-hand side */
static void recursive_free_left(automaton_t *automaton, node_t *root) {
    if (!root) return;
    if (root->down) recursive_free_left(automaton, root->down);
    if (root->left) recursive_free_left(automaton, root->left);
    if (root->right && (root->left || root->up || root->down)) {
         recursive_free_right(automaton, root->right);
    }
    if (root == automaton->outputs->head) {
        free_node(root, INT_ZER);
        automaton->outputs->head = automaton->outputs->tail = NULL;
        return;
    }
    free_node(root, INT_ONE);
}

/* Free each node recursively, starting at right-hand side */
static void recursive_free_right(automaton_t *automaton, node_t *root) {
    if (!root) return;
    if (root->down) recursive_free_right(automaton, root->down);
    if (root->right) recursive_free_right(automaton, root->right);
    if (root->left && (root->right || root->up || root->down)) {
        recursive_free_left(automaton, root->left);
    }
    if (root == automaton->outputs->head) {
        free_node(root, INT_ZER);
        automaton->outputs->head = automaton->outputs->tail = NULL;
        return;
    }
    free_node(root, INT_ONE);
}

/* Free each node */
void free_node(node_t *p1, int free_surround) {
    if (free_surround) null_surrounding_nodes(p1);
    free(p1->state);
    free(p1->str);
    free(p1);
    p1 = NULL;
}

/* Null surrounding nodes */
void null_surrounding_nodes(node_t *p1) {
    if (p1->up) p1->up->down = NULL;
    if (p1->left) p1->left->right = NULL;
    if (p1->right) p1->right->left = NULL;
    if (p1->down) p1->down->up = NULL;
}

/******************************************************************************
Algorithms are fun !!!
******************************************************************************/