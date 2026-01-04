#include <vector>
#include <deque>
#include <cmath>
#include <iostream>
using namespace std;

struct cacheline {
	int tag;
	int data;
	cacheline (int tg, int dt) {
		tag = tg;
		data = dt;
	}
};
struct cache_lvl {
	int block_size;
	int associativity;
	int cache_size;
	int sets, set_bits, offset_bits;
	int level_id = 0;
	int hits = 0;
	int miss = 0;
	vector<deque<cacheline>> cache;
	cache_lvl (int sz, int bl_sz, int ass, int lvl_id) {
		cache_size = sz;
		block_size = bl_sz;
		associativity = ass;
		level_id = lvl_id;
		int row_size = associativity * block_size;
		sets = (cache_size + row_size - 1) / row_size;
		cache.resize(sets);
		set_bits = (int)ceil(log2(sets));
		offset_bits = (int)ceil(log2(block_size));
	}
	int cache_insert (int phy_address) {
		int offset = phy_address & ((1 << offset_bits) - 1);
		int set_index = (phy_address >> offset_bits) & ((1 << set_bits) - 1);
		int tag = phy_address >> (offset_bits + set_bits);
		deque<cacheline> *row = &cache[set_index];
		bool found = false;
		for (auto it = row->begin(); it != row->end(); it++) {
			if (it->tag == tag) {
				found = 1;
				cacheline hit = *it;
				row->erase(it);
				row->push_front(hit);
				break;
			}
		}
		int evicted_addr = -1;
		if (!found) {
			if (row->size() == associativity) {
				int evicted = row->back().tag;
				row->pop_back();
				evicted_addr = (evicted << (set_bits + offset_bits)) | (set_index << offset_bits);
			}
			row->push_front(cacheline(tag, 0));
		}
		return evicted_addr;
	}
	bool cache_read (int phy_address) {
		int offset = phy_address & ((1 << offset_bits) - 1);
		int set_index = (phy_address >> offset_bits) & ((1 << set_bits) - 1);
		int tag = phy_address >> (offset_bits + set_bits);
		deque<cacheline> *row = &cache[set_index];
		bool found = false;
		for (auto it = row->begin(); it != row->end(); it++) {
			if (it->tag == tag) {
				found = 1;
				cacheline hit = *it;
				row->erase(it);
				row->push_front(hit);
				hits++;
				return 1;
			}
		}
		miss++;
		return 0;
	}
	void cache_flush(int phy_address) {
		int set_index = (phy_address >> offset_bits) & ((1 << set_bits) - 1);
		int tag = phy_address >> (offset_bits + set_bits);
		deque<cacheline> *row = &cache[set_index];
		for (auto it = row->begin(); it != row->end(); it++) {
			if (it->tag == tag) {
				row->erase(it);
				return;
			}
		}
	}
	void cache_stats () {
		cout << "Cache hits in level " << level_id << ": " << hits << endl;
		cout << "Cache misses in level " << level_id << ": " << miss << endl;
		float hit_ratio = 0.0;
		if ((hits + miss) != 0) hit_ratio = 100 * float(hits) / (hits + miss);
		cout << "Cache hit percentage is " << hit_ratio << endl;
	}
	void clear_cache () {
		for (auto &set_i : cache) {
			set_i.clear();
		}
		hits = 0;
		miss = 0;
		cout << "Cache Level " << level_id << " has been reset." << endl;
	}
};
