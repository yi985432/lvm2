#include "lvmetad-client.h"

int main() {
	daemon_handle h = lvmetad_open();
	int i;
	for (i = 0; i < 5; ++i ) {
		daemon_reply reply = daemon_send_simple(h, "hello world", "param = %d", 3, NULL);
		fprintf(stderr, "daemon says: %s\n", reply.buffer);
	}
	daemon_close(h);
	return 0;
}
