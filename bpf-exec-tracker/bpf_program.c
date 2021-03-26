#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

struct {
	__uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
	__uint(key_size, sizeof(__u32));
	__uint(value_size, sizeof(__u32));
} dude_perf SEC(".maps");

struct sys_enter_execve_ctx {
	__u16 common_type;
	__u8 common_flag;
	__u8 common_preempt_count;
	__u32 common_pid;
	
	__u32 __syscall_nr;
	__u8* filename;
	__u8** argv;
	__u8** envp;
};

struct process_info {
	__u64 type;
	__u64 id;
	__u8 data[256];
};

SEC("tracepoint/syscalls/sys_enter_execve")
int dude_test(struct sys_enter_execve_ctx *ctx) {
	__u64 id = bpf_get_current_pid_tgid();
	
	char msg[] = "A new process spawned: %u, %s";
	bpf_trace_printk(msg, sizeof(msg), id, ctx->filename);
	
	{
		struct process_info process;
		process.type = 0;
		process.id = id;
		int err = bpf_probe_read(process.data, sizeof(process.data), ctx->filename);
		
		if (err != 0) {
			char msg[] = "Error reading process exec %d";
			bpf_trace_printk(msg, sizeof(msg), err);
			return 0;
		}
		
		bpf_perf_event_output(ctx, &dude_perf, BPF_F_CURRENT_CPU, &process, sizeof(struct process_info));
	}
	
	for (int i = 0; i < 100; i++) {
		void* currentArg;
		int err = bpf_probe_read(&currentArg, sizeof(__u64), ctx->argv + i);
		
		if (err != 0) {
			char msg[] = "Error reading arg %d %d";
			bpf_trace_printk(msg, sizeof(msg), i, err);
			return 0;
		}
		
		if (!currentArg) {
			break;
		}
		
		struct process_info process;
		process.type = 1;
		process.id = id;
		
		int result = bpf_probe_read_str(process.data, sizeof(process.data), currentArg);
		
		if (result < 0) {
			char msg[] = "Error reading arg string %d %d";
			bpf_trace_printk(msg, sizeof(msg), i, err);
			return 0;
		}
		
		bpf_perf_event_output(ctx, &dude_perf, BPF_F_CURRENT_CPU, &process, sizeof(struct process_info));
	}
	
	for (int i = 0; i < 100; i++) {
		void* currentArg;
		int err = bpf_probe_read(&currentArg, sizeof(__u64), ctx->envp + i);
		
		if (err != 0) {
			char msg[] = "Error reading env %d %d";
			bpf_trace_printk(msg, sizeof(msg), i, err);
			return 0;
		}
		
		if (!currentArg) {
			break;
		}
		
		struct process_info process;
		process.type = 2;
		process.id = id;
		
		int result = bpf_probe_read_str(process.data, sizeof(process.data), currentArg);
		
		if (result < 0) {
			char msg[] = "Error reading env string %d %d";
			bpf_trace_printk(msg, sizeof(msg), i, err);
			return 0;
		}
		
		bpf_perf_event_output(ctx, &dude_perf, BPF_F_CURRENT_CPU, &process, sizeof(struct process_info));
	}
	
	return 0;
}

char _license[] SEC("license") = "GPL";
