#include <iostream>
#define PROC_NAME_LEN 10
#define RES_LEN 10
enum proc_state
{
    ready, block, running;
}
struct res_struct
{

} struct proc_struct
{
    int pid;                          // Process ID
    enum proc_state state;            // Process state
    proc_struct *parent;              // the parent process
    char name[PROC_NAME_LEN + 1];     // Process name
    int priority;                     // Process priority
    res_struct resoures[RES_LEN + 1]; // occupied resource
} int main()
{
    return 0;
}
