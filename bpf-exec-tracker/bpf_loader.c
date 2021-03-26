#include <stdio.h>
#include <bpf/libbpf.h>
#include <unistd.h>

struct process_info {
	__u64 type;
	__u64 id;
	__u8 data[256];
};

static void dude_output(void *ctx, int cpu, void *dataPtr, __u32 size) {
	struct process_info* data = (struct process_info*) dataPtr;
	
	if (data->type == 0) {
		printf("New process %llu %s [%d]\n", data->id, data->data, size);
	} else if (data->type == 1) {
		printf("  Arg %llu %s [%d]\n", data->id, data->data, size);
	} else if (data->type == 2) {
		printf("  Env %llu %s [%d]\n", data->id, data->data, size);
	}
}


void dude_lost(void *ctx, int cpu, __u64 lost_cnt)
{
	printf("Lost %llu\n", lost_cnt);
}

int main(int argc, char *argv[]) {
	char* bpfProgramFilename = "bpf_program.o";
	
	struct bpf_object *obj = bpf_object__open_file(bpfProgramFilename, NULL);
	
	if (libbpf_get_error(obj)) {
		printf("ERROR: opening BPF object file failed\n");
		return 1;
	}
	
	if (bpf_object__load(obj)) {
		fprintf(stderr, "ERROR: loading BPF object file failed\n");
		return 1;
	}
	
	int perf_map_fd = bpf_object__find_map_fd_by_name(obj, "dude_perf");
	
	if (perf_map_fd < 0) {
		printf("ERROR: finding a perf_map_fd in obj file failed\n");
		return 1;
	}
	
	struct perf_buffer_opts pb_opts = {};
	pb_opts.sample_cb = dude_output;
	pb_opts.lost_cb = dude_lost;
	struct perf_buffer *pb = perf_buffer__new(perf_map_fd, 1, &pb_opts);
	int ret = libbpf_get_error(pb);
	if (ret) {
		printf("failed to setup perf_buffer: %d\n", ret);
		return 1;
	}
	
	struct bpf_program *prog = bpf_object__find_program_by_name(obj, "dude_test");
	
	if (libbpf_get_error(prog)) {
		printf("ERROR: finding a prog in obj file failed\n");
		return 1;
	}
	
	struct bpf_link *link = bpf_program__attach(prog);
	
	if (libbpf_get_error(link)) {
		printf("ERROR: bpf_program__attach failed\n");
		return 1;
	}
	
	int result = perf_buffer__poll(pb, 1);
	
	while (result >= 0) {
		result = perf_buffer__poll(pb, 1);
	}
	
	printf("Done\n");
	
	getchar();
	
	return 0;
}
