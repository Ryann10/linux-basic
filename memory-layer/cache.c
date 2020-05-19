/**
 * 캐시 메모리 영향으로 프로세스가 접근하는 데이터 사이즈에 따라 데이터를 읽고 쓰는 시간이 어떻게 변화하는지 확인
 *
 * 1. 명령의 첫 번째 파라미터 입력 값을 사이즈로(킬로바이트 단위) 메모리를 확보
 * 2. 확보한 메모리 영역 안에 정해진 횟수 만큼 시퀀셜 접근sequential access 시도
 * 3. 한 번 접근할 때마다 걸린 소요 시간을 표시
 */
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#define CACHE_LINE_SIZE 64
#define NLOOP (4 * 1024UL * 1024 * 1024)
#define NSECS_PER_SEC 1000000000UL

static inline long diff_nsec(struct timespec before, struct timespec after)
{
  return ((after.tv_sec * NSECS_PER_SEC + after.tv_nsec) - (before.tv_sec * NSECS_PER_SEC + before.tv_nsec));
}

int main(int argc, char *argv[])
{
  char *progname;
  progname = argv[0];

  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <size[KB]>\n", progname);
    exit(EXIT_FAILURE);
  }

  register int size;
  size = atoi(argv[1]) * 1024;

  if (!size)
  {
    fprintf(stderr, "size should be >= 1: %d\n", size);
    exit(EXIT_FAILURE);
  }

  char *buffer;
  buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (buffer == (void *)-1)
    err(EXIT_FAILURE, "mmap() failed");

  struct timespec before, after;

  clock_gettime(CLOCK_MONOTONIC, &before);

  int i;
  for (i = 0; i < NLOOP / (size / CACHE_LINE_SIZE); i++)
  {
    long j;
    for (j = 0; j < size; j += CACHE_LINE_SIZE)
      buffer[j] = 0;
  }

  clock_gettime(CLOCK_MONOTONIC, &after);
  printf("%f\n", (double)diff_nsec(before, after) / NLOOP);

  if (munmap(buffer, size) == -1)
    err(EXIT_FAILURE, "munmap() failed");

  exit(EXIT_SUCCESS);
}