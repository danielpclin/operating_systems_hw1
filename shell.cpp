#include <iostream>
#include <unistd.h>
#include <queue>
#include <fcntl.h>
#include "helper.cpp"

class ParsedCommands{
    std::deque<std::string> commands;
    std::deque<std::string> from_files;
    std::deque<std::string> to_files;
public:
    explicit ParsedCommands(std::string line){
        enum Type{
            COMMAND, REDIRECT_TO, REDIRECT_FROM
        };
        Type type = COMMAND;
        int current = 0;
        for (int i=0; i < line.length(); i++) {
            if (line[i] != '|' && line[i] != '>' && line[i] != '<') {
                continue;
            }
            std::string command = trim_copy(line.substr(current, i-current));
            if (!command.empty()){
                switch (type) {
                    case COMMAND:
                        commands.push_back(command);
                        break;
                    case REDIRECT_TO:
                        to_files.push_back(command);
                        break;
                    case REDIRECT_FROM:
                        from_files.push_back(command);
                        break;
                }
            }
            current = i+1;
            switch (line[i]) {
                case '|':
                    type = COMMAND;
                    break;
                case '>':
                    type = REDIRECT_TO;
                    break;
                case '<':
                    type = REDIRECT_FROM;
                    break;
            }
        }
        std::string command = trim_copy(line.substr(current));
        if (!command.empty()){
            switch (type) {
                case COMMAND:
                    commands.push_back(command);
                    break;
                case REDIRECT_TO:
                    to_files.push_back(command);
                    break;
                case REDIRECT_FROM:
                    from_files.push_back(command);
                    break;
            }
        }
    }

    static void create_proc(const std::string& command, int fd_in, int fd_out, int pipes_count, int pipes_fd[][2])
    {
        pid_t proc = fork();

        if (proc < 0)
        {
            fprintf(stderr, "Error: Unable to fork.\n");
            exit(EXIT_FAILURE);
        }
        else if (proc == 0)
        {
            if (fd_in != STDIN_FILENO) { dup2(fd_in, STDIN_FILENO); }
            if (fd_out != STDOUT_FILENO) { dup2(fd_out, STDOUT_FILENO); }

            int P;
            for (P = 0; P < pipes_count; ++P)
            {
                close(pipes_fd[P][0]);
                close(pipes_fd[P][1]);
            }

            char *argv[100];
            char *cstr = new char [command.length()+1];
            std::strcpy (cstr, command.c_str());
            argv[0] = strtok(cstr, " \t\n\r");
            for (int i = 1; i <= 100; ++i)
            {
                argv[i] = strtok(nullptr, " \t\n\r");
                if (argv[i] == nullptr) { break; }
            }

            if (execvp(argv[0], argv) == -1)
            {
                fprintf(stderr, "Error: Command not found %s.\n", argv[0]);

                exit(EXIT_FAILURE);
            }
            /* NEVER REACH */
            exit(EXIT_FAILURE);
        }
    }

    void execute_commands()
    {
        int C, P;

        int cmd_count = commands.size();

        int pipeline_count = cmd_count - 1;

        int pipes_fd[pipeline_count][2];

        for (P = 0; P < pipeline_count; ++P)
        {
            if (pipe(pipes_fd[P]) == -1)
            {
                fprintf(stderr, "Error: Unable to create pipe. (%d)\n", P);
                exit(EXIT_FAILURE);
            }
        }
        int fromfileFP;
        if (!from_files.empty()){
            fromfileFP = open(from_files.back().c_str(), O_RDONLY);
        }

        std::vector<int> tofileFP;
        for (const std::string& file: to_files) {
            tofileFP.push_back(open(file.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH));
        }

        for (C = 0; C < cmd_count; ++C)
        {
            int fd_in = (C == 0) ? (from_files.empty() ? STDIN_FILENO : fromfileFP) : (pipes_fd[C - 1][0]);
            int fd_out = (C == cmd_count - 1) ? (to_files.empty() ? STDOUT_FILENO : tofileFP.at(tofileFP.size()-1)) : (pipes_fd[C][1]);

            create_proc(commands.at(C), fd_in, fd_out, pipeline_count, pipes_fd);
        }

        for (P = 0; P < pipeline_count; ++P)
        {
            close(pipes_fd[P][0]);
            close(pipes_fd[P][1]);
        }
        close(fromfileFP);

        for (const int fp: tofileFP) {
            close(fp);
        }

        for (C = 0; C < cmd_count; ++C)
        {
            int status;
            wait(&status);
            if (status!=EXIT_SUCCESS){
                exit(EXIT_FAILURE);
            }
        }
    }
};



int main(){
    while (true){
        std::string line;
        std::cout << "$ ";
        getline(std::cin, line);
        if (trim_copy(line) == "exit"){
            break;
        }else if(trim_copy(line).empty()){
            continue;
        }
        ParsedCommands parsedCommands(line);
        parsedCommands.execute_commands();
    }
    return 0;
}

