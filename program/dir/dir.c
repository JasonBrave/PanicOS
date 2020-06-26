#include <holeos.h>

int main() {
	int handle = dir_open("/");
	if (handle < 0) {
		printf(1, "dir_open failed with %d\n", handle);
		proc_exit();
	}

	char filename[256];
	while (dir_read(handle, filename)) {
		printf(1, "%s\n", filename);
	}
	dir_close(handle);
	proc_exit();
}
