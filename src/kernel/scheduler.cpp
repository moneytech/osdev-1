#include "kernel.hpp"
#include "ministl.hpp"
#include "locks.hpp"

extern char _start, _end;
const uint32_t default_priority=10;
void *sch_stack;

struct sch_start{
	void (*ptr)(void*);
	void *param;
};

struct sch_stackinfo{
	uint32_t ss;
	uint32_t esp;
} __attribute__((packed));

sch_stackinfo curstack;

struct sch_thread{
	sch_stackinfo stack;
	bool runnable;
	uint32_t magic;
	void *stackptr;
	void *stackbase;
	bool to_be_deleted;
	sch_start *start;
	uint32_t priority;
	uint32_t dynpriority;
	uint64_t ext_id;
	pid_t pid;
	sch_blockcheck blockcheck;
	void *bc_param;
	uint32_t eip;
	bool abortable;
	int abortlevel;
    bool user_abort;
	sch_thread *next;
	uint32_t sch_cycle;
	thread_msg_status::Enum msgstatus;
};

vector<sch_thread*> *threads;
sch_thread *current_thread;
uint64_t current_thread_id;
sch_thread *reaper_thread;
sch_thread *prescheduler_thread;
uint64_t cur_ext_id;
sch_thread *idle_thread;
uint64_t sch_zero=0;

void thread_reaper(void*);
void sch_threadtest();
void sch_idlethread(void*);
void sch_prescheduler_thread(void*);
static sch_thread *sch_get(uint64_t ext_id);

lock sch_lock;
bool sch_inited=false;
static const uint32_t cstart=5;
static uint32_t counter=cstart;

char *sch_threads_infofs(){
	char *buffer=(char*)malloc(4096);
	memset(buffer, 0, 4096);
	sprintf(buffer, "# ID, PID, priority, addr, run, alevel\n");
	{hold_lock hl(sch_lock);
		for(size_t i=0; i<threads->size(); ++i){
			sch_thread *t=(*threads)[i];
			sprintf(&buffer[strlen(buffer)],"%i, %i, %i, %x, %i, %i\n", (int)t->ext_id, (int)t->pid, t->priority, t->eip,
				(bool)t->runnable, t->abortlevel);
		}
    }
    return buffer;
}

void sch_init(){
	dbgout("SCH: Init\n");
	init_lock(sch_lock);
    uint16_t value=0xFFFF;
    dbgpf("SCH: Value: %i\n", (int)value);
	outb(0x43, 0x36);
	outb(0x40, value & 0xFF);
	outb(0x40, (value >> 8) & 0xFF);
	threads=new vector<sch_thread*>();
	sch_stack=(char*)malloc(4096)+4096;
	sch_thread *mainthread=new sch_thread();
	mainthread->runnable=true;
	mainthread->to_be_deleted=false;
	mainthread->priority=default_priority;
	mainthread->dynpriority=0;
	mainthread->magic=0xF00D;
	mainthread->pid=proc_current_pid;
	mainthread->blockcheck=NULL;
	mainthread->bc_param=NULL;
	mainthread->abortlevel=1;
	mainthread->abortable=false;
	mainthread->pid=0;
	mainthread->sch_cycle=0;
	mainthread->msgstatus=thread_msg_status::Normal;
	current_thread_id=mainthread->ext_id=++cur_ext_id;
	threads->push_back(mainthread);
	current_thread=(*threads)[threads->size()-1];
	uint64_t idle_thread_id=sch_new_thread(&sch_idlethread, NULL, 1024);
	idle_thread=sch_get(idle_thread_id);
	uint64_t reaper_thread_id=sch_new_thread(&thread_reaper, NULL, 4096);
	reaper_thread=sch_get(reaper_thread_id);
	uint64_t prescheduler_id=sch_new_thread(&sch_prescheduler_thread, NULL, 1024);
	prescheduler_thread=sch_get(prescheduler_id);
	current_thread->next=prescheduler_thread;
	//sch_threadtest();
	irq_handle(0, &sch_isr);
	sch_inited=true;
	IRQ_clear_mask(0);
	dbgout("SCH: Init complete.\n");
	infofs_register("THREADS", &sch_threads_infofs);
}

void test_priority(void *params){
	uint32_t *p=(uint32_t*)params;
	char c=(char)(p[0]);
	uint32_t priority=p[1];
	current_thread->priority=priority;
	free(params);
	while(true){
		printf("%c", c);
		sch_yield();
	}
}

void sch_threadtest(){
	uint32_t* p1=(uint32_t*)malloc(sizeof(uint32_t)*2);
	p1[0]='.';
	p1[1]=200;
	sch_new_thread(&test_priority, (void*)p1);
	uint32_t* p2=(uint32_t*)malloc(sizeof(uint32_t)*2);
	p2[0]='!';
	p2[1]=400;
	sch_new_thread(&test_priority, (void*)p2);
}

void sch_idlethread(void*){
	sch_set_priority(0xFFFFFFFF);
	while(true)asm volatile("hlt");
}

extern "C" void sch_wrapper(){
    uint64_t ext_id;
    sch_start *start;
    {
        hold_lock hl(sch_lock);
        start = current_thread->start;
        ext_id=(int)current_thread->ext_id;
    }
	dbgpf("SCH: Starting new thread %x (%i) at %x (param %x) [%x].\n", current_thread, (int)ext_id, start->ptr, start->param, start);
    void (*entry)(void*) = start->ptr;
    void *param = start->param;
    free(start);
	entry(param);
	sch_end_thread();
}

uint64_t sch_new_thread(void (*ptr)(void*), void *param, size_t stack_size){
	sch_thread *newthread=new sch_thread();
	sch_start *start=(sch_start*)malloc(sizeof(sch_start));
	start->ptr=ptr;
	start->param=param;
	uint32_t stack=(uint32_t)malloc(stack_size);
	newthread->stackptr=(void*)stack;
	stack+=stack_size;
	stack-=4;
	*(uint32_t*)stack=(uint32_t)&sch_wrapper;
	newthread->stackbase=(void*)stack;
	newthread->stack.ss=0x10;
	newthread->stack.esp=stack;
	newthread->start=start;
	newthread->runnable=true;
	newthread->to_be_deleted=false;
	newthread->magic=0xBABE;
	newthread->priority=default_priority;
	newthread->dynpriority=0;
	newthread->blockcheck=NULL;
	newthread->bc_param=NULL;
	newthread->abortlevel=1;
	newthread->abortable=false;
	newthread->pid=0;
    newthread->user_abort=false;
	newthread->next=NULL;
	newthread->sch_cycle=0;
	newthread->msgstatus=thread_msg_status::Normal;
    take_lock_exclusive(sch_lock);
	newthread->ext_id=++cur_ext_id;
	threads->push_back(newthread);
	release_lock(sch_lock);
	return newthread->ext_id;
}

void thread_reaper(void*){
    sch_set_priority(1);
	while(true){
		bool changed=true;
		while(changed){
			hold_lock lck(sch_lock);
			changed=false;
			for(size_t i=0; i<threads->size(); ++i){
				if((*threads)[i]->to_be_deleted){
					sch_thread *ptr=(*threads)[i];
					uint64_t id=(*threads)[i]->ext_id;
                    void *stackptr=(*threads)[i]->stackptr;
					threads->erase(i);
                    release_lock(sch_lock);
                    free(stackptr);
					delete ptr;
                    take_lock_exclusive(sch_lock);
					changed=true;
					dbgpf("SCH: Reaped %i (%i) [%x].\n", i, (uint32_t)id, stackptr);
					break;
				}
			}
		}
		sch_block();
	}
}

void sch_end_thread(){
    proc_remove_thread(sch_get_id(), current_thread->pid);
    take_lock_exclusive(sch_lock);
	current_thread->runnable=false;
	current_thread->to_be_deleted=true;
	reaper_thread->runnable=true;
	release_lock(sch_lock);
	sch_yield();
	panic("SCH: Attempt to run to_be_deleted thread!");
}

inline void out_regs(const irq_regs &ctx){
	dbgpf("SCH: INTERRUPT %x\n", ctx.int_no);
	dbgpf("EAX: %x EBX: %x ECX: %x EDX: %x\n", ctx.eax, ctx.ebx, ctx.ecx, ctx.edx);
	dbgpf("EDI: %x ESI: %x EBP: %x ESP: %x\n", ctx.edi, ctx.esi, ctx.ebp, ctx.esp);
	dbgpf("EIP: %x CS: %x SS: %x\n", ctx.eip, ctx.cs, ctx.ss);
	dbgpf("EFLAGS: %x ORESP: %x\n", ctx.eflags, ctx.useresp);
}

static bool sch_find_thread(sch_thread *&torun, uint32_t cycle){
	static uint32_t lcycle=0;
	//Find runnable threads and minimum dynamic priority
	int nrunnables=0;
	uint32_t min=0xFFFFFFFF;
	for(size_t i=0; i<threads->size(); ++i){
		//Priority 0xFFFFFFFF == "idle", only run when nothing else is available.
		if(!(*threads)[i]->priority==0xFFFFFFFF) (*threads)[i]->dynpriority=0xFFFFFFFF;
	    if(lcycle != cycle && !(*threads)[i]->runnable && (*threads)[i]->blockcheck!=NULL){
	        (*threads)[i]->runnable=(*threads)[i]->blockcheck((*threads)[i]->bc_param);
	    }
		if((*threads)[i]->runnable){
			if(!(*threads)[i]->priority) panic("(SCH) Thread priority 0 is not allowed.\n");
			nrunnables++;
			if((*threads)[i]->dynpriority < min) min=(*threads)[i]->dynpriority;
		}
	}
	lcycle=cycle;

	//If there are no runnable threads, halt. Hopefully an interrupt will awaken one soon...
	if(nrunnables==0){
		return false;
	}
	
	//Subtract minimum dynamic priority from all threads. If there is now a thread with dynamic priority 0
	//that isn't the current thread, record it
	bool foundtorun=false;
	for(size_t i=0; i<(*threads).size(); ++i){
		if((*threads)[i]->runnable){
			if((*threads)[i]->dynpriority) (*threads)[i]->dynpriority-=min;
			if((*threads)[i]!=current_thread && (*threads)[i]->dynpriority==0){
				foundtorun=true;
				torun=(*threads)[i];
			}
		}
	}
	if(foundtorun){
		return true;
	}else{
		torun=current_thread;
		return true;
	}
}

extern "C" sch_stackinfo *sch_schedule(uint32_t ss, uint32_t esp){
	if(!are_interrupts_enabled()) panic("(SCH) Interrupts disabled in scheduler!\n");
	if(get_lock_owner(sch_lock)!=current_thread_id) panic("(SCH) Bad scheduler locking detected!\n");
	
	//Save current thread's state
	current_thread->stack.ss=ss;
	current_thread->stack.esp=esp;

	//Get next thread
	sch_thread *torun=current_thread->next;

	//Clear old thread's next value, to prevent accidents
	current_thread->next=NULL;
	//If the thread exists, but isn't runnable (made non-runnable since last preschedule), skip it
	if(torun && !torun->runnable) torun=torun->next;
	//If there is no next, run the prescheduler instead
	if(!torun) torun=prescheduler_thread;
	current_thread=torun;
	curstack=current_thread->stack;
	if(!torun->ext_id) panic("(SCH) Thread with no ID?");
	lock_transfer(sch_lock, torun->ext_id);
	current_thread_id=torun->ext_id;
	proc_switch_sch(current_thread->pid, false);
	gdt_set_kernel_stack(current_thread->stackbase);
	return &curstack;
}

extern "C" uint32_t sch_dolock(){
    if(!are_interrupts_enabled()) enable_interrupts();//panic("(SCH) Attempt to yield while interrupts are disabled!");
	if(!try_take_lock_exclusive(sch_lock)){
		dbgout("SCH: Scheduler run while locked!\n");
		return 0;
	}
	return 1;
}

extern "C" void sch_unlock(){
	release_lock(sch_lock);
}

void sch_isr(int irq, isr_regs *regs){
    irq_ack(irq);
	if(try_take_lock_exclusive(sch_lock)){
        counter--;
        if(!counter) {
            counter=cstart;
            sch_abortable(true);
			current_thread->eip = regs->eip;
            release_lock(sch_lock);
            enable_interrupts();
            sch_yield();
            disable_interrupts();
            sch_abortable(false);
        }else{
            release_lock(sch_lock);
        }
	}
}

const uint64_t &sch_get_id(){
	if(!sch_inited) return sch_zero;
	return current_thread_id;
}

void sch_set_priority(uint32_t pri){
	hold_lock hl(sch_lock);
	current_thread->priority=pri;
}

void sch_block(){
    take_lock_recursive(sch_lock);
	current_thread->runnable=false;
	release_lock(sch_lock);
	sch_yield();
}

void sch_unblock(uint64_t ext_id){
	hold_lock hl(sch_lock);
	for(size_t i=0; i<threads->size(); ++i){
		if((*threads)[i]->ext_id==ext_id && !(*threads)[i]->to_be_deleted){
			(*threads)[i]->runnable=true;
			break;
		}
	}
}

bool sch_active(){
	return sch_inited;
}

void sch_setpid(pid_t pid){
	hold_lock hl(sch_lock, false);
	current_thread->pid=pid;
}

void sch_setblock(sch_blockcheck check, void *param){
    { hold_lock hl(sch_lock);
		current_thread->blockcheck=check;
		current_thread->bc_param=param;
    }
    sch_abortable(true);
    sch_block();
    sch_clearblock();
    sch_abortable(false);
}

void sch_clearblock(){
	hold_lock hl(sch_lock, false);
	current_thread->blockcheck=NULL;
	current_thread->bc_param=NULL;
}

bool sch_wait_blockcheck(void *p){
	uint64_t &ext_id=*(uint64_t*)p;
	for(size_t i=0; i<threads->size(); ++i){
		if((*threads)[i]->ext_id==ext_id){
			return false;
		}
	}
	return true;
}

void sch_wait(uint64_t ext_id){
    take_lock_exclusive(sch_lock);
	for(size_t i=0; i<threads->size(); ++i){
		if((*threads)[i]->ext_id==ext_id){
            release_lock(sch_lock);
			sch_setblock(sch_wait_blockcheck, (void*)&ext_id);
            take_lock_exclusive(sch_lock);
			break;
		}
	}
    release_lock(sch_lock);
}

extern "C" void sch_update_eip(uint32_t eip){
    hold_lock hl(sch_lock, false);
	current_thread->eip=eip;
}

uint32_t sch_get_eip(bool lock){
    if(lock) take_lock_recursive(sch_lock);
    uint32_t ret=current_thread->eip;
    if(lock) release_lock(sch_lock);
    return ret;
}

void sch_abortable(bool abortable){
    int alevel;
    {
        hold_lock hl(sch_lock, false);
		current_thread->abortlevel += abortable ? -1 : 1;
        alevel = current_thread->abortlevel;
    }
	if(alevel<=0){
        hold_lock hl(sch_lock, false);
		current_thread->abortlevel=0;
		current_thread->abortable=true;
	}else{
        hold_lock hl(sch_lock, false);
		current_thread->abortable=false;
	}
}

bool sch_abort_blockcheck(void *p){
	uint64_t &ext_id=*(uint64_t*)p;
	for(size_t i=0; i<threads->size(); ++i){
		if((*threads)[i]->ext_id==ext_id){
			return (*threads)[i]->abortable;
		}
	}
	return true;
}

void sch_abort(uint64_t ext_id){
	bool tryagain=true;
	while(tryagain){
        take_lock_recursive(sch_lock);
		bool found=false;
		for(size_t i=0; i<threads->size(); ++i){
			if((*threads)[i]->ext_id==ext_id){
				found=true;
				if((*threads)[i]->abortable){
					(*threads)[i]->runnable=false;
					(*threads)[i]->to_be_deleted=true;
					reaper_thread->runnable=true;
					tryagain=false;
				}else{
                    (*threads)[i]->user_abort=true;
                }
			}
		}
		release_lock(sch_lock);
		if(!found) tryagain=false;
		if(tryagain){
			sch_setblock(&sch_abort_blockcheck, (void*)&ext_id);
		}
	}
}

bool sch_can_lock(){
    if(!try_take_lock_exclusive(sch_lock)) return false;
    release_lock(sch_lock);
    return true;
}

bool sch_user_abort(){
    take_lock_recursive(sch_lock);
    bool ret=current_thread->user_abort;
    release_lock(sch_lock);
    return ret;
}

void sch_prescheduler_thread(void*){
	current_thread->runnable=false;
	uint32_t cycle=0;
	while(true){
		cycle++;
		take_lock_exclusive(sch_lock);
		sch_thread *current=current_thread;
		sch_thread *next=NULL;
		uint32_t count=0;
		while(sch_find_thread(next, cycle)){
			if(next->sch_cycle==cycle || (count && next==idle_thread)){
				break;
			}
			current->next=next;
			current=current->next;
			current->dynpriority=current->priority;
			current->sch_cycle=cycle;
			count++;
		}
		release_lock(sch_lock);
		sch_yield();
	}
}

static sch_thread *sch_get(uint64_t ext_id){
	hold_lock hl(sch_lock);
	for(size_t i=0; i<threads->size(); ++i){
		if((*threads)[i]->ext_id==ext_id) return (*threads)[i];
	}
	return NULL;
}

void sch_set_msgstaus(thread_msg_status::Enum status, uint64_t ext_id){
	current_thread->msgstatus=status;
}

thread_msg_status::Enum sch_get_msgstatus(uint64_t ext_id){
	return current_thread->msgstatus;
}
