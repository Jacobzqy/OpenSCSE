#include <iostream>
#define PROC_NAME_LEN 10
#define RES_LEN 4
int idx = 0;
int current_id = -1;
proc_struct* current_proc = NULL;
proc_struct root = {-1, null, NULL, "root", 2, };
enum proc_state
{
    null, ready, block, running;
};
struct res_struct
{

}; 
struct proc_struct
{
    int pid;                          // Process ID
    enum proc_state state;            // Process state
    proc_struct *parent;              // the parent process
    char name[PROC_NAME_LEN + 1];     // Process name
    int priority;                     // Process priority
    res_struct resoures[RES_LEN + 1]; // occupied resource
};
void proc_create(char* name, int prority)
{
    proc_struct* pcb = new(proc_struct);
    pcb->name = name;
    pcb->priority = prority;
    pcb->pid = idx++;
    if(current_id != -1)
    {
        pcb->parent = current_proc;
        pcb->state = ready;
    }
    else
    {
        pcb->parent = root;
        current_id = pcb->pid;
        current_proc = pcb;
        pcb->state = running;
    }
}