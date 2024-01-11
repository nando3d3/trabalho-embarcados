#ifndef NVS
#define NVS

void read_nvs(char* variable_name, void* value, size_t size);
void write_nvs(char* variable_name, void* value, size_t size);

#endif