#define _CRT_SECURE_NO_WARNINGS

#define TRUE 1
#define FALSE 0

#define NAME_LENGTH 200
#define ACTION_LINE_LENGTH 242  // 22+5+200+5+10
#define ACTION_LENGTH 22
#define COMPONENT_LINE_LENGTH 214  //  200+4+10

#define GENERAL 2
#define INITIALIZE 3
#define FINALIZE 4
#define RENAME = "Rename"

#define EOL '\n'
#define EOS '\0'

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct hw_component
{
	char name[NAME_LENGTH];
	int copies;
	struct hw_component *next;
} HW_component;

void procedure_actions(FILE *actions_file, char *components_path, char *output_path);
void call_action(char *action, char *param1, char *param2, HW_component **head_hw_component, FILE *actions_file);
int parse_action_line(char *pline, char *action, char *param1, char *param2);
void parse_components_line(char *pline, int *copies, char *components);

// utils
void insert_new_component(HW_component **head_hw_component, HW_component *new_component);
HW_component* init_new_component(char *name, int copies);
int search_component_index(char search_term[NAME_LENGTH], HW_component **head_hw_component);
HW_component** return_pointer_from_component_index(int index, HW_component **head_hw_component);
void free_linked_list(HW_component **head_hw_component);

// supported actions
void initialize(HW_component **head_hw_component, FILE *components_file, FILE *actions_file);
void rename_component(char* name, char* new_name, HW_component **head_hw_component);
void update_component(HW_component **head_hw_component, char *name, char *copies_str, FILE *actions_file);
void remove_copies_of_component(char* name, char* copies_str, HW_component **head_hw_component);
void finalize(HW_component **head_hw_component, FILE *output_file);


int main(int argc, char* argv[])
{
	if (argc == 4)
	{
		char *components_path = argv[1],
			*actions_path = argv[2],
			*output_path = argv[3];
		FILE *actions_file = fopen(actions_path, "r");
		if (actions_file == NULL)
		{
			printf("Error: opening %s failed\n", "actions.txt");
			printf("%d", _CrtDumpMemoryLeaks());
			exit(1);
		}

		procedure_actions(actions_file, components_path, output_path);

		fclose(actions_file);
		printf("END OF MAIN: %d", _CrtDumpMemoryLeaks());
		return 0;
	}
	else
	{
		printf("Error: invalid number of arguments (<%d> instead of 3)\n", argc - 1);
		return 1;
	}
}

void procedure_actions(FILE *actions_file, char *components_path, char *output_path)
/**
 * Input: Open files of the action and the components
 * return parameter: None
 * Function functionality: Reads and runs an action per line in the file
 **/
{
	char line[ACTION_LINE_LENGTH + 1], *action = NULL, *param1 = NULL, *param2 = NULL;
	int is_initialized = FALSE;
	HW_component *head_hw_component = NULL;

	while (fgets(line, ACTION_LINE_LENGTH + 1, actions_file) != NULL)
	{
		action = (char*)malloc(sizeof(char)*ACTION_LENGTH + 1);
		if (action == NULL)
		{
			printf("Error: memory allocation failed\n");
			free_linked_list(&head_hw_component);
			fclose(actions_file);
			printf("%d", _CrtDumpMemoryLeaks());
			exit(1);
		}
		param1 = (char*)malloc(sizeof(char)*NAME_LENGTH + 1);
		if (param1 == NULL)
		{
			printf("Error: memory allocation failed\n");
			free(action);
			free_linked_list(&head_hw_component);
			fclose(actions_file);
			printf("%d", _CrtDumpMemoryLeaks());
			exit(1);
		}
		param2 = (char*)malloc(sizeof(char)*NAME_LENGTH + 1);
		if (param2 == NULL)
		{
			printf("Error: memory allocation failed\n");
			free(action);
			free(param1);
			free_linked_list(&head_hw_component);
			fclose(actions_file);
			printf("%d", _CrtDumpMemoryLeaks());
			exit(1);
		}
		
		int op = parse_action_line(line, action, param1, param2);

		switch (op)
		{
			case INITIALIZE:
			{
				FILE *components_file = fopen(components_path, "r");
				if (components_file == NULL)
				{
					printf("Error: opening %s failed\n", "hw_components.txt");
					fclose(actions_file);
					free(action);
					free(param1);
					free(param2);
					exit(1);
				}
				initialize(&head_hw_component, components_file, actions_file);
				fclose(components_file);
				break;
			}
			case FINALIZE:
			{
				FILE *output_file = fopen(output_path, "w");
				if (output_file == NULL)
				{
					printf("Error: opening %s failed\n", "updated_components.txt");
					fclose(actions_file);
					free_linked_list(&head_hw_component);
					free(action);
					free(param1);
					free(param2);
					exit(1);
				}
				finalize(&head_hw_component, output_file);
				fclose(output_file);
				break;
			}
			case GENERAL:
			{
				call_action(action, param1, param2, &head_hw_component, actions_file);
				break;
			}
		}

		free(action);
		free(param1);
		free(param2);
	}
}

int parse_action_line(char *pline, char *action, char *param1, char *param2)
/**
 * Input: Line from action_file.txt and pointers to strings
 * return parameter: Index to distinguish some of the actions
 * Function functionality: Retrieves the action, the component name and the number/component name from the text line
 **/
{
	if (!strcmp(pline, "Initialize\n"))
	{
		return INITIALIZE;
	}
	if (!strcmp(pline, "Finalize"))
	{
		return FINALIZE;
	}
	int line_ix = 0, sub_ix = 0, is_dollar_appeared = FALSE;
	char *p_current_sub = action;

	while (pline[line_ix] != EOL)
	{
		if (pline[line_ix] == '$')
		{
			p_current_sub[sub_ix - 1] = EOS;  // There is always a whitespace in the first two parts
			if (is_dollar_appeared)  // Third part
			{
				p_current_sub = param2;
			}
			else  // Second part
			{
				p_current_sub = param1;
				is_dollar_appeared = TRUE;
			}
			line_ix = line_ix + 4;
			sub_ix = 0;
			continue;
		}
		p_current_sub[sub_ix++] = pline[line_ix++];
	}
	p_current_sub[sub_ix] = EOS;
	return GENERAL;
}

void initialize(HW_component **head_hw_component, FILE *components_file, FILE *actions_file)
/**
 * Input: Empty head for linked list and the open components file to be read
 * return parameter: None
 * Function functionality: Creates a sorted linked list from the hardware in the file
 **/
{
	char line[COMPONENT_LINE_LENGTH], *component_name = (char*)malloc(sizeof(char)*NAME_LENGTH);
	int copies, *pcopies = &copies;
	HW_component *new_component = NULL;

	if (component_name == NULL)
	{
		printf("Error: memory allocation failed\n");
		fclose(components_file);
		fclose(actions_file);
		printf("%d", _CrtDumpMemoryLeaks());
		exit(1);
	}

	fgets(line, COMPONENT_LINE_LENGTH + 1, components_file);
	if (strchr(line, '$') == NULL)
	{
		free(component_name);
		return;
	}
	parse_components_line(line, pcopies, component_name);

	(*head_hw_component) = (HW_component*)malloc(sizeof(HW_component));  // This is the head
	if ((*head_hw_component) == NULL)
	{
		printf("Error: memory allocation failed\n");
		free(component_name);
		fclose(components_file);
		fclose(actions_file);
		printf("%d", _CrtDumpMemoryLeaks());
		exit(1);
	}
	strcpy((*head_hw_component)->name, component_name);
	(*head_hw_component)->copies = copies;
	(*head_hw_component)->next = NULL;

	free(component_name);

	while (fgets(line, COMPONENT_LINE_LENGTH + 1, components_file) != NULL)
	{
		component_name = (char*)malloc(sizeof(char)*NAME_LENGTH);
		if (component_name == NULL)
		{
			printf("Error: memory allocation failed\n");
			free_linked_list(head_hw_component);
			fclose(components_file);
			fclose(actions_file);
			printf("%d", _CrtDumpMemoryLeaks());
			exit(1);
		}
		parse_components_line(line, pcopies, component_name);

		new_component = init_new_component(component_name, copies);
		if (new_component == NULL)
		{
			printf("Error: memory allocation failed\n");
			free_linked_list(head_hw_component);
			fclose(components_file);
			fclose(actions_file);
			printf("%d", _CrtDumpMemoryLeaks());
			exit(1);
		}
		insert_new_component(head_hw_component, new_component);

		free(component_name);
	}
}

void insert_new_component(HW_component **head_hw_component, HW_component *new_component)
/**
 * Input: Head of lined components list and the new component to be added
 * return parameter: None
 * Function functionality: Search and find the right place to enter a new component in alphabetic order
 **/
{
	if (*head_hw_component == NULL)
	{
		*head_hw_component = new_component;
		(*head_hw_component)->next = NULL;
		return;
	}
	
	if ((*head_hw_component)->next == NULL)
	{
		if (strcmp((*head_hw_component)->name, new_component->name) > 0)  // A, B
		{
			HW_component *temp_head = *head_hw_component;
			*head_hw_component = new_component;
			new_component->next = temp_head;
			new_component->next->next = NULL;
		}
		else
		{
			new_component->next = NULL;
			(*head_hw_component)->next = new_component;
		}
		return;
	}

	HW_component *temp_head_hw_component = *head_hw_component;
	while (temp_head_hw_component->next != NULL)
	{
		if (strcmp((*head_hw_component)->name, new_component->name) > 0)
		{
			new_component->next = *head_hw_component;
			(*head_hw_component) = new_component;
			return;
		}
		else if (strcmp((temp_head_hw_component)->next->name, new_component->name) > 0)
		{
			new_component->next = temp_head_hw_component->next;
			temp_head_hw_component->next = new_component;
			return;
		}
		else
		{
			(temp_head_hw_component) = temp_head_hw_component->next;
		}
	}
	new_component->next = NULL;
	temp_head_hw_component->next = new_component;
}

void parse_components_line(char *pline, int *copies, char *component)
/**
 * Input: Line from hw_components.txt and pointers to strings
 * return Parameter: None
 * Function functionality: Retrieves component name and the number of copies
 **/
{
	int line_ix = 0, sub_ix = 0;
	char *p_current_sub = component, string_copies[10];

	while (pline[line_ix] != EOL)
	{
		if (pline[line_ix] == '$')
		{
			p_current_sub[sub_ix - 1] = EOS;  // There is always a whitespace in the first two parts
			p_current_sub = string_copies;
			line_ix = line_ix + 4;
			sub_ix = 0;
			continue;
		}
		p_current_sub[sub_ix++] = pline[line_ix++];
	}

	*copies = atoi(string_copies);
}

int search_component_index(char search_term[NAME_LENGTH], HW_component **head_hw_component)
/**
 * Input: Name of component to search for, head component in linked list
 * return parameter: Index of found component in linked list
 * Function functionality: Searches for a component in linked list. returns index if found, returns "-1" if not found
 **/
{
	int index = 0;

	HW_component *found = *head_hw_component;
	while (found != NULL && strcmp(found->name, search_term) != 0)
	{
		index++;
		found = found->next;
	}

	return (found != NULL) ? index : -1;
}

HW_component** return_pointer_from_component_index(int index, HW_component **head_hw_component)
/**
 * Input: Index of component to find, head component in linked list
 * return parameter: Double pointer to the wanted component
 * Function functionality: Returns a double pointer to the component in the given index of a linked list
 **/
{
	int counter = 0;
	HW_component *found = *head_hw_component;
	HW_component **pfound = &found;
	while (index != counter)
	{
		counter++;
		found = found->next;
	}
	// TODO: check if I can move up
	return pfound;
}

void finalize(HW_component **head_hw_component, FILE *output_file)
/**
 * Input: head linked hardware components list, the open output file in write mode
 * return parameter: None
 * Function functionality: writes the linked list to the file and gets ready to end the program
 **/
{
	if (*head_hw_component == NULL)
	{
		return;
	}
	HW_component *curr_component = *head_hw_component;
	while (curr_component->next != NULL)
	{
		fprintf(output_file, "%s $$$ %d\n", curr_component->name, curr_component->copies);
		curr_component = curr_component->next;
	}
	fprintf(output_file, "%s $$$ %d", curr_component->name, curr_component->copies);
	free_linked_list(head_hw_component);
}

void call_action(char *action, char *param1, char *param2, HW_component **head_hw_component, FILE *actions_file)
/**
 * Input: Action name, the first string is the name of the component and the second is either a name or number of copies
 * return parameter: None
 * Function functionality: Calls supported functions
 **/
{
	if (strcmp(action, "Rename") == 0)
	{
		rename_component(param1, param2, head_hw_component);
	}
	if (strcmp(action, "Returned_from_customer") == 0 || strcmp(action, "Production") == 0)
	{
		update_component(head_hw_component, param1, param2, actions_file);
	}
	if (strcmp(action, "Fatal_malfunction") == 0 || strcmp(action, "Fire") == 0)
	{
		remove_copies_of_component(param1, param2, head_hw_component);
	}
}

void rename_component(char* name, char* new_name, HW_component **head_hw_component)
/**
 * Input: Name to find and change, new name, head component in linked list
 * return parameter: None
 * Function functionality: Finds the given component and renames it, then realphabetizes the linked list. If the component isn't found, the function does nothing.
 **/
{

	int index = search_component_index(name, head_hw_component);

	if (index == -1)
	{
		return;
	}
	else if (index == 0)
	{
		HW_component** component_to_rename = return_pointer_from_component_index(index, head_hw_component);
		strcpy((*component_to_rename)->name, new_name);
		*head_hw_component = (*head_hw_component)->next;
		insert_new_component(head_hw_component, *component_to_rename);
	}
	else
	{
		HW_component** component_before_component_to_rename = return_pointer_from_component_index(index - 1, head_hw_component);
		HW_component* component_to_rename = (*component_before_component_to_rename)->next;
		strcpy((*component_before_component_to_rename)->next->name, new_name);
		(*component_before_component_to_rename)->next = (*component_before_component_to_rename)->next->next;
		insert_new_component(head_hw_component, component_to_rename);
	}
}

void update_component(HW_component **head_hw_component, char *name, char *copies_str, FILE *actions_file)
/**
 * Input: head component in linked list, the name of the component to change and the copies to be updated
 * return parameter: None
 * Function functionality: Updates copies number or add a new component if one does not exist
 **/
{
	int index, copies = atoi(copies_str);
	
	if (*head_hw_component == NULL)
	{
		index = -1;
	}
	else
	{
		index = search_component_index(name, head_hw_component);
	}

	if (index == -1)
	{
		HW_component *new_component = init_new_component(name, copies);
		if (new_component == NULL)
		{
			free_linked_list(head_hw_component);
			fclose(actions_file);
			printf("%d", _CrtDumpMemoryLeaks());
			exit(1);
		}
		insert_new_component(head_hw_component, new_component);
	}
	else
	{
		HW_component **returned_hardware = return_pointer_from_component_index(index, head_hw_component);
		(*returned_hardware)->copies = (*returned_hardware)->copies + copies;
	}
}

void remove_copies_of_component(char* name, char* copies_str, HW_component **head_hw_component)
/**
 * Input: Name of component to remove copies from, number of copies to be removed, head component of linked list
 * return parameter: None
 * Function functionality: Removes the given number of copies from the given component. If the given number of copies is larger than the available copies, the component is updated to 0 copies.
 **/
{
	int index = search_component_index(name, head_hw_component), bad_copies = atoi(copies_str);

	if (index == -1)
	{
		return;
	}
	else
	{
		HW_component** component_to_update = return_pointer_from_component_index(index, head_hw_component);
		if ((*component_to_update)->copies <= bad_copies)
		{
			(*component_to_update)->copies = 0;
		}
		else
		{
			(*component_to_update)->copies = (*component_to_update)->copies - bad_copies;
		}
	}
}

HW_component* init_new_component(char *name, int copies)
/**
 * Input: Component name and number of copies
 * return parameter: New allocated hw component
 * Function functionality: Allocates a space for a new component and fills the params
 **/
{
	HW_component* new_component = (HW_component*)malloc(sizeof(HW_component));
	if (new_component == NULL)
	{
		return NULL;
	}
	strcpy(new_component->name, name);
	new_component->copies = copies;
	return new_component;
}

void free_linked_list(HW_component **head_hw_component)
/**
 * Input: head node of the linked hardware components list
 * return parameter: None
 * Function functionality: frees all the nodes in the linked list
 **/
{
	HW_component *temp = *head_hw_component;
	while (*head_hw_component != NULL)
	{
		temp = *head_hw_component;
		*head_hw_component = (*head_hw_component)->next;
		free(temp);
	}
}