#include "vm_pager.h"

#include <stdio.h>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <map>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "vm_arena.h"

const unsigned int NUM_VIRTUAL_PAGES = VM_ARENA_SIZE / VM_PAGESIZE;

struct page_t {
    std::unordered_set<page_table_entry_t *> mapped_ptes;
    std::string *os_filename;      // only valid if virtual_filename != nullptr
    const char *virtual_filename;  // char* is virtual addr
    unsigned int ppage;
    unsigned int block;  // Swap or Regular Block
    bool resident;
    bool dirty;
    bool referenced;

    bool operator==(const page_t &other) const {
        // Compare the members of this struct with the corresponding members of the
        // other struct
        return (this->mapped_ptes == other.mapped_ptes && this->os_filename == other.os_filename &&
                this->virtual_filename == other.virtual_filename && this->ppage == other.ppage &&
                this->block == other.block && this->resident == other.resident && this->dirty == other.dirty &&
                this->referenced == other.referenced);
        // Return true if they are equal, false otherwise
    }
};

typedef struct {
    std::vector<page_t *> virtual_pages;               // OS Page meta data (referenced, dirty, valid, etc.)
    page_table_entry_t page_table[NUM_VIRTUAL_PAGES];  // Page table with PTEs for this process
    unsigned int size;                                 // SIZE of the Page table, number of PTEs
} process_t;

static unsigned int max_arena_pages;
static unsigned int max_physical_pages;
static unsigned int max_swap_pages;

static pid_t current_pid_num;

static std::unordered_map<pid_t, process_t> processes;
// Way to locate the virtual page
static std::unordered_map<page_table_entry_t *, page_t *> virtual_page_map;
// Map from <filename, block> to a virtual page, only for
// file-backed pages
static std::map<std::pair<std::string, unsigned int>, page_t *> mapped_files;

// static std::deque<page_t> all_virtual_pages; // All Virtual Pages
static std::vector<page_t *> physical_pages;  // for clock algorithm

static std::queue<unsigned int> free_swap_blocks;
static std::queue<unsigned int> free_physical_blocks;
static std::deque<page_t *> clock_queue;

pid_t getpid();
unsigned int addr_to_index(const void *addr);
int evict_physical_page();
void create_empty_arena(pid_t current_pid);

std::string *create_os_filename(page_t *page, pid_t pid);
int make_page_resident(page_t *page);
int write_to_pinned_page(page_t *page);

void set_all_pte_rw(page_t *page, bool read_enable, bool write_enable);
void set_all_pte_ppage(page_t *page, unsigned int ppage);

void print_physical_pages();
void print_virtual_page(page_t *page);

bool check_inside_arena(pid_t pid, const void *addr);

/*
 * vm_init
 *
 * Called when the pager starts.  It should set up any internal data structures
 * needed by the pager.
 *
 * vm_init is passed the number of physical memory pages and the number
 * of blocks in the swap file.
 */
void vm_init(unsigned int memory_pages, unsigned int swap_blocks) {
    assert(processes.empty());
    assert(physical_pages.empty());
    assert(free_swap_blocks.empty());
    assert(free_physical_blocks.empty());
    assert(virtual_page_map.empty());
    assert(mapped_files.empty());

    max_physical_pages = memory_pages;
    max_swap_pages = swap_blocks;
    max_arena_pages = (unsigned int)VM_ARENA_SIZE / VM_PAGESIZE;

    memset(vm_physmem, 0, VM_PAGESIZE);

    physical_pages.resize(max_physical_pages, nullptr);

    for (size_t i = 0; i < max_swap_pages; ++i) {
        free_swap_blocks.push(i);
    }
    for (size_t i = 1; i < max_physical_pages; ++i) {
        free_physical_blocks.push(i);
    }
}

/*
 * vm_create
 * Called when a parent process (parent_pid) creates a new process (child_pid).
 * vm_create should cause the child's arena to have the same mappings and data
 * as the parent's arena.  If the parent process is not being managed by the
 * pager, vm_create should consider the arena to be empty.
 * Note that the new process is not run until it is switched to via vm_switch.
 * Returns 0 on success, -1 on failure.
 */
int vm_create(pid_t parent_pid, pid_t child_pid) {
    // Core: Assume Empty Arena
    assert(processes[parent_pid].size == 0);  // added this
    create_empty_arena(child_pid);
    return 0;
}

/*
 * vm_switch
 *
 * Called when the kernel is switching to a new process, with process
 * identifier "pid".
 */
void vm_switch(pid_t pid) {
    current_pid_num = pid;

    if (processes.find(pid) == processes.end()) {  // Process is New
        create_empty_arena(pid);
    }  // Now Process's Arena should exist

    page_table_base_register = processes[pid].page_table;
}

/*
 * vm_fault
 *
 * Called when current process has a fault at virtual address addr.  write_flag
 * is true if the access that caused the fault is a write.
 * Returns 0 on success, -1 on failure.
 */
int vm_fault(const void *addr, bool write_flag) {
    pid_t current_pid = getpid();

    // First check whether the address is valid
    if (!check_inside_arena(current_pid, addr)) {
        return -1;
    }

    // Address is valid, cur_page is the virtual page
    page_t *cur_page = processes[current_pid].virtual_pages[addr_to_index(addr)];
    assert(cur_page->mapped_ptes.empty() == false);

    if (write_flag == false) {  // Read Fault
                                // Possible Read Fault Scenarios:
                                // Page is swap-backed:
        // File is new (Page 0), resident, but never accessed, PTE = {0, 0, 0} ->
        // Set to referenced, PTE = {0, 1, 0} File is not new (not Page 0) and not
        // resident, PTE = {x, 0, 0} -> Read page from swap, then Set to resident
        // and referenced, PTE = {x, 1, 0} File is resident, but was marked
        // unreferenced by clock, PTE = {x, 0, 0} -> Set to referenced, PTE = {x, 1,
        // dirty?}
        // Page is file-backed:
        // File is not resident, PTE = {x, 0, 0} -> Read page from file, then Set to
        // resident and referenced, PTE = {x, 1, 0} File is resident, but was marked
        // unreferenced by clock, PTE = {x, 0, 0} -> Set to referenced, PTE = {x, 1,
        // dirty?}

        if (cur_page->resident == false) {  // Not resident
            if (make_page_resident(cur_page) == -1) {
                return -1;
            }
            // assert(cur_page->ppage != -1);
        }

        assert(cur_page->resident == true);
        // super smart and important, if dirty then we have to remember that in os
        // code and put
        set_all_pte_rw(cur_page, true, cur_page->dirty);

    } else {  // Write Fault
              // Possible Write Fault Scenarios:
              // Page is swap-backed:
        // File is new (Page 0), resident, but never accessed, PTE = {0, 0, 0} ->
        // Copy page, then set to referenced and dirty, PTE = {x, 1, 1} File is new
        // (Page 0), resident, but only read (referenced), PTE = {0, 1, 0} -> Copy
        // page, then Set to dirty, PTE = {x, 1, 1} File is not new (not Page 0) and
        // not resident, PTE = {x, 0, 0} -> Read page from swap, then Set to
        // resident, referenced, and dirty PTE = {x, 1, 1} File is not new (not Page
        // 0), resident, but only read (referenced), PTE = {x, 1, 0} -> Set to
        // dirty, PTE = {x, 1, 1} File is resident, but was marked unreferenced by
        // clock, PTE = {x, 0, 0} -> Set to referenced, PTE = {x, 1, 1}
        // Page is file-backed:
        // File is not resident, PTE = {x, 0, 0} -> Read page from file, then Set to
        // resident, referenced, and dirty, PTE = {x, 1, 1} File is resident, but
        // only read (referenced), PTE = {x, 1, 0} -> Set to dirty, PTE = {x, 1, 1}
        // File is resident, but was marked unreferenced by clock, PTE = {x, 0, 0}
        // -> Set to referenced, PTE = {x, 1, 1}
        if (cur_page->virtual_filename == nullptr) {         // Swap-backed
            if (cur_page->ppage == 0) {                      // Case 1 and 2
                if (write_to_pinned_page(cur_page) == -1) {  // Copies the page
                    return -1;
                }
            } else {
                if (cur_page->resident == false) {  // Case 3
                    if (make_page_resident(cur_page) == -1) {
                        return -1;
                    }
                }
            }
        } else {  // File-backed
            if (cur_page->resident == false) {
                if (make_page_resident(cur_page) == -1) {
                    return -1;
                }
            }
        }

        assert(cur_page->resident == true);
        assert(cur_page->ppage != 0);
        cur_page->dirty = true;
        set_all_pte_rw(cur_page, true, true);
    }

    cur_page->referenced = true;
    // cur_page is the current virtual page
    // cur_page is now in physical memory, add that to physical_memory index
    // location so that the subset of memory locations that are physical contains
    // cur_page (virtual page)
    physical_pages[cur_page->ppage] = cur_page;

    return 0;
}

/*
 * vm_destroy
 *
 * Called when current process exits.  This gives the pager a chance to
 * clean up any resources used by the process.
 */
void vm_destroy() {
    pid_t current_pid = getpid();
    for (size_t i = 0; i < processes[current_pid].size; ++i) {
        page_t *current_page = processes[current_pid].virtual_pages[i];
        page_table_entry_t *current_pte = &processes[current_pid].page_table[i];
        // Remove the map between pte and virtual page
        virtual_page_map.erase(current_pte);
        unsigned int ppage = 0; // MAJOR MAJOR MAJOR Fix: cannot push page 0 to free block

        if (current_page->virtual_filename == nullptr) {  // Swap-backed page, just erase all data
            if (current_page->resident == true) {
                ppage = current_page->ppage;
                
                if (ppage != 0) {  // Erase from clock queue as well
                    physical_pages[ppage] = nullptr;  // Erase Physical Page Meta Data
                    free_physical_blocks.push(ppage); // MAJOR Fix: Cannot do this for page 0 !!!
                    clock_queue.erase(std::find(clock_queue.begin(), clock_queue.end(), current_page));
                }
            }
            free_swap_blocks.push(current_page->block);

            delete current_page;  // Swap pages must be unique
        } else {                  // Very annoying File-backed page
            current_page->mapped_ptes.erase(current_page->mapped_ptes.find(current_pte));
        }
    }
    // Finally, Clean up Process
    processes.erase(current_pid);
}

/*
 * vm_map
 *
 * A request by the current process for the lowest invalid virtual page in
 * the process's arena to be declared valid.  On success, vm_map returns
 * the lowest address of the new virtual page.  vm_map returns nullptr if
 * the arena is full.
 *
 * If filename is nullptr, block is ignored, and the new virtual page is
 * backed by the swap file, is initialized to all zeroes (from the
 * application's perspective), and private (i.e., not shared with any other
 * virtual page).  In this case, vm_map returns nullptr if the swap file is
 * out of space.
 *
 * If filename is not nullptr, the new virtual page is backed by the specified
 * file at the specified block and is shared with other virtual pages that are
 * mapped to that file and block.  filename is a null-terminated C string and
 * must reside completely in the valid portion of the arena.  In this case,
 * vm_map returns nullptr if filename is not completely in the valid part of
 * the arena.
 * filename is specified relative to the pager's current working directory.
 */
void *vm_map(const char *filename, unsigned int block) {
    pid_t current_pid = getpid();

    assert(processes.find(current_pid) != processes.end());

    if (processes[current_pid].virtual_pages.size() >= max_arena_pages) {  // Arena is Full
        return nullptr;
    }  // Now Arena Still has spaces to allocate -> Check Physical Memory

    // eager reservation, no free swap blocks
    if (filename == nullptr && free_swap_blocks.empty()) {
        return nullptr;
    }

    page_t *p = new page_t();  // New Virtual Page
    assert(p != nullptr);

    p->resident = false; // MAJOR MAJOR FIX: this is not true for swap page
    p->dirty = false;
    p->referenced = false;

    page_table_entry_t pte;

    if (filename == nullptr) {  // Swap-backed Page, Ignore BLOCK
        p->block = free_swap_blocks.front();
        free_swap_blocks.pop();
        p->virtual_filename = nullptr;
        p->os_filename = nullptr;
        unsigned int ppage = 0;  // assign physical page 0 for now
        p->ppage = ppage;
        // MAJOR MAJOR MAJOR FIX, why tf did we forget this before ?
        p->resident = true;
        pte = {ppage, true, false};

        processes[current_pid].page_table[processes[current_pid].size] = std::move(pte);
        page_table_entry_t *new_pte = &processes[current_pid].page_table[processes[current_pid].size++];

        // Add an alias for
        // this new pte ptr,
        // increment size

        // Check PTE is not mapped to any virtual page yet
        assert(virtual_page_map.find(new_pte) == virtual_page_map.end());
        assert(p->mapped_ptes.empty());  // Sanity Check

        // important for file packed
        p->mapped_ptes.insert(new_pte);  // Second, add the new &pte to the virtual
                                         // page (which is new here)

        virtual_page_map[new_pte] = p;  // Third, Map PTE to new virtual page (only 1-to-1 for swap pages)

        processes[current_pid].virtual_pages.push_back(p);  // Lastly, add the pointer to the set of process's own
                                                            // virtual pages as usual

    } else {  // File-backed Page, BLOCK NUMBER input matters
        p->block = block;
        p->virtual_filename = filename;

        // Now get real file name
        // Have to do this now to make sure the filename is valid
        p->os_filename = create_os_filename(p, current_pid);
        if (p->os_filename == nullptr) {
            return nullptr;
        }

        // MAJOR FIX: Check if page already mapped? (Spec 5.4.2)
        unsigned int ppage = 0;  // assign physical page 0 for now (assume not mapped then check)
        std::pair<std::string, unsigned int> file_key = {*p->os_filename, p->block};

        if (mapped_files.find(file_key) == mapped_files.end()) {  // New Page
            p->ppage = ppage;
            pte = {ppage, false, false};

            // First, add the new pte to the page table array of the process
            processes[current_pid].page_table[processes[current_pid].size] = std::move(pte);

            // Add an alias for
            // this new pte ptr,
            // increment size
            page_table_entry_t *new_pte = &processes[current_pid].page_table[processes[current_pid].size++];

            // Check PTE is not mapped to any virtual page yet
            assert(virtual_page_map.find(new_pte) == virtual_page_map.end());

            assert(p->mapped_ptes.empty());  // Sanity Check
            p->mapped_ptes.insert(new_pte);  // Second, add the new &pte to the virtual
                                             // page (which is new here)

            virtual_page_map[new_pte] = p;  // Third, Map PTE to new virtual page (the
                                            // page might be mapped to many pte's later)
            mapped_files[file_key] = p;     // Fourth, map key to the new virtual page [FILE-BACKED ONLY]

            processes[current_pid].virtual_pages.push_back(p);  // Lastly, add the pointer to the set of process's own
                                                                // virtual pages as usual

        } else {                    // Already mapped sometime before, just copy the mappings
            delete p->os_filename;  // Does not need the temporary page
            delete p;

            page_t *existing_page = mapped_files[file_key];
            // Get the existing PUBLIC PAGE
            assert(existing_page != nullptr);  // Sanity Check

            if (existing_page->mapped_ptes.empty()) {
                pte = {existing_page->ppage, existing_page->referenced,
                       existing_page->referenced && existing_page->dirty};
            } else {
                pte = **(existing_page->mapped_ptes.begin());
                assert((*existing_page->mapped_ptes.begin())->ppage == pte.ppage);
                assert((*existing_page->mapped_ptes.begin())->read_enable == pte.read_enable);
                assert((*existing_page->mapped_ptes.begin())->write_enable == pte.write_enable);
            }

            processes[current_pid].page_table[processes[current_pid].size] =
                std::move(pte);  // First, add the new pte to the page table array of the process
            page_table_entry_t *new_pte = &processes[current_pid].page_table[processes[current_pid].size++];
            // Add an alias for
            // this new pte ptr,
            // increment size

            existing_page->mapped_ptes.insert(new_pte);
            // Second, add the new &pte to the virtual page (which is
            // NOT new here)

            virtual_page_map[new_pte] = existing_page;  // Third, Map PTE to existing (the page might be mapped
                                                        // to more pte's later)

            processes[current_pid].virtual_pages.push_back(existing_page);  // Lastly, add the pointer to the set of
                                                                            // process's own virtual pages as usual
        }
    }

    assert(processes[current_pid].virtual_pages.back()->mapped_ptes.empty() == false);
    assert(processes[current_pid].virtual_pages.size() != 0);
    return (void *)((processes[current_pid].virtual_pages.size() - 1) * (unsigned long)VM_PAGESIZE +
                    (unsigned long)VM_ARENA_BASEADDR);
}

// Helper Functions

pid_t getpid() { return current_pid_num; }

unsigned int addr_to_index(const void *addr) {
    // return ((uintptr_t) addr - (uintptr_t) VM_ARENA_BASEADDR) >> 0x10;
    return ((uintptr_t)addr - (uintptr_t)VM_ARENA_BASEADDR) / VM_PAGESIZE;
}

// todo instead of this logic we can just have an unsigned integer of used_pages
bool physical_pages_full() {               // This is slow (linear), can add a variable to
                                           // keep track of it
    unsigned int sum = 1;                  // Page zero is pinned
    assert(physical_pages[0] == nullptr);  // Page zero is pinned
    for (size_t i = 1; i < max_physical_pages; ++i) {
        if (physical_pages[i] != nullptr) {
            ++sum;
        }
    }
    return sum >= max_physical_pages;
}

int evict_physical_page() {                                // MAJOR UPDATE
    assert(free_physical_blocks.empty());                  // Sanity Check
    assert(clock_queue.size() == max_physical_pages - 1);  // Sanity Check
    page_t *evicted_page = clock_queue.front();

    while (evicted_page->referenced == true) {  // Spec way of clock algo.
        assert(physical_pages[evicted_page->ppage] != nullptr);
        evicted_page->referenced = false;
        set_all_pte_rw(evicted_page, false, false);
        clock_queue.pop_front();
        clock_queue.push_back(evicted_page);
        evicted_page = clock_queue.front();
    }

    physical_pages[evicted_page->ppage] = nullptr;
    clock_queue.pop_front();  // Evict this page
    free_physical_blocks.push(evicted_page->ppage);

    if (evicted_page->dirty == true) {
        if (evicted_page->virtual_filename != nullptr) {
            assert(evicted_page->os_filename != nullptr);
        }

        const char *filename = evicted_page->virtual_filename ? evicted_page->os_filename->c_str() : nullptr;
        if (file_write(filename, evicted_page->block, (&((char *)vm_physmem)[evicted_page->ppage * VM_PAGESIZE])) ==
            -1) {
            return -1;
        }

        evicted_page->dirty = false;

        if (evicted_page->virtual_filename != nullptr &&
            evicted_page->mapped_ptes.empty() == true) {  // If the file-backed page is not mapped (no one is using
                                                          // it) anymore, REMOVE the Public Page
            std::pair<std::string, unsigned int> key = {*evicted_page->os_filename, evicted_page->block};
            mapped_files.erase(key);  // Mark the pair not mapped anymore

            // no memory leak!
            delete evicted_page->os_filename;
            delete evicted_page;  // All public pages must be unique
            return 0;
        }
    }

    evicted_page->resident = false;
    set_all_pte_rw(evicted_page, false, false);

    return 0;
}

void create_empty_arena(pid_t current_pid) {
    std::vector<page_t *> virtual_pages;  // OS page metadata
    virtual_pages.reserve(max_arena_pages);

    process_t current_process = {std::move(virtual_pages), {}, 0};

    // Process now contains an empty vector of page_t and an empty page table
    // array
    assert(processes.find(current_pid) == processes.end());
    processes[current_pid] = std::move(current_process);
}

std::string *create_os_filename(page_t *page, pid_t pid) {
    assert(page->virtual_filename != nullptr);  // Sanity Check
    pid_t page_pid = pid;

    if (check_inside_arena(page_pid, page->virtual_filename) == false) {
        return nullptr;  // Invalid Filename Starting Address
    }                    // At least the starting address is valid

    std::string real_name = "";
    char cur_char = '\0';
    char *cur_char_ptr = (char *)page->virtual_filename;

    do {
        if (check_inside_arena(page_pid, cur_char_ptr) == false) {
            return nullptr;
        }  // Page is still valid

        unsigned cur_page_idx = addr_to_index(cur_char_ptr);
        page_t *cur_page = processes[page_pid].virtual_pages[cur_page_idx];

        if (cur_page->resident == false || cur_page->referenced == false) { // MAJOR FIX: need to fault for both cases
            if (vm_fault(cur_char_ptr, false) == -1) {
                return nullptr;
            }  // Read Fault, now page should be resident
        }
        assert(cur_page->resident == true);
        assert(cur_page->referenced == true);

        uintptr_t cur_page_base_phys_addr = (uintptr_t) & ((char *)vm_physmem)[cur_page->ppage * VM_PAGESIZE];
        // both virtual
        uintptr_t virtual_page_base_addr = cur_page_idx * (uintptr_t)VM_PAGESIZE + (uintptr_t)VM_ARENA_BASEADDR;

        assert((uintptr_t)cur_char_ptr >= virtual_page_base_addr);
        char *cur_char_phys_addr =
            (char *)(cur_page_base_phys_addr + ((uintptr_t)cur_char_ptr - virtual_page_base_addr));
        cur_char = *cur_char_phys_addr;
        real_name.push_back(cur_char);
        ++cur_char_ptr;
    } while (cur_char != '\0');

    std::string *os_filename = new std::string();
    assert(os_filename != nullptr);
    *os_filename = real_name;

    // dynamic mem
    return os_filename;
}

int make_page_resident(page_t *page) {
    assert(page->referenced == false);
    assert(page->resident == false);
    assert(page->dirty == false);

    if (free_physical_blocks.empty()) {                        // No free physical memory, need to evict
        assert(clock_queue.size() == max_physical_pages - 1);  // Sanity Check
        if (evict_physical_page() == -1) {
            return -1;
        }
    }
    assert(!free_physical_blocks.empty());
    assert(free_physical_blocks.front() != 0);

    unsigned int ppage = free_physical_blocks.front();
    const char *filename = page->virtual_filename ? page->os_filename->c_str() : nullptr;

    if (file_read(filename, page->block, (&((char *)vm_physmem)[ppage * VM_PAGESIZE])) == -1) {
        return -1;
    }

    // physical pages is like the subset of the pages that are resident
    physical_pages[ppage] = page;
    free_physical_blocks.pop();
    clock_queue.push_back(page);

    page->ppage = ppage;
    page->resident = true;

    set_all_pte_rw(page, false, false);
    set_all_pte_ppage(page, ppage);

    return ppage;
}

int write_to_pinned_page(page_t *page) {
    assert(page->ppage == 0);  // Sanity Check

    if (free_physical_blocks.empty()) {                        // Find free block
        assert(clock_queue.size() == max_physical_pages - 1);  // Sanity Check
        if (evict_physical_page() == -1) {
            return -1;
        }
    }

    assert(!free_physical_blocks.empty());
    assert(free_physical_blocks.front() != 0);

    unsigned int ppage = free_physical_blocks.front();
    physical_pages[ppage] = page;
    free_physical_blocks.pop();

    page->resident = true;
    page->ppage = ppage;
    clock_queue.push_back(page);

    set_all_pte_ppage(page, ppage);
    set_all_pte_rw(page, false,
                   false);  // This is okay, because we will sort it out later when it returns

    memset(&((char *)vm_physmem)[ppage * VM_PAGESIZE], 0, VM_PAGESIZE);
    return 0;
}

void set_all_pte_rw(page_t *page, bool read_enable, bool write_enable) {
    for (page_table_entry_t *pte : page->mapped_ptes) {
        assert(pte != nullptr);
        pte->read_enable = read_enable;
        pte->write_enable = write_enable;
    }
}

void set_all_pte_ppage(page_t *page, unsigned int ppage) {
    for (page_table_entry_t *pte : page->mapped_ptes) {
        assert(pte != nullptr);
        pte->ppage = ppage;
    }
}

void print_virtual_page(page_t *page) {
    printf("#########[START VIRTUAL PAGE]#########\n");
    printf("Block Number:\t%u\n", page->block);
    printf("Dirty Bit:\t\t%d\n", page->dirty);
    printf("File Name Addr:\t%p\n", page->virtual_filename);
    printf("Residency:\t\t%d\n", page->resident);
    printf("Reference:\t\t%d\n", page->referenced);
    printf("PTEs Linked:\t%zu\n", page->mapped_ptes.size());
    printf("---------[P       T       E]---------\n");
    for (auto &ptr : page->mapped_ptes) {
        printf("Address:\t%p : %u, %u, %u\n", ptr, ptr->ppage, ptr->read_enable, ptr->write_enable);
    }
    printf("#########[END OF VIRTUAL PAG]#########\n");
}

void print_physical_pages() {
    printf("$$$$$$$$$$$$$$$$$$$$[START PHYSICAL PAGES]$$$$$$$$$$$$$$$$$$$$\n");
    for (size_t i = 0; i < max_physical_pages; ++i) {
        if (physical_pages[i] == nullptr) {
            if (i != 0) {
                printf("\n\n\n[E    M    P    T    Y]\n\n\n");
            } else {
                printf("\n\n\n[P   I   N   N   E   D]\n\n\n");
            }
        } else {
            print_virtual_page(physical_pages[i]);
        }

        if (i != max_physical_pages - 1) {
            printf(
                "--------------------[ ↑↑ Page %lu ↓↓ Page %lu "
                "]--------------------\n",
                i, i + 1);
        }
    }
    printf("$$$$$$$$$$$$$$$$$$$$[END OF PHYSICAL PAGE]$$$$$$$$$$$$$$$$$$$$\n");
}

bool check_inside_arena(pid_t pid, const void *addr) {
    return !((uintptr_t)addr < (uintptr_t)VM_ARENA_BASEADDR ||
             (uintptr_t)addr >= (uintptr_t)VM_ARENA_BASEADDR + (uintptr_t)VM_PAGESIZE * processes[pid].size);
}
