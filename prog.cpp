#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

// split by space
vector<string> split(string str)
{
    istringstream str_in(str);
    vector<string> rtrn;
    string temp;
    while (str_in >> temp)
    {
        rtrn.push_back(temp);
    }

    return rtrn;
}
// System is a library function that uses system calls in its implementation
int lsh_loop(void)
{
    int BUFFSIZE = 1024;
    int out_socket;
    struct sockaddr_in address;
    string input;
    int TCP_FLAG = 0;
    int std_fd = 400;
    // int curr_output = dup(1);
    // prepare the tcp socket for the local server.
    // if ((out_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    // {
    //     perror("Error: ");
    // }
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0)
    {
        perror("Error: ");
    }

    // if ((out_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    // {
    //     perror("Error: ");
    // }
    // if (connect(out_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    // {
    //     cout << "Connection Failed. Server down?\n";
    // }
    char *buf = (char *)malloc(sizeof(char) * BUFFSIZE);
    while (1)
    {

        // cout<<"yes master?: "; old prompt

        // getcwd(new prompt): a system call.
        input = "";
        char *current_dir = getcwd(buf, BUFFSIZE);
        if (current_dir == NULL && errno != ERANGE)
        {
            perror("Error in getcwd");
            cout << "E" << endl;
        }
        else
        {
            printf("\033[1;34m~%s\033[0m$ ", buf);
        }
        // input proccessing:
        getline(cin, input);
        vector<string> input_split = split(input);

        // menu:
        if (input == "EXIT")
        {
            if (TCP_FLAG == 1)
            {
                close(out_socket);
            }
            exit(EXIT_SUCCESS);
        }
        if (input == "")
        {
            continue;
        }
        else if (input_split[0] == "ECHO")
        {
            // if(send(out_socket,"test",sizeof("test"),0)<0)
            // {
            //     perror("Error: ");
            // }
            for (int i = 1; i < input_split.size(); i++)
            {
                printf("%s ", input_split[i].c_str());
            }
            printf("\n");
        }
        else if (input_split[0] == "TCP" && stoi(input_split[1]) <= 65535 && stoi(input_split[1]) >= 1)
        {
            if (TCP_FLAG == 1)
            {
                close(out_socket);
            }
            if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0)
            {
                perror("Error: ");
            }

            if ((out_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("Error: ");
            }
            if (connect(out_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
            {
                cout << "Connection Failed. Server down?\n";
                continue;
            }
            TCP_FLAG = 1;
            dup2(STDOUT_FILENO, std_fd);
            dup2(out_socket, STDOUT_FILENO);
        }
        else if (input == "DIR")
        {
            // DIR *dirp = opendir(".");
            // dirent *curr_dir = readdir(dirp);

            // while((curr_dir=readdir(dirp))!= NULL)
            // {
            //     cout<<curr_dir->d_name<<endl;
            // }
            // closedir(dirp);
            // system("DIR"); this is the usage of system function as required
            // below is the implementation using forc/execv/wait
            pid_t pid = fork();

            if (pid == 0) // if child
            {
                char *argv[2];
                char command[] = "/bin/dir";
                argv[0] = command;
                argv[1] = NULL;
                execv(argv[0], argv);
            }
            else if (pid > 0) // if parent, wait for the child process to end
            {
                int status;
                waitpid(pid, &status, 0);
            }
        }
        else if (input == "LOCAL")
        {
            if (TCP_FLAG == 1)
            {
                cout << endl;
                cout << "Client disconnected" << endl;
                dup2(std_fd, STDOUT_FILENO);
                close(out_socket);
                TCP_FLAG = 0;
            }
        }
        // CD is a system call and not a library function
        else if (input_split[0] == "CD")
        {
            // CHDIR is a system call function and not a library functtion.
            const char *cstr = input_split[1].c_str();
            if (chdir(cstr) < 0)
            {
                cout << "Changing directory failed" << endl;
            }

            //**
            // char command[256] = "cd ";
            // for (int i = 0; i < 256 && i<input_split[1].size(); i++)
            // {
            //     command[i+3] = input_split[1][i];
            // }
            // cout<<command<<endl;

            // if(system("cd ..")<0)
            // {
            //     cout<<"Changing directory failed"<<endl;
            // }
        }
        else if (input_split[0] == "COPY")
        {
            if (input_split.size() != 3)
            {
                cout << "Error in syntax. Try COPY SRC DST" << endl;
            }
            else
            {
                char src_name[input_split[1].size()];
                char dst_name[input_split[2].size()];
                strcpy(src_name, input_split[1].c_str());
                strcpy(dst_name, input_split[2].c_str());

                FILE *src = fopen(src_name, "r"); // read source
                FILE *dst = fopen(dst_name, "w");

                char fbuffer[BUFFSIZE];
                memset(fbuffer, '\0', BUFFSIZE);

                while (fread(fbuffer, 1, BUFFSIZE, src) == BUFFSIZE)
                {
                    fwrite(fbuffer, 1, BUFFSIZE, dst);
                    memset(fbuffer, '\0', BUFFSIZE);
                }
                // for when the buffer was not filled completely, we want to write only relevant chars.
                char chr[1];
                for (int i = 0; i < BUFFSIZE; i++)
                {
                    if (fbuffer[i] == '\0')
                    {
                        break;
                    }
                    chr[0] = fbuffer[i];
                    fwrite(chr, 1, 1, dst);
                }
                fclose(src);
                fclose(dst);
            }
        }
        // This delete implementation is using a system call, that is unlink.
        else if (input_split[0] == "DELETE")
        {
            if (input_split.size() != 2)
            {
                cout << "Error in syntax. Try DELETE filename" << endl;
            }
            else
            {
                char path[input_split[1].size()];
                strcpy(path, input_split[1].c_str());
                if (unlink(path) < 0)
                {
                    perror("Error occured; ");
                }
            }
        }
        else
        {
            cout << "Unknown command\n";
        }
    }
}

int main()
{
    lsh_loop();
    // string in;
    // getline(cin,in);
    // vector<string> sp = split(in);
    // for (int i = 0; i < sp.size(); i++)
    // {
    //     cout<<sp[i]<<",";
    // }
    // cout<<endl;

    return 0;
}