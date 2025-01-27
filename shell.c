#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <sys/stat.h> // For mkdir
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARG_SIZE 100

// Function declarations

void dt();
void sysinfo();
void fact();
void tconv();

void calc()
{
    double num1, num2, result;
    char op;

    printf("Enter a calculation (e.g., 5 + 3): ");
    scanf("%lf %c %lf", &num1, &op, &num2);

    switch (op)
    {
    case '+':
        result = num1 + num2;
        printf("Result: %.2lf\n", result);
        break;
    case '-':
        result = num1 - num2;
        printf("Result: %.2lf\n", result);
        break;
    case '*':
        result = num1 * num2;
        printf("Result: %.2lf\n", result);
        break;
    case '/':
        if (num2 == 0)
        {
            printf("Error: Division by zero is not allowed.\n");
        }
        else
        {
            result = num1 / num2;
            printf("Result: %.2lf\n", result);
        }
        break;
    default:
        printf("Error: Invalid operator. Use +, -, *, or /.\n");
        break;
    }
}
void parse_input(char *input, char **args, int *background, char **input_file, char **output_file);
void execute_command(char **args, int background, char *input_file, char *output_file);

void help();

void tconv()
{
    int temp, choice;
    float result;

    printf("Temperature Conversion:\n");
    printf("1. Celsius to Fahrenheit\n");
    printf("2. Fahrenheit to Celsius\n");
    printf("Enter choice: ");
    scanf("%d", &choice);
    printf("Enter temperature: ");
    scanf("%d", &temp);

    switch (choice)
    {
    case 1:
        result = (temp * 9 / 5) + 32;
        printf("Celsius to Fahrenheit: %.2f\n", result);
        break;
    case 2:
        result = (temp - 32) * 5 / 9;
        printf("Fahrenheit to Celsius: %.2f\n", result);
        break;
    default:
        printf("Invalid choice.\n");
        break;
    }
}

void dt()
{
    time_t tt = time(NULL);
    printf("%s", ctime(&tt));
}

void fact()
{
    int n;
    unsigned long long result = 1;
    printf("Enter a number: ");
    scanf("%d", &n);

    if (n < 0)
    {
        printf("Error: Factorial is not defined for negative numbers.\n");
        return;
    }

    for (int i = 1; i <= n; i++)
    {
        result *= i;
    }

    printf("Factorial of %d is %llu\n", n, result);
}

void sysinfo()
{
    struct utsname sys;
    if (uname(&sys) == 0)
    {
        printf("\nSystem Information:\n");
        printf("  System Name: %s\n", sys.sysname);
        printf("  Node Name  : %s\n", sys.nodename);
        printf("  Release    : %s\n", sys.release);
        printf("  Version    : %s\n", sys.version);
        printf("  Machine    : %s\n", sys.machine);
    }
    else
    {
        perror("Unable to fetch system information");
    }
}

int main()
{

    printf("\n\n*******************************\n\n");
    printf("Welcome to Garvit's Terminal\n");
    dt();
    printf("Type \"help\" for more commands!\n\n");

    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARG_SIZE];
    int background = 0;       // Flag for background processes
    char *input_file = NULL;  // For input redirection
    char *output_file = NULL; // For output redirection

    while (1)
    {
        // Display shell prompt
        printf("~ ");
        fflush(stdout);

        // Get user input
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL)
        {
            break; // Exit on EOF
        }

        // Remove trailing newline character
        input[strcspn(input, "\n")] = 0;

        // Reset flags and files
        background = 0;
        input_file = NULL;
        output_file = NULL;

        // Parse input into arguments and check for redirection
        parse_input(input, args, &background, &input_file, &output_file);

        // Handle built-in commands like cd, exit, help
        if (args[0] == NULL)
        {
            continue; // Empty input
        }
        else if (strcmp(args[0], "exit") == 0)
        {
            break; // Exit the shell
        }
        else if (strcmp(args[0], "help") == 0)
        {
            help();
            continue;
        }
        else if (strcmp(args[0], "dt") == 0)
        {
            dt();
            continue;
        }
        else if (strcmp(args[0], "fact") == 0)
        {
            fact();
            continue;
        }
        else if (strcmp(args[0], "tconv") == 0)
        {
            tconv();
            continue;
        }
        else if (strcmp(args[0], "sysinfo") == 0)
        {
            sysinfo();
            continue;
        }
        else if (strcmp(args[0], "calc") == 0)
        {
            calc();
            continue;
        }
        else if (strcmp(args[0], "cd") == 0)
        {
            if (args[1] == NULL)
            {
                fprintf(stderr, "cd: missing argument\n");
            }
            else if (chdir(args[1]) != 0)
            {
                perror("cd");
            }
            continue;
        }

        // Execute the command
        execute_command(args, background, input_file, output_file);
    }

    return 0;
}

void parse_input(char *input, char **args, int *background, char **input_file, char **output_file)
{
    char *token;
    int arg_index = 0;

    // Tokenize input
    token = strtok(input, " ");
    while (token != NULL)
    {
        if (strcmp(token, "<") == 0)
        {
            // Input redirection
            token = strtok(NULL, " ");
            if (token != NULL)
            {
                *input_file = token;
            }
            else
            {
                fprintf(stderr, "Syntax error: expected file after '<'\n");
                return;
            }
        }
        else if (strcmp(token, ">") == 0)
        {
            // Output redirection
            token = strtok(NULL, " ");
            if (token != NULL)
            {
                *output_file = token;
            }
            else
            {
                fprintf(stderr, "Syntax error: expected file after '>'\n");
                return;
            }
        }
        else if (strcmp(token, "&") == 0)
        {
            // Background process
            *background = 1;
        }
        else
        {
            args[arg_index++] = token;
        }

        token = strtok(NULL, " ");
    }

    args[arg_index] = NULL; // Null-terminate the argument list
}

void execute_command(char **args, int background, char *input_file, char *output_file)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
    }
    else if (pid == 0)
    {
        // Child process

        // Handle input redirection
        if (input_file != NULL)
        {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0)
            {
                perror("open (input_file)");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // Handle output redirection
        if (output_file != NULL)
        {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
            {
                perror("open (output_file)");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Execute the command
        if (execvp(args[0], args) == -1)
        {
            perror("execvp");
        }

        exit(EXIT_FAILURE);
    }
    else
    {
        // Parent process
        if (!background)
        {
            waitpid(pid, NULL, 0); // Wait for the child process to finish
        }
        else
        {
            printf("Process running in background with PID %d\n", pid);
        }
    }
}

void help()
{
    printf("\nCommands:\n\n");

    printf(" ***** System Info Commands ******\n\n");
    printf("sysinfo - To get system information\n");
    printf("dt      - Display current date and time\n");
    printf("clear   - Clear the screen\n");
    printf("exit    - Exit the program\n\n");

    printf(" ***** Directory Management ******\n\n");
    printf("ls      - List files and directories in the current directory\n");
    printf("cd      - Change the current working directory\n");
    printf("mkdir   - Change the current working directory\n");
    printf("rmdir   - Change the current working directory\n");

    // ~ cat > a.txt
    // Hi my name is garvit
    // ~ cat < a.txt
    // Hi my name is garvit
    // ~

    printf(" ***** Custom Function ******\n\n");
    printf("fact    - Find Factorial of a number\n");
    printf("calc    - Perform basic calculations\n");
    printf("tconv   - Convert temperatures\n\n");

    printf("- support input (<) and output (>) redirection\n");
}
