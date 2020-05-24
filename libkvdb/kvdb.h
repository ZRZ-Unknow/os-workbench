struct kvdb;
struct kvdb *kvdb_open(const char *filename);
int kvdb_close(struct kvdb *db);
int kvdb_put(struct kvdb *db, const char *key, const char *value);
char *kvdb_get(struct kvdb *db, const char *key);
