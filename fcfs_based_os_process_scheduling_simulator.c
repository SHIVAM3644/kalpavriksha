#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define HASHMAP_BUCKET_COUNT 1031
#define MAX_INPUT_LINE_LENGTH 512
#define MAX_PROCESS_NAME_LENGTH 128

typedef enum ProcessStateEnum
{
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_WAITING,
    PROCESS_STATE_TERMINATED,
    PROCESS_STATE_KILLED
} ProcessStateEnum;

typedef struct ProcessControlBlock
{
    char process_name[MAX_PROCESS_NAME_LENGTH];
    int pid;
    int total_cpu_burst;
    int cpu_executed;
    int io_start_time;
    int io_duration;
    int io_remaining;
    int io_completed_at_tick;
    int turnaround_time;
    int waiting_time;
    ProcessStateEnum state;
    int completion_time;
    int has_started_io;
    struct ProcessControlBlock *next_in_hash;
} ProcessControlBlock;

typedef struct QueueNode
{
    ProcessControlBlock *pcb;
    struct QueueNode *next;
} QueueNode;

typedef struct LinkedQueue
{
    QueueNode *front;
    QueueNode *rear;
} LinkedQueue;

typedef struct PidHashMap
{
    ProcessControlBlock **buckets;
    int bucket_count;
} PidHashMap;

typedef struct KillEvent
{
    int pid;
    int tick_time;
    struct KillEvent *next;
} KillEvent;

typedef struct SimulationContext
{
    PidHashMap *pcbs_map;
    LinkedQueue *ready_queue;
    LinkedQueue *waiting_queue;
    LinkedQueue *terminated_queue;
    KillEvent *kill_events_head;
} SimulationContext;

static void *safe_malloc(size_t size)
{
    void *pointer = malloc(size);
    if (pointer == NULL)
    {
        return NULL;
    }
    return pointer;
}

static void trim_whitespace(char *string)
{
    char *start = string;

    while (isspace((unsigned char)*start))
    {
        start++;
    }

    if (start != string)
    {
        memmove(string, start, strlen(start) + 1);
    }

    size_t length = strlen(string);

    while (length > 0 && isspace((unsigned char)string[length - 1]))
    {
        string[--length] = '\0';
    }
}

static void initialize_queue(LinkedQueue *queue)
{
    queue->front = NULL;
    queue->rear = NULL;
}

static int is_queue_empty(const LinkedQueue *queue)
{
    return (queue->front == NULL);
}

static void enqueue_process(LinkedQueue *queue, ProcessControlBlock *pcb)
{
    QueueNode *node = (QueueNode *)safe_malloc(sizeof(QueueNode));

    if (node == NULL)
    {
        printf("Error: could not allocate QueueNode.\n");
        return;
    }

    node->pcb = pcb;
    node->next = NULL;

    if (queue->rear == NULL)
    {
        queue->front = node;
        queue->rear = node;
    }
    else
    {
        queue->rear->next = node;
        queue->rear = node;
    }
}

static ProcessControlBlock *dequeue_process(LinkedQueue *queue)
{
    if (queue->front == NULL)
    {
        return NULL;
    }

    QueueNode *node = queue->front;
    ProcessControlBlock *pcb = node->pcb;

    queue->front = node->next;

    if (queue->front == NULL)
    {
        queue->rear = NULL;
    }

    free(node);
    return pcb;
}

static int remove_process_from_queue_by_pid(LinkedQueue *queue, int pid_to_remove)
{
    QueueNode *previous = NULL;
    QueueNode *current = queue->front;

    while (current != NULL)
    {
        if (current->pcb->pid == pid_to_remove)
        {
            if (previous == NULL)
            {
                queue->front = current->next;
            }
            else
            {
                previous->next = current->next;
            }

            if (current == queue->rear)
            {
                queue->rear = previous;
            }

            free(current);
            return 1;
        }

        previous = current;
        current = current->next;
    }

    return 0;
}

static void move_io_completed_to_ready(LinkedQueue *waiting_queue, LinkedQueue *ready_queue, int current_tick)
{
    QueueNode *previous = NULL;
    QueueNode *current = waiting_queue->front;

    while (current != NULL)
    {
        ProcessControlBlock *current_pcb = current->pcb;
        QueueNode *next_node = current->next;

        if (current_pcb->io_remaining <= 0 && current_pcb->io_completed_at_tick >= 0 && current_tick > current_pcb->io_completed_at_tick)
        {
            if (previous == NULL)
            {
                waiting_queue->front = next_node;
            }
            else
            {
                previous->next = next_node;
            }

            if (current == waiting_queue->rear)
            {
                waiting_queue->rear = previous;
            }

            current_pcb->state = PROCESS_STATE_READY;
            current_pcb->io_completed_at_tick = -1;

            enqueue_process(ready_queue, current_pcb);
            free(current);
        }
        else
        {
            previous = current;
        }

        current = next_node;
    }
}

static PidHashMap *create_pid_hashmap(int bucket_count)
{
    PidHashMap *map = (PidHashMap *)safe_malloc(sizeof(PidHashMap));

    if (map == NULL)
    {
        printf("Error: could not allocate hashmap.\n");
        return NULL;
    }

    map->bucket_count = bucket_count;

    map->buckets = (ProcessControlBlock **)safe_malloc(sizeof(ProcessControlBlock *) * bucket_count);

    if (map->buckets == NULL)
    {
        printf("Error: could not allocate hashmap buckets.\n");
        free(map);
        return NULL;
    }

    for (int bucket_index = 0; bucket_index < bucket_count; bucket_index++)
    {
        map->buckets[bucket_index] = NULL;
    }

    return map;
}

static int hashmap_hash_function(PidHashMap *map, int pid)
{
    long hashed = ((long)pid) % map->bucket_count;

    if (hashed < 0)
    {
        hashed += map->bucket_count;
    }

    return (int)hashed;
}

static void hashmap_insert(PidHashMap *map, ProcessControlBlock *pcb)
{
    int bucket_index = hashmap_hash_function(map, pcb->pid);

    pcb->next_in_hash = map->buckets[bucket_index];
    map->buckets[bucket_index] = pcb;
}

static ProcessControlBlock *hashmap_find(PidHashMap *map, int pid)
{
    int bucket_index = hashmap_hash_function(map, pid);
    ProcessControlBlock *current = map->buckets[bucket_index];

    while (current != NULL)
    {
        if (current->pid == pid)
        {
            return current;
        }

        current = current->next_in_hash;
    }

    return NULL;
}

static int hashmap_remove(PidHashMap *map, int pid)
{
    int bucket_index = hashmap_hash_function(map, pid);

    ProcessControlBlock *previous = NULL;
    ProcessControlBlock *current = map->buckets[bucket_index];

    while (current != NULL)
    {
        if (current->pid == pid)
        {
            if (previous == NULL)
            {
                map->buckets[bucket_index] = current->next_in_hash;
            }
            else
            {
                previous->next_in_hash = current->next_in_hash;
            }

            return 1;
        }

        previous = current;
        current = current->next_in_hash;
    }

    return 0;
}

static ProcessControlBlock *create_process_control_block(const char *name, int pid, int cpu_burst, int io_start_time, int io_duration)
{
    ProcessControlBlock *pcb = (ProcessControlBlock *)safe_malloc(sizeof(ProcessControlBlock));

    if (pcb == NULL)
    {
        printf("Error: could not allocate PCB.\n");
        return NULL;
    }

    strncpy(pcb->process_name, name, MAX_PROCESS_NAME_LENGTH - 1);
    pcb->process_name[MAX_PROCESS_NAME_LENGTH - 1] = '\0';

    pcb->pid = pid;
    pcb->total_cpu_burst = cpu_burst;
    pcb->cpu_executed = 0;

    pcb->io_start_time = io_start_time;
    pcb->io_duration = io_duration;
    pcb->io_remaining = io_duration;
    pcb->io_completed_at_tick = -1;

    pcb->turnaround_time = -1;
    pcb->waiting_time = -1;
    pcb->completion_time = -1;

    pcb->state = PROCESS_STATE_READY;
    pcb->has_started_io = 0;

    pcb->next_in_hash = NULL;

    return pcb;
}

static int parse_integer_strict(const char *token, int *value_out)
{
    if (token == NULL || *token == '\0')
    {
        return 0;
    }

    char *end_pointer = NULL;
    long parsed_value = strtol(token, &end_pointer, 10);

    if (*end_pointer != '\0')
    {
        return 0;
    }

    if (parsed_value < INT_MIN || parsed_value > INT_MAX)
    {
        return 0;
    }

    *value_out = (int)parsed_value;
    return 1;
}

static KillEvent *insert_kill_event_sorted(KillEvent *head, int pid, int tick_time)
{
    KillEvent *new_event = (KillEvent *)safe_malloc(sizeof(KillEvent));

    if (new_event == NULL)
    {
        printf("Error: could not allocate KillEvent.\n");
        return head;
    }

    new_event->pid = pid;
    new_event->tick_time = tick_time;
    new_event->next = NULL;

    if (head == NULL || tick_time < head->tick_time)
    {
        new_event->next = head;
        return new_event;
    }

    KillEvent *previous = NULL;
    KillEvent *current = head;

    while (current != NULL && current->tick_time <= tick_time)
    {
        previous = current;
        current = current->next;
    }

    previous->next = new_event;
    new_event->next = current;

    return head;
}

static KillEvent *pop_kill_events_at_tick(KillEvent **head_reference, int tick)
{
    KillEvent *head = *head_reference;

    if (head == NULL)
    {
        return NULL;
    }

    if (head->tick_time != tick)
    {
        return NULL;
    }

    KillEvent *result_head = head;
    KillEvent *tail = head;

    head = head->next;

    while (tail->next != NULL && tail->next->tick_time == tick)
    {
        tail = tail->next;
    }

    *head_reference = tail->next;
    tail->next = NULL;

    return result_head;
}

static void free_kill_event_list(KillEvent *head)
{
    KillEvent *current = head;

    while (current != NULL)
    {
        KillEvent *next_item = current->next;
        free(current);
        current = next_item;
    }
}

static ProcessControlBlock *dequeue_non_killed(LinkedQueue *ready_queue)
{
    ProcessControlBlock *candidate = NULL;

    while (1)
    {
        candidate = dequeue_process(ready_queue);

        if (candidate == NULL)
        {
            return NULL;
        }

        if (candidate->state != PROCESS_STATE_KILLED)
        {
            return candidate;
        }
        else
        {
            enqueue_process(ready_queue, candidate);
        }
    }
}

static int count_active_processes(PidHashMap *map)
{
    int count = 0;

    for (int bucket_index = 0; bucket_index < map->bucket_count; bucket_index++)
    {
        ProcessControlBlock *item = map->buckets[bucket_index];

        while (item != NULL) 
        { 
            count++; 
            item = item->next_in_hash; 
        }
    }

    return count;
}

static int apply_kill_events(SimulationContext *context, int system_tick, ProcessControlBlock **running_process)
{
    int removed_processes = 0;
    KillEvent *events_to_apply = pop_kill_events_at_tick(&context->kill_events_head, system_tick);
    KillEvent *current_event = events_to_apply;
  
    while (current_event != NULL)
    {
        int kill_pid = current_event->pid;
        ProcessControlBlock *target = hashmap_find(context->pcbs_map, kill_pid);
      
        if (target == NULL)
        {
            printf("Warning: KILL event at tick %d for PID %d - process not found.\n", system_tick, kill_pid);
            current_event = current_event->next;
            continue;
        }
       
        if (*running_process != NULL && (*running_process)->pid == kill_pid)
        {
            (*running_process)->state = PROCESS_STATE_KILLED;
            (*running_process)->completion_time = system_tick;
            (*running_process)->turnaround_time = -1;
            (*running_process)->waiting_time = -1;
            enqueue_process(context->terminated_queue, *running_process);
            hashmap_remove(context->pcbs_map, kill_pid);
            *running_process = NULL;
            removed_processes++;
        }
        else
        {
            int removed_from_ready = remove_process_from_queue_by_pid(context->ready_queue, kill_pid);
            
            if (removed_from_ready)
            {
                target->state = PROCESS_STATE_KILLED;
                target->completion_time = system_tick;
                target->turnaround_time = -1;
                target->waiting_time = -1;
                enqueue_process(context->terminated_queue, target);
                hashmap_remove(context->pcbs_map, kill_pid);
                removed_processes++;
            }
            else
            {
                int removed_from_waiting = remove_process_from_queue_by_pid(context->waiting_queue, kill_pid);

                if (removed_from_waiting)
                {
                    target->state = PROCESS_STATE_KILLED;
                    target->completion_time = system_tick;
                    target->turnaround_time = -1;
                    target->waiting_time = -1;
                    enqueue_process(context->terminated_queue, target);
                    hashmap_remove(context->pcbs_map, kill_pid);
                    removed_processes++;
                }
                else
                {
                    if (target->state != PROCESS_STATE_TERMINATED && target->state != PROCESS_STATE_KILLED)
                    {
                        target->state = PROCESS_STATE_KILLED;
                        target->completion_time = system_tick;
                        target->turnaround_time = -1;
                        target->waiting_time = -1;
                        enqueue_process(context->terminated_queue, target);
                        hashmap_remove(context->pcbs_map, kill_pid);
                        removed_processes++;
                    }
                }
            }
        }
        current_event = current_event->next;
    }

    free_kill_event_list(events_to_apply);
    return removed_processes;
}

static void pick_next_process(SimulationContext *context, ProcessControlBlock **running_process)
{
    if (*running_process != NULL)
    {
        return;
    } 

    ProcessControlBlock *next = dequeue_non_killed(context->ready_queue);

    if (next != NULL)
    {
        next->state = PROCESS_STATE_RUNNING;
        *running_process = next;
    }
}

static int execute_cpu_cycle(SimulationContext *context, ProcessControlBlock **running_process, int system_tick)
{
    if (*running_process == NULL)
    {
        return 0;
    }

    (*running_process)->cpu_executed += 1;

    if (!(*running_process)->has_started_io && (*running_process)->io_start_time >= 0 && (*running_process)->cpu_executed == (*running_process)->io_start_time)
    {
        (*running_process)->has_started_io = 1;
        (*running_process)->io_remaining = (*running_process)->io_duration;
        (*running_process)->io_completed_at_tick = -1;
        (*running_process)->state = PROCESS_STATE_WAITING;
        enqueue_process(context->waiting_queue, *running_process);
        *running_process = NULL;

        return 0;
    }
    if (*running_process != NULL && (*running_process)->cpu_executed >= (*running_process)->total_cpu_burst)
    {
        (*running_process)->state = PROCESS_STATE_TERMINATED;
        (*running_process)->completion_time = system_tick + 1;
        (*running_process)->turnaround_time = (*running_process)->completion_time;
        (*running_process)->waiting_time = (*running_process)->turnaround_time - (*running_process)->total_cpu_burst;
        enqueue_process(context->terminated_queue, *running_process);
        hashmap_remove(context->pcbs_map, (*running_process)->pid);
        *running_process = NULL;
        return 1;
    }
    return 0;
}

static void process_waiting_queue(SimulationContext *context, int system_tick)
{
    QueueNode *cursor = context->waiting_queue->front;

    while (cursor != NULL)
    {
        ProcessControlBlock *waiting_pcb = cursor->pcb;

        if (waiting_pcb->state == PROCESS_STATE_KILLED) 
        { 
            cursor = cursor->next; 
            continue; 
        }

        if (waiting_pcb->io_remaining > 0)
        {
            waiting_pcb->io_remaining -= 1;

            if (waiting_pcb->io_remaining == 0)
            {
                waiting_pcb->io_completed_at_tick = system_tick;
            }
        }
        cursor = cursor->next;
    }
}

static int should_stop_simulation(SimulationContext *context, ProcessControlBlock *running_process, int active_remaining)
{
    if (active_remaining <= 0 && running_process == NULL && is_queue_empty(context->ready_queue) && is_queue_empty(context->waiting_queue))
    {
        return 1;
    }

    return 0;
}

static void run_fcfs_simulation(SimulationContext *context)
{
    int system_tick = 0;
    ProcessControlBlock *currently_running = NULL;
    int active_processes_remaining = count_active_processes(context->pcbs_map);

    if (active_processes_remaining == 0)
    {
        return;
    }

    while (1)
    {
        int removed_by_kill = apply_kill_events(context, system_tick, &currently_running);
        active_processes_remaining -= removed_by_kill;

        if (currently_running != NULL && currently_running->state == PROCESS_STATE_KILLED)
        {
            currently_running = NULL;
        }

        pick_next_process(context, &currently_running);

        int terminated_count = execute_cpu_cycle(context, &currently_running, system_tick);
        active_processes_remaining -= terminated_count;

        process_waiting_queue(context, system_tick);
        move_io_completed_to_ready(context->waiting_queue, context->ready_queue, system_tick);

        if (should_stop_simulation(context, currently_running, active_processes_remaining))
        {
            break;
        }

        system_tick++;
    }
}

static void print_summary_table(const LinkedQueue *terminated_queue)
{
    int process_count = 0;
    QueueNode *iterator = terminated_queue->front;

    while (iterator != NULL) 
    { 
        process_count++;
        iterator = iterator->next; 
    }

    ProcessControlBlock **sorted_list = (ProcessControlBlock **)safe_malloc(sizeof(ProcessControlBlock *) * (process_count > 0 ? process_count : 1));

    if (sorted_list == NULL) 
    { 
        printf("Error: could not allocate summary list.\n"); 
        return; 
    }

    iterator = terminated_queue->front;

    int index_position = 0;
    
    while (iterator != NULL) 
    {
        sorted_list[index_position++] = iterator->pcb; 
        iterator = iterator->next; 
    }
    for (int outer_index = 0; outer_index < process_count - 1; outer_index++)
    {
        for (int inner_index = outer_index + 1; inner_index < process_count; inner_index++)
        {
            if (sorted_list[outer_index]->pid > sorted_list[inner_index]->pid)
            {
                ProcessControlBlock *temporary = sorted_list[outer_index];
                sorted_list[outer_index] = sorted_list[inner_index];
                sorted_list[inner_index] = temporary;
            }
        }
    }

    printf("\nSummary\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("| %-6s | %-16s | %-8s | %-6s | %-10s | %-8s | %-7s |\n",
           "PID", "Name", "CPU", "IO", "Turnaround", "Wait", "Status");
    printf("--------------------------------------------------------------------------------\n");

    for (int print_index = 0; print_index < process_count; print_index++)
    {
        ProcessControlBlock *pcb = sorted_list[print_index];
        int total_io = (pcb->io_duration < 0) ? 0 : pcb->io_duration;

        if (pcb->state == PROCESS_STATE_KILLED)
        {
            char killed_msg[MAX_INPUT_LINE_LENGTH];
            snprintf(killed_msg, sizeof(killed_msg), "KILLED at %d", pcb->completion_time);

            printf("| %-6d | %-16s | %-8d | %-6d | %-10s | %-8s | %-7s |\n",
                   pcb->pid,
                   pcb->process_name,
                   pcb->total_cpu_burst,
                   total_io,
                   "-", "-",
                   killed_msg);
        }
        else
        {
            if (pcb->turnaround_time < 0)
            { 
                pcb->turnaround_time = 0;
            }

            pcb->waiting_time = pcb->turnaround_time - pcb->total_cpu_burst;

            if (pcb->waiting_time < 0)
            {
                pcb->waiting_time = 0;
            }

            printf("| %-6d | %-16s | %-8d | %-6d | %-10d | %-8d | %-7s |\n",
                   pcb->pid,
                   pcb->process_name,
                   pcb->total_cpu_burst,
                   total_io,
                   pcb->turnaround_time,
                   pcb->waiting_time,
                   "OK");
        }
    }

    printf("--------------------------------------------------------------------------------\n");
    free(sorted_list);
}

int main(void)
{
    char input_line[MAX_INPUT_LINE_LENGTH];
    PidHashMap *pcbs_map = create_pid_hashmap(HASHMAP_BUCKET_COUNT);
    LinkedQueue ready_queue;
    LinkedQueue waiting_queue;
    LinkedQueue terminated_queue;

    initialize_queue(&ready_queue);
    initialize_queue(&waiting_queue);
    initialize_queue(&terminated_queue);

    KillEvent *kill_events_head = NULL;

    while (1)
    {
        if (fgets(input_line, sizeof(input_line), stdin) == NULL)
        {
            break;
        }

        trim_whitespace(input_line);

        if (strlen(input_line) == 0)
        {
            break;
        } 

        char copy_of_line[MAX_INPUT_LINE_LENGTH];
        strncpy(copy_of_line, input_line, sizeof(copy_of_line) - 1);
        copy_of_line[sizeof(copy_of_line) - 1] = '\0';

        char *token = strtok(copy_of_line, " \t");

        if (token == NULL)
        {
            continue;
        }

        if (strcasecmp(token, "KILL") == 0)
        {
            char *pid_token = strtok(NULL, " \t");
            char *time_token = strtok(NULL, " \t");

            if (pid_token == NULL || time_token == NULL) 
            {
                printf("Error: Invalid KILL format.\n");
                continue;
            }

            int pid_value;
            int kill_tick;

            if (!parse_integer_strict(pid_token, &pid_value) || !parse_integer_strict(time_token, &kill_tick)) 
            {
                printf("Error: Invalid KILL values.\n");
                continue;
            }

            if (kill_tick < 0) 
            { 
                printf("Error: Kill tick must be >= 0.\n"); 
                continue; 
            }

            kill_events_head = insert_kill_event_sorted(kill_events_head, pid_value, kill_tick);
        }
        else
        {
            char process_name_local[MAX_PROCESS_NAME_LENGTH];
            strncpy(process_name_local, token, MAX_PROCESS_NAME_LENGTH - 1);
            process_name_local[MAX_PROCESS_NAME_LENGTH - 1] = '\0';
            char *pid_token = strtok(NULL, " \t");
            char *cpu_token = strtok(NULL, " \t");
            char *io_start_token = strtok(NULL, " \t");
            char *io_duration_token = strtok(NULL, " \t");

            if (!pid_token || !cpu_token || !io_start_token || !io_duration_token) 
            { 
                printf("Error: Invalid process format.\n"); 
                continue; 
            }

            int pid_value;
            int cpu_burst;
            int io_start;
            int io_duration;

            if (!parse_integer_strict(pid_token, &pid_value)) 
            { 
                printf("Error: Invalid PID.\n"); 
                continue; 
            }

            if (!parse_integer_strict(cpu_token, &cpu_burst)) 
            { 
                printf("Error: Invalid CPU burst.\n"); 
                continue; 
            }

            if (strcmp(io_start_token, "-") == 0)
            {
                io_start = -1;
            }
            else 
            { 
                if (!parse_integer_strict(io_start_token, &io_start)) 
                { 
                    printf("Error: Invalid IO start.\n"); 
                    continue; 
                } 
            }

            if (strcmp(io_duration_token, "-") == 0)
            {
                io_duration = 0;
            }
            else 
            { 
                if (!parse_integer_strict(io_duration_token, &io_duration)) 
                { 
                    printf("Error: Invalid IO duration.\n"); 
                    continue; 
                } 
            }

            if (cpu_burst <= 0) 
            { 
                printf("Error: CPU burst must be > 0.\n"); 
                continue; 
            }

            if (io_start < -1) 
            { 
                printf("Error: IO start invalid.\n"); 
                continue; 
            }

            if (io_duration < 0) 
            { 
                printf("Error: IO duration invalid.\n"); 
                continue; 
            }

            if (hashmap_find(pcbs_map, pid_value) != NULL) 
            { 
                printf("Error: Duplicate PID.\n"); 
                continue; 
            }

            ProcessControlBlock *pcb = create_process_control_block(process_name_local, pid_value, cpu_burst, io_start, io_duration);
            hashmap_insert(pcbs_map, pcb);
            enqueue_process(&ready_queue, pcb);
        }
    }

    SimulationContext simulation;
    simulation.pcbs_map = pcbs_map;
    simulation.ready_queue = &ready_queue;
    simulation.waiting_queue = &waiting_queue;
    simulation.terminated_queue = &terminated_queue;
    simulation.kill_events_head = kill_events_head;
    run_fcfs_simulation(&simulation);
    print_summary_table(&terminated_queue);

    QueueNode *cursor = terminated_queue.front;

    while (cursor != NULL)
    {
        ProcessControlBlock *pcb_to_free = cursor->pcb;
        cursor = cursor->next;
        free(pcb_to_free);
    }

    free(pcbs_map->buckets);
    free(pcbs_map);

    return 0;
}
