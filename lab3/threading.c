#include <threading.h>

void t_init()
{
        //Initialize all task contexts
        for(int i = 0; i < NUM_CTX; i++){
                //All contexts are invalid initially
                contexts[i].state = INVALID;
        }
        //Initially an invalid index for no current task
        current_context_idx = NUM_CTX;
}

int32_t t_create(fptr foo, int32_t arg1, int32_t arg2)
{
        //Find unused slot in contexts array
        int32_t task_indx = -1;
        for(int i = 0; i < NUM_CTX; i++){
                if(contexts[i].state == INVALID){
                        task_indx = i;
                        break; 
                }
        }

        //No available task slot
        if(task_indx == -1){
                return -1;
        }

        //Initialize context for the new task
        getcontext(&contexts[task_indx].context); 

        //Allocate memory for the stack
        contexts[task_indx].context.uc_stack.ss_sp = malloc(STK_SZ);
        contexts[task_indx].context.uc_stack.ss_size = STK_SZ;
        contexts[task_indx].context.uc_stack.ss_flags = 0;
        contexts[task_indx].context.uc_link = NULL; 

        //Set the new context to executed 'foo' w/ its args
        makecontext(&contexts[task_indx].context, (void (*)()) foo, 2, arg1, arg2); 

        //Don't forget to marke the context as VALID
        contexts[task_indx].state = VALID; 

        //Return 0 on success
        return 0;
}       

int32_t t_yield()
{
        // TODO:
}

void t_finish()
{
        // TODO:
}
