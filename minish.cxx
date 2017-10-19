#include <iostream> 
#include <string>
#include <string.h>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <iterator>
#include <unistd.h>
#include <sys/wait.h>


using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::string;

namespace minish {
    

    struct shell_input
    {
            std::vector<string> v;
            bool eof_bit;
            shell_input(std::vector<string> _v, bool _b) : v(_v), eof_bit(_b) {};
    };

    bool call_process(std::vector<string> v_of_s);
    shell_input shell_command_io();
    void in_namespace_main(std::vector<string> args);

} // namespace minish

int main(int argc, char** argv)
{
    std::vector<std::string> a(argc);
    for(size_t i = 0; i < argc; ++i)  a.push_back(argv[i]);
    minish::in_namespace_main(a);
    return 0;
}

minish::shell_input minish::shell_command_io()
{
    const string shell_ppt {"[minish]> "};
    string shell_in;
    std::vector<string> v;
    cout << shell_ppt;
    std::getline(cin,shell_in);
    std::istringstream s(shell_in);
    while(s >> shell_in) {
        cout << "Adding " << shell_in << " to args vector." << endl;
        v.push_back(shell_in);
    }
    cout << "Returning vector to main." << endl;
    if(cin.eof()) {
        cin.clear();
        cout << "EOF was input." << endl;
        return minish::shell_input(v,false);
    }
    return minish::shell_input(v,true);
}

void minish::in_namespace_main(std::vector<string> args) 
{
    const std::unordered_map<string,unsigned> minish_calls
    {   
        {"exit",1},
        {"ls",10},
        {"ps",10}
    };

    bool should_run {true};
    while(should_run) {
        
        auto shell_user_input = shell_command_io();
        should_run = shell_user_input.eof_bit;
        if(!should_run) break;
        cout << "EOF bit off." << endl;
        auto it = std::begin(shell_user_input.v);
        if(it == std::end(shell_user_input.v)) break;
        cout << "Parsing shell input." << endl;
        auto process_string = shell_user_input.v[0];

        
        auto call_lookup = minish_calls.find(process_string);
        unsigned call_code;
        if(call_lookup != std::end(minish_calls)) call_code = call_lookup->second;
        cout << "Looked up call code -> " << call_code << "." << endl;

        switch(call_code) {
            
            case 1  : { cout << "User requested exit." << endl;
                        should_run = false;  
                        break;
                      }

            case 10 : { should_run = minish::call_process(shell_user_input.v);
                        break;
                      }

            default : { cout << "Command not recognized" << endl; }

        }
    }   //  while should_run
    cout << "Exiting." << endl;
}

bool minish::call_process(std::vector<string> v_of_s)
{   
    auto it = std::begin(v_of_s);
    cout << "Calling " << *it; 
    if(v_of_s.size()==1) 
    { 
        cout << "." << endl;    
    } else { 
        cout << " with args:";
        for(; it != std::end(v_of_s); ++it) cout << " " << *it;
        cout << "." << endl;
    }

    // copy first string in vector to const c-string process name
    const char* process = v_of_s[0].c_str();
    // convert vector of strings to const array of non-const c-strings
    char** const arg_vector = new char*[v_of_s.size()+1];
    for(size_t i = 0; i < v_of_s.size(); ++i)
        arg_vector[i] = strdup(v_of_s[i].c_str());
    arg_vector[v_of_s.size()] = 0;

    pid_t call_pid = fork();
    if(call_pid < 0) {
        cout << "Call to " << v_of_s[0] << " failed." << endl;
        return false;
    } else if(call_pid == 0) {
        int ret_fm_exec = execvp(process, arg_vector);
        if(ret_fm_exec < 0) {
            cout << "I am the child process but I did not call " << process << endl;
            return false;
        }
    } else {
        for(size_t i = 0; i < v_of_s.size(); ++i)
            delete[] arg_vector[i];
        delete[] arg_vector;
        cout << "Freeing C-Strings from heap." << endl;
        cout << "[minish] just called child process " << v_of_s[0] << " with PID: " << call_pid << "." << endl;
        int wc = wait(0);
        return true;
    }
}

