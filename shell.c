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
#define MAX_ARG_SIZE 64

// Function declarations

void dt();
void sysinfo();
void fact();

void calc();

void help()
{
    printf("\n==================== Command List ====================\n\n");

    // System Information Commands Section
    printf(" ***** System Information Commands *****\n\n");
    printf("sysinfo  - Display system information\n");
    printf("dt       - Display the current date and time\n");
    printf("clear    - Clear the screen\n");
    printf("exit     - Exit the program\n\n");

    // Directory Management Commands Section
    printf(" ***** Directory Management Commands *****\n\n");
    printf("ls       - List files and directories in the current directory\n");
    printf("cd       - Change the current working directory\n");
    printf("mkdir    - Create a new directory\n");
    printf("rmdir    - Remove an empty directory\n\n");

    // Example of Redirection
    printf(" ***** Input/Output Redirection Example *****\n\n");
    printf("~ cat > a.txt\n");
    printf("Hi, my name is Garvit\n");
    printf("~ cat < a.txt\n");
    printf("Hi, my name is Garvit\n");
    printf("~\n\n");

    // Custom Functions Section
    printf(" ***** Custom Function Commands *****\n\n");

    printf("fact     - Find the factorial of a number\n");
    printf("calc     - Perform basic calculations\n");
    printf("tconv    - Convert temperatures\n\n");

    // Input/Output Redirection Info
    printf(" ***** Additional Information *****\n");
    printf("- Supports input (<) and output (>) redirection\n");

    printf("\n========================================================\n");
}

// Function Defination

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

void parse_input(char *input, char **input_file, char **output_file, char **args, int *background)
{
    char *token;
    int index = 0;

    // Tokenize the user input based on spaces
    token = strtok(input, " ");
    while (token != NULL)
    {
        if (strcmp(token, "<") == 0)
        {
            // Handle input redirection
            token = strtok(NULL, " ");
            if (token != NULL)
            {
                *input_file = token;
            }
            else
            {
                fprintf(stderr, "Error: expected file after '<'\n");
                return;
            }
        }
        else if (strcmp(token, ">") == 0)
        {
            // Handle output redirection
            token = strtok(NULL, " ");
            if (token != NULL)
            {
                *output_file = token;
            }
            else
            {
                fprintf(stderr, "Error: expected file after '>'\n");
                return;
            }
        }
        else if (strcmp(token, "&") == 0)
        {
            // Indicate the process should run in the background
            *background = 1;
        }
        else
        {
            args[index++] = token;
        }

        // Move to the next token
        token = strtok(NULL, " ");
    }

    // Terminate the argument list with a NULL pointer
    args[index] = NULL;
}

void execute_command(char *input_file, char *output_file, char **args, int background)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
    }
    else if (pid == 0)
    {
        // In the child process

        // Set up input redirection if needed
        if (input_file != NULL)
        {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0)
            {
                perror("Error opening input file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO); // Redirect stdin to input file
            close(fd_in);
        }

        // Set up output redirection if needed
        if (output_file != NULL)
        {
            int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0)
            {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO); // Redirect stdout to output file
            close(fd_out);
        }

        // Execute the command
        if (execvp(args[0], args) == -1)
        {
            perror("execvp");
        }

        // Exit child process if execvp fails
        exit(EXIT_FAILURE);
    }
    else
    {
        // In the parent process

        if (!background)
        {
            // Wait for the child process to finish if not running in the background
            waitpid(pid, NULL, 0);
        }
        else
        {
            // If the command is running in the background, notify the user
            printf("Background process started with PID %d\n", pid);
        }
    }
}

void show_welcome_message()
{
    printf("\n\n===============================\n\n");
    printf("Welcome to Garvit's Custom Shell\n");
    dt();
    printf("Type \"help\" to view available commands!\n\n");
}

int main()
{
    // Display the welcome message and date/time
    show_welcome_message();

    char input[MAX_INPUT_SIZE]; // Buffer to store user input
    char *args[MAX_ARG_SIZE];   // Array for command arguments
    int is_background = 0;      // Flag to indicate background process
    char *input_redir = NULL;   // For input redirection
    char *output_redir = NULL;  // For output redirection

    while (1)
    {
        // Prompt the user for input
        printf("~ ");
        fflush(stdout);

        // Read input from user
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL)
        {
            break; // Exit the loop if input is EOF
        }

        // Clean up the newline character from the input
        input[strcspn(input, "\n")] = 0;

        // Reset flags and redirection pointers
        is_background = 0;
        input_redir = NULL;
        output_redir = NULL;

        // Parse the user input to separate the command and arguments
        parse_input(input, &input_redir, &output_redir, args, &is_background);

        // Skip empty commands
        if (args[0] == NULL)
        {
            continue;
        }

        // Check for built-in commands
        if (strcmp(args[0], "exit") == 0)
        {
            break; // Exit the shell
        }
        else if (strcmp(args[0], "help") == 0)
        {
            help(); // Display help information
            continue;
        }
        else if (strcmp(args[0], "dt") == 0)
        {
            dt(); // Show date and time
            continue;
        }
        else if (strcmp(args[0], "fact") == 0)
        {
            fact(); // Calculate factorial
            continue;
        }
        else if (strcmp(args[0], "tconv") == 0)
        {
            tconv(); // Perform temperature conversion
            continue;
        }
        else if (strcmp(args[0], "sysinfo") == 0)
        {
            sysinfo(); // Display system information
            continue;
        }
        else if (strcmp(args[0], "calc") == 0)
        {
            calc(); // Perform calculations
            continue;
        }
        else if (strcmp(args[0], "cd") == 0)
        {
            // Handle the 'cd' command for directory changes
            if (args[1] == NULL)
            {
                fprintf(stderr, "cd: missing directory argument\n");
            }
            else if (chdir(args[1]) != 0)
            {
                perror("cd"); // Print error if 'cd' fails
            }
            continue;
        }
        // If no built-in command, execute external command
        execute_command(input_redir, output_redir, args, is_background);
    }
    return 0;
}
