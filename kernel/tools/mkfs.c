#include <user.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#define IMG_SIZE (64 * 1024 * 1024)

int main(int argc, char *argv[]) {
  int fd;
  uint8_t *disk;

  // TODO: argument parsing

  assert((fd = open(argv[1], O_RDWR)) > 0);
  assert((ftruncate(fd, IMG_SIZE)) == 0);
  assert((disk = mmap(NULL, IMG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) != (void *)-1);

  // TODO: mkfs

  munmap(disk, IMG_SIZE);
  close(fd);
}
