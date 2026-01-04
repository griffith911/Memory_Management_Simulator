#include <iostream>
#include <string>
#include "memory_allocator/memory.hpp"
#include "virtual_memory/virtual_memory.hpp"
using namespace std;

void stats (Memory* m, virtual_memory* vm) {
	if (m) {
		m->memory_stats();
	}
	if (vm) {
		vm->ram_stats();
		vm->lvl1();
		vm->lvl2();
		return;
	}
}
void dump (Memory* m, virtual_memory* vm) {
	if (m) {
		m->memory_dump();
	}
	if (vm) {
		vm->used_pages();
		vm->used_frames();
	}
}
void clear_all (Memory* m, virtual_memory* vm) {
	if (m)
		m->clear_memory();
	if (vm) {
		vm->clear_ram();
		if (vm->level1) vm->level1->clear_cache();
		if (vm->level2) vm->level2->clear_cache();
	}
	cout << "Everything cleared!" << endl;
}
vector<string> split (string s) {
	vector<string> cmds;
	string temp;
	for (char i : s) {
		if (i == ' ') {
			if (temp != "") cmds.push_back(temp);
			temp.clear();
		}
		else {
			temp.push_back(i);
		}
	}
	if (temp != "") cmds.push_back(temp);
	return cmds;
}
void run () {
	Memory* M1 = NULL;
	virtual_memory* MMU = NULL;
	for (int i = 0; i < 10000; i++) {
		string s;
		getline(cin, s);
		vector<string> cmd = split(s);
		if (cmd.size() == 0) continue;
		if (cmd[0] == "init") {
			if (cmd.size() < 2) continue;
			if (cmd[1] == "memory") {
				if (cmd.size() < 3) continue;
				int size = stoi(cmd[2]);
				if (M1) delete M1;
				M1 = new Memory(size);
				cout << "Memory Inititated." << endl;
			}
			else if (cmd[1] == "RAM" || cmd[1] == "ram") {
				if (cmd.size() < 4) continue;
				int rs = stoi(cmd[2]), ps = stoi(cmd[3]);
				if (M1 == NULL) {
					cout << "Error: Memory Heap not created" << endl;
					continue;
				}
				if (MMU) delete MMU;
				MMU = new virtual_memory(M1->TOTAL_MEMORY,rs,ps);
				cout << "RAM Inititated" << endl;
			}
			else if (cmd[1] == "cache") {
				if (cmd.size() < 6) continue;
				int level = stoi(cmd[2]);
				if (level == 1) {
					int cache1_size = stoi(cmd[3]), cache1_block_size = stoi(cmd[4]), cache1_associativity = stoi(cmd[5]);
					if (M1 == NULL) {
						cout << "Error: Memory Heap not created" << endl;
						continue;
					}
					if (MMU == NULL) {
						cout << "Error: RAM not created" << endl;
						continue;
					}
					MMU->level1 = new cache_lvl(cache1_size,cache1_block_size,cache1_associativity,1);
					cout << "Cache Level 1 inititated." << endl;
				}
				else if (level == 2) {
					int cache2_size = stoi(cmd[3]), cache2_block_size = stoi(cmd[4]), cache2_associativity = stoi(cmd[5]);
					if (M1 == NULL) {
						cout << "Error: Memory Heap not created" << endl;
						continue;
					}
					if (MMU == NULL) {
						cout << "Error: RAM not created" << endl;
						continue;
					}
					MMU->level2 = new cache_lvl(cache2_size,cache2_block_size,cache2_associativity,2);
					cout << "Cache Level 2 inititated." << endl;
				}
			}
		}
		else if (cmd[0] == "set") {
			if (cmd.size() < 3) continue;
			string s = cmd[1], allo = cmd[2];
			if (M1 == NULL) {
				cout << "Error: Memory not inititated" << endl;
				continue;
			}
			if (allo == "first_fit") M1->allocator = 1;
			if (allo == "best_fit") M1->allocator = 2;
			if (allo == "worst_fit") M1->allocator = 3;
		}
		else if (cmd[0] == "malloc") {
			if (cmd.size() < 2) continue;
			int size = stoi(cmd[1]);
			if (M1 == NULL) {
				cout << "Error: Memory not inititated" << endl;
				continue;
			}
			if (M1->allocator == 1) M1->malloc_first(size);
			if (M1->allocator == 2) M1->malloc_best(size);
			if (M1->allocator == 3) M1->malloc_worst(size);
		}
		else if (cmd[0] == "free") {
			if (cmd.size() < 2) continue;
			int BLOCK_ID = stoi(cmd[1]);
			if (M1 == NULL) {
				cout << "Error: Memory not inititated" << endl;
				continue;
			}
			M1->free_block(BLOCK_ID);
		}
		else if (cmd[0] == "read") {
			if (cmd.size() < 2) continue;
			int address = stoi(cmd[1]);
			if (MMU == NULL) {
				cout << "Error: RAM not inititated" << endl;
				continue;
			}
			MMU->total_read(address);
		}
		else if (cmd[0] == "clear") {
			if (cmd.size() < 2) continue;
			if (cmd[1] == "ram" || cmd[1] == "RAM") {
				if (MMU)
					MMU->clear_ram();
			}
			else if (cmd[1] == "cache") {
				if (MMU == NULL) continue;
				if (MMU->level1) MMU->level1->clear_cache();
				if (MMU->level2) MMU->level2->clear_cache();
			}
			else if (cmd[1] == "memory") {
				if (M1)
					M1->clear_memory();
			}
			else if (cmd[1] == "all") {
				clear_all(M1, MMU);
			}
		}
		else if (cmd[0] == "dump") {
			dump(M1, MMU);
		}
		else if (cmd[0] == "stats") {
			stats(M1, MMU);
		}
		else if (cmd[0] == "exit") {
			break;
		}
	}
}
