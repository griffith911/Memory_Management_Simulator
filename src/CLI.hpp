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
void run () {
	Memory* M1 = NULL;
	virtual_memory* MMU = NULL;
	for (int i = 0; i < 10000; i++) {
		string cmd; cin >> cmd;
		if (cmd == "init") {
			string c2; cin >> c2;
			if (c2 == "memory") {
				int size; cin >> size;
				M1 = new Memory(size);
			}
			else if (c2 == "RAM" || c2 == "ram") {
				int rs, ps; cin >> rs >> ps;
				if (M1 == NULL) {
					cout << "Error: Memory Heap not created" << endl;
					continue;
				}
				MMU = new virtual_memory(M1->TOTAL_MEMORY,rs,ps);
			}
			else if (c2 == "cache") {
				int level; cin >> level;
				if (level == 1) {
					int cache1_size, cache1_block_size, cache1_associativity;
					cin >> cache1_size >> cache1_block_size >> cache1_associativity;
					if (M1 == NULL) {
						cout << "Error: Memory Heap not created" << endl;
						continue;
					}
					if (MMU == NULL) {
						cout << "Error: RAM not created" << endl;
						continue;
					}
					MMU->level1 = new cache_lvl(cache1_size,cache1_block_size,cache1_associativity,1);
				}
				else if (level == 2) {
					int cache2_size, cache2_block_size, cache2_associativity;
					cin >> cache2_size >> cache2_block_size >> cache2_associativity;
					if (M1 == NULL) {
						cout << "Error: Memory Heap not created" << endl;
						continue;
					}
					if (MMU == NULL) {
						cout << "Error: RAM not created" << endl;
						continue;
					}
					MMU->level2 = new cache_lvl(cache2_size,cache2_block_size,cache2_associativity,2);
				}
			}
		}
		else if (cmd == "set") {
			string s, allo; cin >> s >> allo;
			if (M1 == NULL) {
				cout << "Error: Memory not inititated" << endl;
				continue;
			}
			if (allo == "first_fit") M1->allocator = 1;
			if (allo == "best_fit") M1->allocator = 2;
			if (allo == "worst_fit") M1->allocator = 3;
		}
		else if (cmd == "malloc") {
			int size; cin >> size;
			if (M1 == NULL) {
				cout << "Error: Memory not inititated" << endl;
				continue;
			}
			if (M1->allocator == 1) M1->malloc_first(size);
			if (M1->allocator == 2) M1->malloc_best(size);
			if (M1->allocator == 3) M1->malloc_worst(size);
		}
		else if (cmd == "free") {
			int BLOCK_ID; cin >> BLOCK_ID;
			if (M1 == NULL) {
				cout << "Error: Memory not inititated" << endl;
				continue;
			}
			M1->free_block(BLOCK_ID);
		}
		else if (cmd == "read") {
			int address; cin >> address;
			if (MMU == NULL) {
				cout << "Error: RAM not inititated" << endl;
				continue;
			}
			MMU->total_read(address);
		}
		else if (cmd == "dump") {
			dump(M1, MMU);
		}
		else if (cmd == "stats") {
			stats(M1, MMU);
		}
		else if (cmd == "exit") {
			break;
		}
	}
}