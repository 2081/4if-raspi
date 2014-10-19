#include "sched.h"
#include "phyAlloc.h"
#include "hw.h"

pcb_cycle* g_process_list_current = NULL; 

void init_ctx( ctx_s* ctx, func_t f, unsigned int stack_size)
{
	ctx->sp_end = phyAlloc_alloc(stack_size);
	ctx->sp = ctx->sp_end + stack_size*MEM_UNIT;
	ctx->pc = f;
}

/*void __attribute__((naked)) switch_to( ctx_s* ctx )
{
	__asm("push {r0-r12}");
	__asm("mov %0, lr" : "=r"(g_current_ctx->pc));
	__asm("mov %0, sp" : "=r"(g_current_ctx->sp));	
	
	g_current_ctx = ctx;

	// WARNING : can't access ctx after sp was changed
	// (it's a local variable so it relies on the stack)

	__asm("mov sp, %0" : : "r"(ctx->sp));
	__asm("pop {r0-r12}");
	__asm("bx %0" : : "r"(g_current_ctx->pc)); // warning here
}*/

// PCB

void init_pcb( struct pcb_s* pcb, ctx_s* ctx, func_t fct, void* args)
{
	init_ctx(ctx, fct, STACK_SIZE);
	pcb->ctx  = ctx;
	pcb->entry_point  = fct;
	pcb->args = args;
	pcb->state = NEW;
}

// PCB LIST
void pcb_cycle_add( pcb_cycle* list, struct pcb_s* pcb )
{
	if( list == NULL )
	{
		g_process_list_current = list = ALLOC(pcb_cycle);
		list->next = NULL;
		list->prev = NULL;
	}
	if( list->pcb == NULL )
	{
		list->pcb = pcb;
		list->next = list;
		list->prev = list;
	} else {
		pcb_cycle* pcbl = ALLOC(pcb_cycle);
		
		pcbl->pcb = pcb;

		pcbl->prev = list->prev;
		list->prev->next = pcbl;
		pcbl->next = list;
		list->prev = pcbl;
	}
}

void pcb_cycle_remove( pcb_cycle* element)
{
	if(element == NULL)
		return;

	if(element->next == element)//Only element in the list
	{
		g_process_list_current = NULL;
	} else {
		if(element == g_process_list_current)
		{
			g_process_list_current = g_process_list_current->next;
		}
		element->next->prev = element->prev;
		element->prev->next = element->next;
	}
}

inline struct pcb_s* current_pcb()
{
	return g_process_list_current->pcb;
}

inline ctx_s* current_ctx()
{
	return g_process_list_current->pcb->ctx;
}

// ALL



struct pcb_s* next_process()
{
	return NULL;
}

void start_sched()
{
	ENABLE_IRQ();
	set_tick_and_enable_timer();

	//struct pcb_s* pcb = ALLOC(struct pcb_s);
	//pcb_cycle_add(g_process_list_current, pcb);

}
void create_process(func_t f, void* args, unsigned int stack_size)
{
	struct pcb_s* pcb = ALLOC(struct pcb_s);
	pcb_cycle_add(g_process_list_current, pcb);
	init_pcb( pcb, ALLOC(ctx_s), f,args);
}

void start_current_process()
{
	//pcb_cycle* pcbc = g_process_list_current;
	
	
	__asm("mov %0, sp" : "=r"(g_process_list_current->pcb->sp_return));
	__asm("mov sp, %0" : : "r"(g_process_list_current->pcb->ctx->sp));
	//ENABLE_IRQ();
	__asm("push {pc}");
	__asm("rfeia sp!");

	set_tick_and_enable_timer();

	g_process_list_current->pcb->entry_point();
	
	DISABLE_IRQ();
	//__asm("sub lr, lr, #4");
	//__asm("srsdb sp!, #0x13");
	//__asm("cps #0x13");

	__asm("mov sp, %0" : : "r"(g_process_list_current->pcb->sp_return));
}

void elect()
{
	g_process_list_current = g_process_list_current->next;
}

void __attribute__((naked)) ctx_switch_from_irq()
//TODO : POURQUOI ON PEUT PAS APPELER DES PUTAINS DE GETTERS !!
{
	DISABLE_IRQ();
	__asm("sub lr, lr, #4");
	__asm("srsdb sp!, #0x13");
	__asm("cps #0x13");

	if( g_process_list_current->pcb != NULL && g_process_list_current->pcb->state != NEW)
	{
		__asm("push {r0-r12}");
		__asm("mov %0, lr" : "=r"(g_process_list_current->pcb->ctx->pc));
		__asm("mov %0, sp" : "=r"(g_process_list_current->pcb->ctx->sp));
	} else {
		//TODO : sp main -> lr
	}
	elect();
	if( g_process_list_current->pcb->state == NEW )
	{
		g_process_list_current->pcb->state = RUNNING;
		start_current_process(); // Updates the current pcb
		g_process_list_current->pcb->state = TERMINATED;
		__asm("push {r0-r12}");
		__asm("mov %0, lr" : "=r"(g_process_list_current->pcb->ctx->pc));
		__asm("mov %0, sp" : "=r"(g_process_list_current->pcb->ctx->sp));
		elect();
		// The old process is dead, long live the new process
	}
	if (g_process_list_current->pcb->state == TERMINATED)
	{
		pcb_cycle* pcbc = g_process_list_current;

		pcb_cycle_remove(pcbc);
		FREE(pcbc->pcb->ctx->sp_end, STACK_SIZE);
		FREE(pcbc->pcb->ctx, ctx_s);
		FREE(pcbc->pcb, struct pcb_s);
		FREE(pcbc, pcb_cycle);
	}
	if(g_process_list_current == NULL)
		return; // no more

	// WARNING : can't access ctx after sp was changed
	// (it's a local variable so it relies on the stack)

	__asm("mov sp, %0" : : "r"(g_process_list_current->pcb->ctx->sp));
	__asm("mov lr, %0" : : "r"(g_process_list_current->pcb->ctx->pc));
	// ^-- bx avec cette syntaxe se sert d'un registre
	__asm("pop {r0-r12}");
	
	__asm("push {lr}");
	set_tick_and_enable_timer();
	__asm("rfeia sp!");
	//__asm("bx lr");
}


