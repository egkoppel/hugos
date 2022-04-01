bits 64

section .text
global task_switch
global task_init
extern current_task_ptr_read ; Task*(*)(void)
extern current_task_ptr_write ; void(*)(std::shared_ptr<Task>*)

; void task_init(void)
; takes 3 args, 1st arg in r13, 2nd arg in r14, 3rd arg in r15
task_init:
	mov rdi, r13
	mov rsi, r14
	mov rdx, r15
	ret

; void task_switch(std::shared_ptr<Task> *new_task)
; *new_task in rdi
task_switch:
	push rbx
	push rbp
	push r12
	push r13
	push r14
	push r15

	push rdi
	call current_task_ptr_read ; rax = Task*
	mov qword [rax + 2*8], rsp ; save stack pointer
	pop rdi

	; load new task
	call current_task_ptr_write ; current_task = *new_task (rdi)
	call current_task_ptr_read ; rax = Task* (new_task)
	mov rsp, qword [rax + 2*8] ; restore stack pointer
	mov rax, qword [rax + 3*8] ; load new cr3 ito rax
	mov rcx, cr3 ; load old cr3 into rcx

	cmp rcx, rax ; check if tlb flush needed
	je .after_addr_space_switch
	mov cr3, rax ; set new cr3
.after_addr_space_switch:
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rbx
	ret