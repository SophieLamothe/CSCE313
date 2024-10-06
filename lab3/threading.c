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
        volatile int32_t task_indx = -1; //Marked as volatile for clobbered warning
        volatile int i = 0; 
        for(i = 0; i < NUM_CTX; i++){
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
        if(getcontext(&contexts[task_indx].context) == -1){
                return -1; 
        } 

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
        //Check if no task is running
        if(current_context_idx == NUM_CTX){
                return -1; 
        }

        //Take  snapshot of current task's context
        getcontext(&contexts[current_context_idx].context); 
        
        //Save current context index
        int prev_context_idx = current_context_idx; 

        //Search for the next valid task
        for(int i = 1; i <= NUM_CTX; i++){
                //Need to wrap around once it reaches end of context array
                int next_context_idx = (current_context_idx + i) % NUM_CTX;
                if(contexts[next_context_idx].state == VALID){
                        current_context_idx = (uint8_t)next_context_idx;
                        break; 
                }
        }

        //Check for edge case of no valid tasks
        if(current_context_idx == prev_context_idx){
                return -1; 
        }

        //Perform context switch
        if(swapcontext(&contexts[prev_context_idx].context, &contexts[current_context_idx].context) == -1){
                return -1; 
        }

        //Return number of valid tasks
        int valid_tasks = 0; 
        for(int i = 0; i < NUM_CTX; i++){
                if(contexts[i].state == VALID && i != current_context_idx){
                        valid_tasks++;
                }
        }
        
        return valid_tasks;
}

void t_finish()
{
        //Mark current task as done
        int task_idx = current_context_idx;

        //Free the stack
        free(contexts[task_idx].context.uc_stack.ss_sp); 

        //Reset context entry to all zeros
        memset(&contexts[task_idx], 0, sizeof(struct worker_context)); 

        //Mark context as DONE
        contexts[task_idx].state = DONE; 

        //Yield to the next task
        t_yield(); 
}
