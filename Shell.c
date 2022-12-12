#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "Shell.h"
#include "StringVector.h"

int* jobs;
size_t jobs_length;

void handle_sigchld() {
    int status;
    pid_t pid = waitpid( -1, &status, WNOHANG );
    for( size_t i = 0; i < jobs_length; i++ ) {
        if( jobs[i] == pid ) {
            jobs[i] = jobs[jobs_length - 1];
            jobs_length--;
            break;
        }
    }
}

void shell_init(struct Shell *this)
{
    this->running = false;
    this->line = NULL;
    this->line_number = 0;
    this->line_length = 0;
    signal( SIGCHLD, handle_sigchld );
}

void shell_free(struct Shell *this)
{
    if (NULL != this->line)
    {
        free(this->line);
        this->line = NULL;
    }
    this->line_length = 0;
}

void shell_run(struct Shell *this)
{
    this->running = true;
    printf("* Shell started\n");
    while (this->running)
    {
        shell_read_line(this);
        shell_execute_line(this);
    }
    printf("* Shell stopped\n");
}

void shell_read_line(struct Shell *this)
{
    this->line_number++;
    char *buf = getcwd(NULL, 0);
    printf("%d: %s> ", this->line_number, buf);
    free(buf);
    getline(&this->line, &this->line_length, stdin);
}

static void
do_help(struct Shell *this, const struct StringVector *args)
{
    printf("-> commands: exit, cd, help, ?.\n");
    (void)this;
    (void)args;
}



static void
do_jobs()
{
    for (size_t i = 0; i < jobs_length; i++)
    {
        printf("%d \n", jobs[i]);
    }
}

static int
is_redirect(const struct StringVector *args)
{
    // On ignore le premier et le dernier argument
    for(size_t i = 1; i < args->size - 1; i++)
    {
        if(strcmp( string_vector_get(args, i), ">") == 0)
        {
            return i;
        }
    }
    return -1;
}

static void
do_redirect(const struct StringVector *args, int redirectPos)
{
    // la fonction ne marche que dans certain cas
    size_t fullCommandSize = string_vector_size(args) - 1; // -1 pour enlever le point d'exclamation
    char *full_first_command = malloc(sizeof(char) * 100);
    char *full_second_command = malloc(sizeof(char) * 100);
    for(int i = 1; i < redirectPos; i++)
    {
        strcat(full_first_command, string_vector_get(args, i));
        strcat(full_first_command, " ");
    }
    for(size_t i = redirectPos + 1; i < fullCommandSize + 1; i++)
    {
        strcat(full_second_command, string_vector_get(args, i));
        strcat(full_second_command, " ");
    }
    printf("first command: %s\n", full_first_command);
    printf("second command: %s\n", full_second_command);
    char *tempFileName = "tmp.txt";
    strcat(full_first_command, " > ");
    strcat(full_first_command, tempFileName);
    printf("full first command: %s\n", full_first_command);
    system(full_first_command);
    free(full_first_command);
    strcat(full_second_command, " < ");
    strcat(full_second_command, tempFileName);
    printf("full second command: %s\n", full_second_command);   
    system(full_second_command);
    free(full_second_command);
}

static void
do_command(const struct StringVector *args)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        char *command = string_vector_get(args, 1);
        char *all_args[string_vector_size(args)];
        for (size_t i = 1; i < string_vector_size(args); i++)
        {
            all_args[i - 1] = string_vector_get(args, i);
        }
        all_args[string_vector_size(args) - 1] = NULL;
        execvp(command, all_args);
        printf("Command not found: %s\n", command);
        exit(EXIT_FAILURE);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
    }
}

static void
do_system(struct Shell *this, const struct StringVector *args)
{
    char *last = string_vector_get(args, string_vector_size(args) - 1);
    if (strcmp(last, "&") == 0)
    {
        pid_t pid = fork();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
        if (pid == 0)
        {
            char *command = string_vector_get(args, 1);
            char *argv[string_vector_size(args)];
            for (size_t i = 1; i < string_vector_size(args) - 1; i++)
            {
                argv[i - 1] = string_vector_get(args, i);
            }
            argv[string_vector_size(args) - 2] = NULL;
            execvp(command, argv);
            printf("Command not found: %s\n", command);
            exit(1);
        }
        else
        {
            jobs = realloc(jobs, sizeof(int) * (jobs_length + 1));
            jobs[jobs_length] = pid;
            jobs_length++;
            printf("Process %d started in background\n", pid);
        }
    }
    else if (strcmp(string_vector_get(args, 1), "jobs") == 0)
    {
        do_jobs();
    }
    else if (is_redirect(args) != -1)
    {
        do_redirect(args, is_redirect(args));
    }
    else
    {
        do_command(args);
    }
    (void)this;
    (void)args;
}

static void
do_cd(struct Shell *this, const struct StringVector *args)
{
    int nb_tokens = string_vector_size(args);
    char *tmp;
    if (1 == nb_tokens)
    {
        tmp = getenv("HOME");
    }
    else
    {
        tmp = string_vector_get(args, 1);
    }
    int rc = chdir(tmp);
    if (0 != rc)
        printf("directory '%s' not valid\n", tmp);
    (void)this;
}

static void
do_pwd(struct Shell *this, const struct StringVector *args)
{
    char *buf = getcwd(NULL, 0);
    printf("%s\n", buf);
    free(buf);
    (void)this;
    (void)args;
}

static void
do_rappel(struct Shell *this, const struct StringVector *args)
{
    (void)this;
    (void)args;
}

static void
do_execute(struct Shell *this, const struct StringVector *args)
{
    (void)this;
    (void)args;
}

static void
do_exit(struct Shell *this, const struct StringVector *args)
{
    this->running = false;
    (void)this;
    (void)args;
}

typedef void (*Action)(struct Shell *, const struct StringVector *);

static struct
{
    const char *name;
    Action action;
} actions[] = {
    {.name = "exit", .action = do_exit}, 
    {.name = "cd", .action = do_cd}, 
    {.name = "rappel", .action = do_rappel}, 
    {.name = "help", .action = do_help}, 
    {.name = "?", .action = do_help}, 
    {.name = "!", .action = do_system}, 
    {.name = "pwd", .action = do_pwd}, 
    {.name = NULL, .action = do_execute}
};

Action
get_action(char *name)
{
    int i = 0;
    while (actions[i].name != NULL && strcmp(actions[i].name, name) != 0)
    {
        i++;
    }
    return actions[i].action;
}

void shell_execute_line(struct Shell *this)
{
    struct StringVector tokens = split_line(this->line);
    int nb_tokens = string_vector_size(&tokens);

    if (nb_tokens == 0)
    {
        printf("-> Nothing to do !\n");
    }
    else
    {
        char *name = string_vector_get(&tokens, 0);
        Action action = get_action(name);
        action(this, &tokens);
    }

    string_vector_free(&tokens);
}
