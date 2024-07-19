
void label_init(void);

int label_load(const char *filename);

const char *label_lookup(int addr);

int label_byname(const char *name, int *value);

