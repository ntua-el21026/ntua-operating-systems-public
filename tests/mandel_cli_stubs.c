/* Bounded rendering support for command-line and synchronization tests. */
#include <sys/types.h>
#include <stddef.h>
int mandel_iterations_at_point(double x, double y, int max) { return (int)(x + y); }
unsigned char xterm_color(int color) { return color; }
void set_xterm_color(int fd, unsigned char color) {}
void reset_xterm_color(int fd) {}
