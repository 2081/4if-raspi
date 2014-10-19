#ifndef _SCHED_H_
#define _SCHED_H_

#define STACK_SIZE 	8192
#define MEM_UNIT 	4
#define REG_COUNT 	13
#define NULL 		0

#define ALLOC(X) 	(phyAlloc_alloc(sizeof( X )))
#define FREE(P,X)	(phyAlloc_free(P,sizeof(X)))

typedef void (*func_t)(void);


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/**
 * Structure ctx_s
 * Handles a context's values :
	-> sp : stack pointer, value of the assembly variable sp when
		the context was created or left.
	-> pc : program counter, value to load in pc when restoring
		this context
	-> sp_end : intial value of sp, used to free the stack memory.
 **/
typedef struct ctx_s_
{
	void* sp;
	void* pc;
	void* sp_end;
} ctx_s;

// Initialize a context with the given values
void init_ctx( ctx_s* ctx, func_t f, unsigned int stack_size);

//void __attribute__((naked)) switch_to( ctx_s* ctx );

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/**
 * Structure pcb_s
 * Process control block, used to manage a context :
	-> ctx : reference to the context of the process
	-> entry_point : "main" function of the process
	-> args : pointer to the arguments for the entry_point
	-> state : state of the process.
 **/
typedef enum { NEW, READY, RUNNING, WAITING, TERMINATED } process_state;

struct pcb_s {
	ctx_s* ctx;
	func_t entry_point;
	void* args;
	process_state state;
	void* sp_return;
};

void init_pcb( struct pcb_s* pcb, ctx_s* ctx, func_t f, void* args);



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/**
 * Type pcb_cycle (struct pcb_cycle_)
 * Cycle of PCBs
	-> pcb : reference to the node's pcb
	-> next : pointer to the next node (can be self)
	-> prev : pointer to the previous node (can be self)
 **/
typedef struct pcb_cycle_ {
	struct pcb_s* pcb;
	struct pcb_cycle_* next;
	struct pcb_cycle_* prev;


} pcb_cycle;

void pcb_cycle_add( pcb_cycle* list, struct pcb_s* pcb);
void pcb_cycle_remove( pcb_cycle* element );

pcb_cycle* g_process_list_current;

inline struct pcb_s* current_pcb();
inline ctx_s* current_ctx();

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

ctx_s* g_current_ctx;


// ALL
void start_sched();
void create_process(func_t f, void* args, unsigned int stack_size);
void start_current_process();
void elect();
//void __attribute__((naked)) ctx_switch();
void __attribute__((naked)) ctx_switch_from_irq();



#endif



