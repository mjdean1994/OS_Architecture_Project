int read_sector(int sector_number, unsigned char* buffer);
int write_sector(int sector_number, unsigned char* buffer);

unsigned int get_fat_entry(int fat_entry_number, unsigned char* fat);
void set_fat_entry(int fat_entry_number, int value, unsigned char* fat);
