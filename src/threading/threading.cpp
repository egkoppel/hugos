#include "threading.hpp"
#include <assert.h>

using namespace threads;

atomic_uint_fast64_t threads::next_pid = 1;

Scheduler threads::scheduler = Scheduler();

extern "C" void task_init(void);
extern "C" void task_switch(std::shared_ptr<Task> *new_task);
std::shared_ptr<Task> current_task_ptr;

extern "C" Task* current_task_ptr_read() {
	return current_task_ptr.get();
}

extern "C" void current_task_ptr_write(std::shared_ptr<Task> *t) {
	current_task_ptr = *t;
}

void Scheduler::print_tasks() {
	for (auto& task : tasks) {
		auto state = task->get_state();
		printf("%lli: %s - %s\n", task->get_pid(), task->get_name().c_str(), state == task_state::RUNNING ? "RUNNING" : "BLOCKED");
	}
}

std::shared_ptr<Task> Task::new_kernel_task(std::string name, void(*entry_func)(uint64_t, uint64_t, uint64_t), uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	uint64_t cr3;
	__asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
	auto task = std::make_shared<Task>(name, cr3);
	auto& stack = task->get_stack();

	*((uint64_t*)stack.top - 1) = (uint64_t)entry_func;
	*((uint64_t*)stack.top - 2) = (uint64_t)task_init;
	*((uint64_t*)stack.top - 3) = (uint64_t)0; // rbx
	*((uint64_t*)stack.top - 4) = (uint64_t)0; // rbp
	*((uint64_t*)stack.top - 5) = (uint64_t)0; // r12
	*((uint64_t*)stack.top - 6) = (uint64_t)arg1; // r13 - task_init arg 1
	*((uint64_t*)stack.top - 7) = (uint64_t)arg2; // r14 - task_init arg 2
	*((uint64_t*)stack.top - 8) = (uint64_t)arg3; // r15 - task_init arg 3
	return task;
}

std::shared_ptr<Task> threads::init_multitasking(uint64_t stack_bottom, uint64_t stack_top) {
	uint64_t cr3;
	__asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
	auto kernel_task = std::make_shared<threads::Task>("kernel_task", cr3, stack_top, stack_top);
	current_task_ptr = kernel_task;
	scheduler.lock().add_task(kernel_task);
	return kernel_task;
}

void Scheduler::schedule() {
	if (running_tasks.size() == 0) return; // Only one task - don't need to switch

	auto new_task = running_tasks[0]; // Get first task
	running_tasks.pop_front();

	running_tasks.push_back(current_task_ptr);
	
	task_switch(&new_task);
}

void Scheduler::add_task(std::shared_ptr<Task> task) {
	tasks.push_back(task);
	running_tasks.push_back(task);
}

void Scheduler::block(task_state reason) {
	assert(reason != task_state::READY && reason != task_state::RUNNING);
}

SchedulerLock Scheduler::lock() {
	return SchedulerLock(this);
}

SchedulerLock::SchedulerLock(Scheduler *s): s(s) {
	__asm__ volatile("cli");
	s->IRQ_disable_counter++;
}

SchedulerLock::~SchedulerLock() {
	s->IRQ_disable_counter--;
	if (s->IRQ_disable_counter == 0) __asm__ volatile("sti");
}

void SchedulerLock::print_tasks() { s->print_tasks(); }
void SchedulerLock::add_task(std::shared_ptr<Task> task) { s->add_task(task); }
void SchedulerLock::schedule() { s->schedule(); }
void SchedulerLock::block(task_state reason) { s->block(reason); }
