
void idle_task(void) {

	asm volatile("idle_loop:\n"
			"wfi\n"
			"b idle_loop\n"
			:::);
}
