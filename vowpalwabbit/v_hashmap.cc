#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "v_array.h"
#include "v_hashmap.h"

/*
size_t myhash(int k)
{
  //return (size_t)(k * 382490328 + 34280);   // ok hash function
  return ((size_t)k) % 3; // crappy hash function
}

bool inteq(int a, int b) { return (a==b); }

void printit(int k, float v) {
  printf("  iter(%d, %g)\n", k, v);
}

void test_v_hashmap()
{
  v_hashmap<int,float> hm = v_hashmap<int,float>(15, FLT_MAX, &inteq);

  for (int k=-5; k<=5; k++)
    printf("get(%d,%zu) -> %g\n", k, myhash(k), hm.get(k, myhash(k)));
  printf("\n");

  for (int k=-5; k<=5; k+=5) {
    printf("put(%d,%zu) <- %g\n", k, myhash(k), (float)k);
    hm.put(k, myhash(k), (float)k);
  }
  printf("\n");
  for (int k=-5; k<=5; k++)
    printf("get(%d,%zu) -> %g\n", k, myhash(k), hm.get(k, myhash(k)));
  printf("\n");

  hm.iter(&printit);
  hm.double_size();
  hm.iter(&printit);

  for (int k=-5; k<=5; k++)
    printf("get(%d,%zu) -> %g\n", k, myhash(k), hm.get(k, myhash(k)));
  printf("\n");

  for (int k=-5; k<=5; k++)
    printf("get(%d,%zu) -> %g\n", k, myhash(k), hm.get(k, myhash(k)));
  printf("\n");

  for (int k=-5; k<=5; k+=2) {
    printf("put(%d,%zu) <- %g\n", k, myhash(k), (float)k);
    hm.put(k, myhash(k), (float)k);
  }
  printf("\n");

  for (int k=-5; k<=5; k++)
    printf("get(%d,%zu) -> %g\n", k, myhash(k), hm.get(k, myhash(k)));
  printf("\n");

  for (int k=-10; k<=10; k+=1) {
    printf("put(%d,%zu) <- %g\n", k, myhash(k), (float)k);
    hm.put(k, myhash(k), (float)k);
  }
  printf("\n");

  for (int k=-5; k<=5; k++)
    printf("get(%d,%zu) -> %g\n", k, myhash(k), hm.get(k, myhash(k)));
  printf("\n");
  hm.iter(&printit);
}

*/
