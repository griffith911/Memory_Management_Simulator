#include <vector>
#include <deque>
#include <iostream>
#include "../cache/cache.hpp" 
using namespace std;

struct PageTableEntry {
	bool in_ram = 0;
	int frame_number = -1;
	int last_accessed = -1;
};
struct Frame {
	bool occupied = 0;
	int page_id = -1;
	int data;
};
struct virtual_memory {
	int PageSize;
	int ram_size;
	int total_memory;
	int pages, frames;
	vector<PageTableEntry> PageTable;
	vector<Frame> RAM;
	deque<int> empty_frames, LRU;
	int page_hits = 0;
	int page_faults = 0;
	int lvl1_to_2 = 0;
	int cache_to_ram = 0;
	cache_lvl* level1 = NULL;
	cache_lvl* level2 = NULL;
	virtual_memory (int TOT_MEM, int RAM_SIZE, int PAGE_SIZE) {
		total_memory = TOT_MEM;
		ram_size = RAM_SIZE;
		PageSize = PAGE_SIZE;
		frames = ram_size / PageSize;
		pages = total_memory / PageSize;
		PageTable.resize(pages);
		RAM.resize(frames);
		for (int i = 0; i < frames; i++)
			empty_frames.push_front(i);
	}
	bool ram_read (int address) {
		// vpn = virtual_page_number
		int vpn = address / PageSize;
		int offset = address % PageSize;
		PageTableEntry* page = &PageTable[vpn];
		if (page->in_ram) {
			page_hits++;
			int frame_no = page->frame_number;
			for (auto it = LRU.begin(); it != LRU.end(); it++) {
				if (*it == frame_no) {
					LRU.erase(it);
					LRU.push_front(frame_no);
					return 1;
				}
			}
		}
		else {
			page_faults++;
			return 0;
		}
		return 0;
	}
	void ram_insert (int address) {
		int vpn = address / PageSize;
		int offset = address % PageSize;
		PageTableEntry* page = &PageTable[vpn];
		if (page->in_ram) {
			ram_read(address);
			page_hits--;
		}
		else {
			if (empty_frames.size() == 0) {
				// evict the victim page
				int evicted = LRU.back();
				LRU.pop_back();
				empty_frames.push_front(evicted);
				Frame* frame = &RAM[evicted];
				PageTableEntry* old_page = &PageTable[frame->page_id];
				old_page->in_ram = false;
				old_page->frame_number = -1;
				frame->occupied = false;
				frame->page_id = -1;
				cout << "Evicted from Frame No. " << evicted << endl;
				int frame_start = evicted * PageSize;
				// flush level 1
				if (level1 != NULL) {
					for (int addr = frame_start; addr < frame_start + PageSize; addr += level1->block_size) {
						level1->cache_flush(addr);
					}
				}
				// flush level 2
				if (level2 != NULL) {
					for (int addr = frame_start; addr < frame_start + PageSize; addr += level2->block_size) {
						level2->cache_flush(addr);
					}
				}
			}
			// load the required page
			int free_frame = empty_frames.back();
			empty_frames.pop_back();
			LRU.push_front(free_frame);
			cout << "Inserted in Frame No. " << free_frame << endl;
			Frame* frame = &RAM[free_frame];
			frame->occupied = true;
			frame->page_id = vpn;
			page->in_ram = true;
			page->frame_number = free_frame;
		}
	}
	// cache integration
	void total_read (int address) {
		bool fault = false;
		if (!ram_read(address)) {
			cout << "Page fault, loading from disk address = " << address << endl;
			ram_insert(address);
			fault = true;
		}
		int vpn = address / PageSize;
		int offset = address % PageSize;
		PageTableEntry* page = &PageTable[vpn];
		int phy_frame_no = page->frame_number;
		int phy_address = (phy_frame_no * PageSize) + offset;

		if (!fault && level1 != NULL) {
			if (level1->cache_read(phy_address)) {
				cout << "Found in Cache level 1 " << address << endl;
				return;
			}
			if (level2 != NULL) {
				if (level2->cache_read(phy_address)) {
					lvl1_to_2++;
					cout << "Found in Cache level 2 " << address << endl;
					level1->cache_insert(phy_address);
					return;
				}
			}
		}
		if (!fault) {
			cache_to_ram++;
			cout << "Not in Cache. Found in RAM." << endl;
		}
		if (level2 != NULL) level2->cache_insert(phy_address);
		if (level1 != NULL) level1->cache_insert(phy_address);
	}
	float hit_ratio (int hit_no, int miss_no) {
		if ((hit_no + miss_no) == 0) return 0.0;
		return 100 * float(hit_no) / (hit_no + miss_no);
	}
	void ram_stats () {
		if (level1) {
			cout << "Cache hits in level 1: " << level1->hits << endl;
			cout << "Cache misses in level 1: " << level1->miss << endl;
			cout << "Cache hit ratio on level 1: " << hit_ratio(level1->hits,level1->miss) << "%" << endl;
		}
		if (level2) {
			cout << "Cache hits in level 2: " << level2->hits << endl;
			cout << "Cache misses in level 2: " << level2->miss << endl;
			cout << "Cache hit ratio on level 2: " << hit_ratio(level2->hits,level2->miss) << "%" << endl;
		}
		if (level1 && level2) {
			cout << "Total Cache hit ratio is ";
			cout << hit_ratio(level1->hits+level2->hits, level1->miss+level2->miss) << "%" << endl;
			cout << "Miss propagation from level 1 to level 2: " << lvl1_to_2 << endl;
		}
		cout << "Cache to RAM miss propagation: " << cache_to_ram << endl;
		cout << "Page hit ratio is " << hit_ratio(page_hits,page_faults) << "%" << endl;
	}
	void used_frames () {
		cout << "Currently used Frames in RAM: ";
		for (auto it = LRU.begin(); it != LRU.end(); it++) {
			cout << *it << " ";
		}
		cout << endl;
	}
	void used_pages () {
		cout << "Currently used PAGE_ID's in virtual memory: ";
		for (int i = 0; i < pages; i++) {
			if (PageTable[i].in_ram) {
				cout << i << " ";
			}
		}
		cout << endl;
	}
	void lvl1 () {
		if (level1 == NULL) return;
		size_t res = 0;
		for (int i = 0; i < level1->sets; i++) {
			res += level1->cache[i].size();
		}
		cout << "Cache level 1 is filled upto ";
		cout << res;
		cout << " out of " << level1->cache_size / level1->block_size << endl;
	}
	void lvl2 () {
		if (level2 == NULL) return;
		size_t res = 0;
		for (int i = 0; i < level2->sets; i++) {
			res += level2->cache[i].size();
		}
		cout << "Cache level 2 is filled upto ";
		cout << res;
		cout << " out of " << level2->cache_size / level2->block_size << endl;
	}
};
