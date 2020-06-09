#define _CRT_SECURE_NO_WARNINGS

#define TRUE 1
#define FALSE 0

#define NAME_LENGTH 200
#define ACTION_LINE_LENGTH 242  // 22+5+200+5+10
#define ACTION_LENGTH 22

#define GENERAL 2
#define INITIALIZE 3
#define FINALIZE 4

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

void read_actions(FILE *actions_file, FILE *components_file);
int parse_action_line(char *pline, char *action, char *param1, char *param2);

void initialize(HW_component **empty_hw_component);
void finalize(HW_component **head_hw_component);
void call_action(char *action, char *param1, char *param2, HW_component *head_hw_component);

int main()
{
	char *actions_path = "C:\\Users\\talil\\OneDrive\\sem B\\C\\HW5\\txt files\\actions.txt",
		*components_path = "C:\\Users\\talil\\OneDrive\\sem B\\C\\HW5\\txt files\\hw_components.txt",
		*output_path = "C:\\Users\\talil\\OneDrive\\sem B\\C\\HW5\\txt files\\components_updated.txt";  // TODO: turn to args from outside
	FILE *actions_file = fopen(actions_path, "r"), *components_file = fopen(components_path, "r");;
	
	check_open_file(actions_file, actions_path, "actions.txt");
	check_open_file(components_file, components_path, "hw_components.txt");
	
	read_actions(actions_file, components_file);

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

void read_actions(FILE *actions_file, FILE *components_file)
/**
 * Input: open files of the action and the components
 * return parameter: None
 * Function functionality:
 **/
{
	char line[ACTION_LINE_LENGTH + 1];
	char *action = NULL;
	char *param1 = NULL;
	char *param2 = NULL;
	HW_component *head_hw_component;
	
	while (fgets(line, ACTION_LINE_LENGTH + 1, actions_file) != NULL)
	{
		action = (char*) malloc(sizeof(char)*ACTION_LENGTH + 1);
		param1 = (char*) malloc(sizeof(char)*NAME_LENGTH + 1);
		param2 = (char*) malloc(sizeof(char)*NAME_LENGTH + 1);
		int op = parse_action_line(line, action, param1, param2);

		switch (op)
		{
			case INITIALIZE:
			{
				initialize(&head_hw_component);
				break;
			}
			case FINALIZE:
			{
				finalize(&head_hw_component);
				break;
			}
			case GENERAL:
			{
				//call_action(action, param1, param2, head_hw_component);
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
		return GENERAL;
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
			p_current_sub[sub_ix-1] = EOS;  // There is always a whitespace in the first two parts
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

void initialize(HW_component **empty_hw_component)
/**
 * Input:
 * return parameter: None
 * Function functionality:
 **/
{
	
}

void finalize(HW_component **head_hw_component)
/**
 * Input:
 * return parameter: None
 * Function functionality:
 **/
{
	
}

void call_action(char *action, char *param1, char *param2, HW_component *head_hw_component)
/**
 * Input:
 * return parameter: None
 * Function functionality:
 **/
{
	// TODO: add switch case and call the functions according to the text ("fire", "production" etc.)
}


/***
 * Initialize
Returned_from_customer $$$ Hub ultra $$$ 12
Fire $$$ Hub $$$ 13
Returned_from_customer $$$ Modem $$$ 12
Production $$$ Nic $$$ 2
Fire $$$ Router promore 10000GG $$$ 1
Rename $$$ Router pro 100GG $$$ Router pro112 10000GG
Returned_from_customer $$$ Hub pro $$$ 5
Production $$$ Nic pro 2000GG $$$ 8
Fatal_malfunction $$$ Nic 100GG $$$ 7
Production $$$ Nic pro 2000GG $$$ 8000
Returned_from_customer $$$ Hub ultraZZZZZ $$$ 12
Finalize
 */
