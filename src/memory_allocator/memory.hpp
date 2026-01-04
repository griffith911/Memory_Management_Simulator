#include <iostream>
#include <string>

using namespace std;

struct Block {
	int size = 0;
	int block_id = 0;
	int address = 0;
	Block* next = NULL;
	Block* prev = NULL;
	bool is_free = true;
};
struct Memory {
	Block* head;
	int TOTAL_MEMORY;
	int id_counter = 0;
	int allocator = 1;
	Memory(int tm) {
		TOTAL_MEMORY = tm;
		head = new Block();
		head->size = TOTAL_MEMORY;
	}
	Block* malloc_first (int rsz) {
		Block* temp = head;
		Block* first_fit = NULL;
		while (temp) {
			if (temp->is_free && temp->size >= rsz) {
				first_fit = temp;
				break;
			}
			temp = temp->next;
		}
		if (first_fit) {
			id_counter++;
			cout << "Allocated block id=" << id_counter << " at address=" << first_fit->address << endl;
			int extra = first_fit->size - rsz;
			first_fit->block_id = id_counter;
			first_fit->is_free = false;
			if (extra) {
				Block* new_hole = new Block();
				new_hole->next = first_fit->next;
				first_fit->next = new_hole;
				first_fit->size = rsz;
				new_hole->size = extra;
				new_hole->prev = first_fit;
				new_hole->address = first_fit->address + rsz;
				if (new_hole->next)
					new_hole->next->prev = new_hole;
			}
		}
		else {
			cout << "Error: Memory Full, free some memory" << endl;
		}
		return head;
	}
	Block* malloc_best (int rsz) {
		Block* temp = head;
		int best_size = 1000000000;
		Block* best_fit = NULL;
		while (temp != NULL) {
			if (temp->is_free && temp->size >= rsz) {
				if (temp->size < best_size) {
					best_size = temp->size;
					best_fit = temp;
				}
			}
			temp = temp->next;
		}
		if (best_fit) {
			id_counter++;
			cout << "Allocated block id=" << id_counter << " at address=" << best_fit->address << endl;
			if (best_size == rsz) {
				best_fit->is_free = false;
				best_fit->block_id = id_counter;
				return head;
			}
			Block* new_hole = new Block();
			new_hole->address = best_fit->address + rsz;
			new_hole->size = best_fit->size - rsz;
			best_fit->is_free = false;
			best_fit->size = rsz;
			best_fit->block_id = id_counter;
			Block* agla = best_fit->next;
			new_hole->next = agla;
			new_hole->prev = best_fit;
			best_fit->next = new_hole;
			if (agla != NULL) {
				agla->prev = new_hole;
			}
		}
		else {
			cout << "Error: Memory Full, free some memory" << endl;
		}
		return head;
	}
	Block* malloc_worst (int rsz) {
		Block* temp = head;
		Block* worst_fit = NULL;
		int worst_size = -1;
		while (temp) {
			if (temp->is_free && temp->size >= rsz) {
				if (temp->size >= worst_size) {
					worst_size = temp->size;
					worst_fit = temp;
				}
			}
			temp = temp->next;
		}
		if (worst_fit) {
			id_counter++;
			cout << "Allocated block id=" << id_counter << " at address=" << worst_fit->address << endl;
			int extra = worst_size - rsz;
			worst_fit->is_free = false;
			worst_fit->block_id = id_counter;
			if (extra) {
				Block* new_hole = new Block();
				new_hole->next = worst_fit->next;
				new_hole->address = worst_fit->address + rsz;
				new_hole->size = extra;
				new_hole->prev = worst_fit;
				worst_fit->next = new_hole;
				worst_fit->size = rsz;
				if (new_hole->next)
					new_hole->next->prev = new_hole;
			}
		}
		else {
			cout << "Error: Memory full, free some memory" << endl;
		}
		return head;
	}
	void free_block (int b_id) {
		Block* temp = head;
		while (temp) {
			if (temp->block_id == b_id) {
				temp->block_id = 0;
				temp->is_free = true;
				cout << "Block " << b_id << " freed from address ";
				cout << temp->address << " to " << temp->address + temp->size - 1 << endl;
				// coalesce
				if (temp->prev && temp->prev->is_free) {
					Block* cur = temp;
					temp = temp->prev;
					temp->size += cur->size;
					temp->next = cur->next;
					if (cur->next) {
						cur->next->prev = temp;
					}
					delete cur;
				}
				if (temp->next && temp->next->is_free) {
					Block* nxfr = temp->next;
					temp->size += nxfr->size;
					temp->next = nxfr->next;
					if (nxfr->next) {
						nxfr->next->prev = temp;
					}
					delete nxfr;
				}
				return;
			}
			temp = temp->next;
		}
		cout << "No Block found with id=" << b_id << endl;
	}
	void memory_dump () {
		Block* temp = head;
		while (temp) {
			cout << "[" << temp->address << " - " << temp->address + temp->size - 1 << "]";
			if (temp->is_free) {
				cout << " FREE" << endl;
			}
			else {
				cout << " USED (id=" << temp->block_id << ")" << endl;
			}
			temp = temp->next;
		}
	}
	void clear_memory () {
		Block* temp = head;
		while (temp) {
			Block* next = temp->next;
			delete temp;
			temp = next;
		}
		head = new Block();
		head->size = TOTAL_MEMORY;
		id_counter = 0;
		cout << "All Block have been freed." << endl;
	}
	void memory_stats () {
		Block* temp = head;
		int largest_free = 0;
		int total_free = 0;
		int total_memory = 0;
		while (temp) {
			total_memory += temp->size;
			if (temp->is_free) {
				total_free += temp->size;
				if (temp->size > largest_free)
					largest_free = temp->size;
			}
			temp = temp->next;
		}
		float ext_frag = (1 - float(largest_free) / total_free) * 100;
		cout << "Total Memory: " << total_memory << '\n';
		cout << "Total Free Memory: " << total_free << '\n';
		cout << "Allocated Memory: " << total_memory - total_free << '\n';
		cout << "External Fragmentation: " << ext_frag << '\n';
	}
};
