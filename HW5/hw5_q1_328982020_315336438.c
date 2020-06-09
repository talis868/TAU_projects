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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct hw_component
{
	char name[NAME_LENGTH];
	int copies;
	struct hw_component *next;
} HW_component;

void check_open_file(FILE *file, char *path, char *file_name);

void procedure_actions(FILE *actions_file, FILE *components_file);
void call_action(char *action, char *param1, char *param2, HW_component **head_hw_component);
int parse_action_line(char *pline, char *action, char *param1, char *param2);
void parse_components_line(char *pline, int *copies, char *components);

// utils
void insert_new_component(HW_component **head_hw_component, HW_component *new_component);
HW_component* init_new_component(char *name, int copies);
int search_and_return_index(char search_term[NAME_LENGTH], HW_component **head_hw_component);
HW_component** return_pointer(int index, HW_component **head_hw_component);

// supported actions
void initialize(HW_component **head_hw_component, FILE *components_file);
void rename_component(char new_name[NAME_LENGTH], HW_component **component_to_change);
void finalize(HW_component **head_hw_component);
void update_component(HW_component **head_hw_component, char *name, char *copies_str);

int main()
{
	char *actions_path = "C:\\Users\\talil\\OneDrive\\sem B\\C\\HW5\\txt files\\actions.txt",
		*components_path = "C:\\Users\\talil\\OneDrive\\sem B\\C\\HW5\\txt files\\hw_components.txt",
		*output_path = "C:\\Users\\talil\\OneDrive\\sem B\\C\\HW5\\txt files\\components_updated.txt";  // TODO: turn to args from outside
	FILE *actions_file = fopen(actions_path, "r"), *components_file = fopen(components_path, "r");;

	check_open_file(actions_file, actions_path, "actions.txt");
	check_open_file(components_file, components_path, "hw_components.txt");

	procedure_actions(actions_file, components_file);

	fclose(actions_file);
	fclose(components_file);

	// TODO: memory leaks check

}

void check_open_file(FILE *file, char *path, char *file_name)
/**
 * Input:
 * return parameter: None
 * Function functionality:
 **/
{
	if (file == NULL)
	{
		printf("Error: opening %s failed\n", file_name);  // TODO: now what? exit?
	}
}

void procedure_actions(FILE *actions_file, FILE *components_file)
/**
 * Input: open files of the action and the components
 * return parameter: None
 * Function functionality: reads and runs an action per line in the file
 **/
{
	char line[ACTION_LINE_LENGTH + 1];
	char *action = NULL;
	char *param1 = NULL;
	char *param2 = NULL;
	HW_component *head_hw_component;

	while (fgets(line, ACTION_LINE_LENGTH + 1, actions_file) != NULL)
	{
		action = (char*)malloc(sizeof(char)*ACTION_LENGTH + 1);
		param1 = (char*)malloc(sizeof(char)*NAME_LENGTH + 1);
		param2 = (char*)malloc(sizeof(char)*NAME_LENGTH + 1);
		int op = parse_action_line(line, action, param1, param2);

		switch (op)
		{
		case INITIALIZE:
		{
			initialize(&head_hw_component, components_file);
			break;
		}
		case FINALIZE:
		{
			finalize(&head_hw_component);
			break;
		}
		case GENERAL:
		{
			call_action(action, param1, param2, &head_hw_component);
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
 * Input: line from action_file.txt and pointers to strings
 * return parameter: index to distinguish some of the actions
 * Function functionality: retrieves the action, the component name and the number/component name from the text line
 **/
{
	if (!strcmp(pline, "Initialize\n"))
	{
		return INITIALIZE;
	}
	if (!strcmp(pline, "Finalize\n"))
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

void initialize(HW_component **head_hw_component, FILE *components_file)
/**
 * Input: empty head for linked list and the open components file to be read
 * return parameter: None
 * Function functionality: creates a sorted linked list from the hardware in the file
 **/
{
	char line[COMPONENT_LINE_LENGTH], *component_name = (char*)malloc(sizeof(char)*NAME_LENGTH);
	int copies, *pcopies = &copies;
	HW_component *new_component = NULL;

	fgets(line, COMPONENT_LINE_LENGTH + 1, components_file);
	parse_components_line(line, pcopies, component_name);

	(*head_hw_component) = (HW_component*)malloc(sizeof(HW_component));  // This is the head
	strcpy((*head_hw_component)->name, component_name);
	(*head_hw_component)->copies = copies;
	(*head_hw_component)->next = NULL;

	free(component_name);

	while (fgets(line, COMPONENT_LINE_LENGTH + 1, components_file) != NULL)
	{
		component_name = (char*)malloc(sizeof(char)*NAME_LENGTH);
		parse_components_line(line, pcopies, component_name);

		new_component = init_new_component(component_name, copies);
		insert_new_component(head_hw_component, new_component);

		free(component_name);
	}
	//current->next = NULL;
	char search_term[NAME_LENGTH] = "Modem";
	int index = search_and_return_index(search_term, head_hw_component);
	HW_component** found = return_pointer(index, head_hw_component);
}

void insert_new_component(HW_component **head_hw_component, HW_component *new_component)
/**
 * Input: head of lined components list and the new component to be added
 * return parameter: None
 * Function functionality: search and find the right place to enter a new component in alphabetic order 
 **/
{
	if ((*head_hw_component)->next == NULL)
	{
		if (strcmp((*head_hw_component)->name, new_component->name) > 0)  // A, B
		{
			HW_component *temp_head = *head_hw_component;
			*head_hw_component = new_component;
			new_component->next = temp_head;
			new_component->next->next = NULL;
		}
		else  // B, A TODO: check what if the string are equal!
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
 * Input: line from hw_components.txt and pointers to strings
 * return parameter: None
 * Function functionality: retrieves component name and the number of copies
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

int search_and_return_index(char search_term[NAME_LENGTH], HW_component **head_hw_component)
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

HW_component** return_pointer(int index, HW_component **head_hw_component)
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

//void rename_component(char new_name[NAME_LENGTH], HW_component **component_to_change)
//{
//	(*component_to_change)->name = new_name;
//}

void finalize(HW_component **head_hw_component)
/**
 * Input:
 * return parameter: None
 * Function functionality:
 **/
{

}

void call_action(char *action, char *param1, char *param2, HW_component **head_hw_component)
/**
 * Input: action name, the first string is the name of the component and the second is either a name or number of copies
 * return parameter: None
 * Function functionality: Calls supported functions
 **/
{
	if (strcmp(action, "Rename") == 0)
	{
		{
			int index = search_and_return_index(param1, head_hw_component);
			HW_component **component_to_change = return_pointer(index, head_hw_component);
			//rename_component(param2, component_to_change);
		}
	}
	if (strcmp(action, "Returned_from_customer") == 0 || strcmp(action, "Production") == 0)
	{
		update_component(head_hw_component, param1, param2);
	}
}

void update_component(HW_component **head_hw_component, char *name, char *copies_str)
/**
 * Input: head component in linked list, the name of the component to change and the copies to be updated
 * return parameter: None
 * Function functionality: Updates copies number or add a new component if one does not exist
 **/
{
	int index = search_and_return_index(name, head_hw_component), copies = atoi(copies_str);

	if (index == -1)
	{
		HW_component *new_component = init_new_component(name, copies);
		insert_new_component(head_hw_component, new_component);
	}
	else
	{
		HW_component **returned_hardware = return_pointer(index, head_hw_component);
		(*returned_hardware)->copies = (*returned_hardware)->copies + copies;
	}
}

HW_component* init_new_component(char *name, int copies)
/**
 * Input: component name and number of copies
 * return parameter: new allocated hw component
 * Function functionality: Allocates a space for a new component and fills the params
 **/
{
	HW_component* new_component = (HW_component*)malloc(sizeof(HW_component));
	strcpy(new_component->name, name);
	new_component->copies = copies;
	return new_component;
}
