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

static void *safe_malloc(size_t size)
{
    void *pointer = malloc(size);

    if (pointer == NULL)
    {
        printf("Fatal: memory allocation failed\n");

        exit(EXIT_FAILURE);
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

    map->bucket_count = bucket_count;
    map->buckets = (ProcessControlBlock **)safe_malloc(sizeof(ProcessControlBlock *) * bucket_count);

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
    pcb->state = PROCESS_STATE_READY;
    pcb->completion_time = -1;
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

static void run_fcfs_simulation(PidHashMap *pcbs_map, LinkedQueue *ready_queue, LinkedQueue *waiting_queue, LinkedQueue *terminated_queue, KillEvent *kill_events_head)
{
    int system_tick = 0;
    ProcessControlBlock *currently_running = NULL;
    int active_processes_remaining = 0;

    for (int bucket_index = 0; bucket_index < pcbs_map->bucket_count; bucket_index++)
    {
        ProcessControlBlock *item = pcbs_map->buckets[bucket_index];

        while (item != NULL)
        {
            active_processes_remaining++;
            item = item->next_in_hash;
        }
    }

    if (active_processes_remaining == 0)
    {
        return;
    }

    while (1)
    {
        KillEvent *events_to_apply = pop_kill_events_at_tick(&kill_events_head, system_tick);
        KillEvent *current_kill_event = events_to_apply;

        while (current_kill_event != NULL)
        {
            int kill_pid = current_kill_event->pid;
            ProcessControlBlock *target_pcb = hashmap_find(pcbs_map, kill_pid);

            if (target_pcb == NULL)
            {
                fprintf(stderr, "Warning: KILL event at tick %d for PID %d - process not found.\n", system_tick, kill_pid);
            }
            else
            {
                if (currently_running != NULL && currently_running->pid == kill_pid)
                {
                    currently_running->state = PROCESS_STATE_KILLED;
                    currently_running->completion_time = system_tick;
                    currently_running->turnaround_time = -1;
                    currently_running->waiting_time = -1;
                    enqueue_process(terminated_queue, currently_running);
                    hashmap_remove(pcbs_map, kill_pid);
                    currently_running = NULL;
                    active_processes_remaining--;
                }
                else
                {
                    int removed_from_ready = remove_process_from_queue_by_pid(ready_queue, kill_pid);

                    if (removed_from_ready)
                    {
                        target_pcb->state = PROCESS_STATE_KILLED;
                        target_pcb->completion_time = system_tick;
                        target_pcb->turnaround_time = -1;
                        target_pcb->waiting_time = -1;
                        enqueue_process(terminated_queue, target_pcb);
                        hashmap_remove(pcbs_map, kill_pid);
                        active_processes_remaining--;
                    }
                    else
                    {
                        int removed_from_waiting = remove_process_from_queue_by_pid(waiting_queue, kill_pid);

                        if (removed_from_waiting)
                        {
                            target_pcb->state = PROCESS_STATE_KILLED;
                            target_pcb->completion_time = system_tick;
                            target_pcb->turnaround_time = -1;
                            target_pcb->waiting_time = -1;
                            enqueue_process(terminated_queue, target_pcb);
                            hashmap_remove(pcbs_map, kill_pid);
                            active_processes_remaining--;
                        }
                        else
                        {
                            if (target_pcb->state == PROCESS_STATE_TERMINATED || target_pcb->state == PROCESS_STATE_KILLED)
                            {
                            }
                            else
                            {
                                target_pcb->state = PROCESS_STATE_KILLED;
                                target_pcb->completion_time = system_tick;
                                target_pcb->turnaround_time = -1;
                                target_pcb->waiting_time = -1;
                                enqueue_process(terminated_queue, target_pcb);
                                hashmap_remove(pcbs_map, kill_pid);
                                active_processes_remaining--;
                            }
                        }
                    }
                }
            }
            current_kill_event = current_kill_event->next;
        }

        free_kill_event_list(events_to_apply);

        if (currently_running != NULL && currently_running->state == PROCESS_STATE_KILLED)
        {
            currently_running = NULL;
        }

        if (currently_running == NULL)
        {
            currently_running = dequeue_non_killed(ready_queue);

            if (currently_running != NULL)
            {
                currently_running->state = PROCESS_STATE_RUNNING;
            }
        }

        if (currently_running != NULL)
        {
            currently_running->cpu_executed += 1;

            if (!currently_running->has_started_io && currently_running->io_start_time >= 0 && currently_running->cpu_executed == currently_running->io_start_time)
            {
                currently_running->has_started_io = 1;
                currently_running->io_remaining = currently_running->io_duration;
                currently_running->io_completed_at_tick = -1;
                currently_running->state = PROCESS_STATE_WAITING;
                enqueue_process(waiting_queue, currently_running);
                currently_running = NULL;
            }
            else if (currently_running != NULL && currently_running->cpu_executed >= currently_running->total_cpu_burst)
            {
                currently_running->state = PROCESS_STATE_TERMINATED;
                currently_running->completion_time = system_tick + 1;
                currently_running->turnaround_time = currently_running->completion_time;
                currently_running->waiting_time = currently_running->turnaround_time - currently_running->total_cpu_burst;
                enqueue_process(terminated_queue, currently_running);
                hashmap_remove(pcbs_map, currently_running->pid);
                active_processes_remaining--;
                currently_running = NULL;
            }
        }

        QueueNode *waiting_cursor = waiting_queue->front;

        while (waiting_cursor != NULL)
        {
            ProcessControlBlock *waiting_pcb = waiting_cursor->pcb;

            if (waiting_pcb->state == PROCESS_STATE_KILLED)
            {
                waiting_cursor = waiting_cursor->next;
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
            waiting_cursor = waiting_cursor->next;
        }

        move_io_completed_to_ready(waiting_queue, ready_queue, system_tick);

        if (active_processes_remaining <= 0 && currently_running == NULL && is_queue_empty(ready_queue) && is_queue_empty(waiting_queue))
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

    ProcessControlBlock **sorted_list = (ProcessControlBlock **)safe_malloc(sizeof(ProcessControlBlock *) * process_count);

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
                fprintf(stderr, "Error: Invalid KILL format.\n");
                continue;
            }

            int pid_value;
            int kill_tick;

            if (!parse_integer_strict(pid_token, &pid_value) || !parse_integer_strict(time_token, &kill_tick))
            {
                fprintf(stderr, "Error: Invalid KILL values.\n");
                continue;
            }

            if (kill_tick < 0)
            {
                fprintf(stderr, "Error: Kill tick must be >= 0.\n");
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
                fprintf(stderr, "Error: Invalid process format.\n");
                continue;
            }

            int pid_value;
            int cpu_burst;
            int io_start;
            int io_duration;

            if (!parse_integer_strict(pid_token, &pid_value))
            {
                fprintf(stderr, "Error: Invalid PID.\n");
                continue;
            }

            if (!parse_integer_strict(cpu_token, &cpu_burst))
            {
                fprintf(stderr, "Error: Invalid CPU burst.\n");
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
                    fprintf(stderr, "Error: Invalid IO start.\n");
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
                    fprintf(stderr, "Error: Invalid IO duration.\n");
                    continue;
                }
            }

            if (cpu_burst <= 0)
            {
                fprintf(stderr, "Error: CPU burst must be > 0.\n");
                continue;
            }

            if (io_start < -1)
            {
                fprintf(stderr, "Error: IO start invalid.\n");
                continue;
            }

            if (io_duration < 0)
            {
                fprintf(stderr, "Error: IO duration invalid.\n");
                continue;
            }

            if (hashmap_find(pcbs_map, pid_value) != NULL)
            {
                fprintf(stderr, "Error: Duplicate PID.\n");
                continue;
            }

            ProcessControlBlock *pcb = create_process_control_block(process_name_local, pid_value, cpu_burst, io_start, io_duration);
            hashmap_insert(pcbs_map, pcb);
            enqueue_process(&ready_queue, pcb);
        }
    }

    run_fcfs_simulation(pcbs_map, &ready_queue, &waiting_queue, &terminated_queue, kill_events_head);
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
