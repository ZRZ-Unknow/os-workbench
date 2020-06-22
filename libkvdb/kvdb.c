#include <stdio.h>
#include <assert.h>

struct kvdb {
  // your definition here
};

struct kvdb *kvdb_open(const char *filename) {
  return NULL;
}

int kvdb_close(struct kvdb *db) {
  return -1;
}

int kvdb_put(struct kvdb *db, const char *key, const char *value) {
  return -1;
}

char *kvdb_get(struct kvdb *db, const char *key) {
  return NULL;
}
