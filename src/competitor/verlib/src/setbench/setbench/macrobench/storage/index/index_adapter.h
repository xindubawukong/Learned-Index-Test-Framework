/*
 * File:   index_with_rq.h
 * Author: trbot
 *
 * Created on May 28, 2017, 3:03 PM
 */

#ifndef INDEX_WITH_RQ_H
#define INDEX_WITH_RQ_H

#include <limits>
#include <csignal>
#include <cstring>
#include "row.h"
#include "row_lock.h"
#include "index_base.h"     // for table_t declaration, and parent class inheritance

#include <ctime>
#include "plaf.h"
#include "random_xoshiro256p.h"
#include "adapter.h"

//static PAD;
static Random64 rngs[MAX_THREADS_POW2]; // create per-thread random number generators (padded to avoid false sharing)

/**
 * Define index data structure and record manager types
 */

//    typedef Node<FAT_NODE_DEGREE, KEY_TYPE> NODE_TYPE;
//    typedef SCXRecord<FAT_NODE_DEGREE, KEY_TYPE> DESCRIPTOR_TYPE;
//    typedef record_manager<RECLAIMER_TYPE, ALLOCATOR_TYPE, POOL_TYPE, NODE_TYPE> RECORD_MANAGER_TYPE;
//    typedef bslack<FAT_NODE_DEGREE, KEY_TYPE, less<KEY_TYPE>, RECORD_MANAGER_TYPE> INDEX_ADAPTER_T;
//    #define INDEX_CONSTRUCTOR_ARGS g_thread_cnt, FAT_NODE_DEGREE, __NO_KEY, SIGQUIT
//    #define CALL_CALCULATE_INDEX_STATS_FOREACH_CHILD(x, depth) { \
//        for (int __i=0;__i<(x)->getABDegree();++__i) { \
//            calculate_index_stats((NODE_TYPE *) (x)->ptrs[__i], (depth)); \
//        } \
//    }
//    #define ISLEAF(x) (x)->isLeaf()
//    #define VALUES_ARRAY_TYPE void **

/**
 * Create an adapter class for the DBx1000 index interface
 */

class Index : public index_base {
private:
    ds_adapter<KEY_TYPE, VALUE_TYPE> * index;

//    unsigned long alignment[9] = {0};
//    unsigned long sum_nodes_depths = 0;
//    unsigned long sum_leaf_depths = 0;
//    unsigned long num_leafs = 0;
//    unsigned long num_nodes = 0;
//    unsigned long num_keys = 0;
//    int max_depth = 0;
    PAD;

//    void calculate_index_stats(NODE_TYPE * curr, int depth) {
//        if (curr == NULL) return;
//        unsigned node_alignment = (unsigned long) curr & 63UL;
//        assert((node_alignment % 8) == 0);
//        alignment[node_alignment/8]++;
//        sum_nodes_depths += depth;
//        ++num_nodes;
//        if (depth > max_depth) {
//            max_depth = depth;
//        }
//        if (ISLEAF(curr)) {
//            ++num_leafs;
//            ++num_keys; // TODO: fix number of keys for the (a,b)-tree!!!
//            sum_leaf_depths += depth;
//        } else {
//            CALL_CALCULATE_INDEX_STATS_FOREACH_CHILD(curr, 1+depth);
//        }
//    }

private:
    static void callback_free_rows_and_items(KEY_TYPE key, VALUE_TYPE item, string this_idx_name) {
        while (item) {
            // first try to free row inside the item
            // (but skip this for tpcc-indexes i_orderline_wd and i_customer_last,
            //  since they are duplicate indexes that simply share rows with another index...)
            if (this_idx_name.compare("CUSTOMER_LAST_IDX") != 0 &&
                this_idx_name.compare("ORDERLINE_WD_IDX") != 0) {

                item->setbench_deinit();
            }

            // then try to free the item
            VALUE_TYPE nextItem = item->next;
            free(item);
            item = nextItem;
        }
    }

public:

    ~Index() {
        std::cout<<"  performing iterate()"<<std::endl;
#ifdef DS_ADAPTER_SUPPORTS_TERMINAL_ITERATE
        index->iterate(callback_free_rows_and_items, this->index_name);
#else
        std::cout<<"  WARNING: this index type does not implement an iterate() function to be used in cleaning up garbage memory generated by macrobench..."<<std::endl;
#endif
        std::cout<<"  performing delete"<<std::endl;
        delete index;
    }

    // WARNING: DO NOT OVERLOAD init() WITH NO ARGUMENTS!!!
    RC init(uint64_t part_cnt, table_t * table) {
        if (part_cnt != 1) setbench_error("part_cnt != 1 unsupported");

        srand(time(NULL));
        for (int i=0;i<MAX_THREADS_POW2;++i) {
            rngs[i].setSeed(rand());
        }

        KEY_TYPE minKey = __NO_KEY;
        KEY_TYPE maxKey = std::numeric_limits<KEY_TYPE>::max();
        VALUE_TYPE reservedValue = __NO_VALUE;

        if (g_thread_cnt > MAX_THREADS_POW2) {
            setbench_error("g_thread_cnt > MAX_THREADS_POW2");
        }
        index = new ds_adapter<KEY_TYPE, VALUE_TYPE>(MAX_THREADS_POW2, minKey, maxKey, reservedValue, rngs);
        this->table = table;

        return RCOK;
    }

    RC index_insert(KEY_TYPE key, VALUE_TYPE newItem, int part_id = -1) {
#if defined USE_RANGE_QUERIES
        auto oldVal = index->insertIfAbsent(tid, key, newItem);
//#ifndef NDEBUG
//        if (oldVal != index->NO_VALUE) {
//            cout<<"index_insert found element already existed."<<std::endl;
//            cout<<"index name="<<index_name<<std::endl;
//            cout<<"key="<<key<<std::endl;
//        }
//        assert(oldVal == index->NO_VALUE);
//#endif
        // TODO: determine if there are index collisions in anything but orderline
        //       (and determine why they happen in orderline, and if it's ok)
#else
        lock_key(key);
            auto oldItem = index->insertIfAbsent(tid, key, newItem);
            if (oldItem != index->getNoValue()) {
                // adding to existing list
                assert(newItem->next == NULL);
                newItem->next = oldItem->next;
                oldItem->next = newItem;
            }
        unlock_key(key);
#endif
        INCREMENT_NUM_INSERTS(tid);
        return RCOK;
    }
    RC index_read(KEY_TYPE key, VALUE_TYPE * item, int part_id = -1, int thd_id = 0) {

        #define IRT_VALIDATE 1
        #define IRT_LOCK 2
        #define IRT_UNSAFE 3

        #define INDEX_READ_TECHNIQUE IRT_VALIDATE

#if INDEX_READ_TECHNIQUE == IRT_VALIDATE
        while (1) {
            vwlock s1 = read_lock_state(key);
            if (state_is_locked(s1)) continue; // spin until key is unlocked

            *item = (VALUE_TYPE) index->find(tid, key);

            vwlock s2 = read_lock_state(key);
            if (s2 == s1) break; // validate key is still unlocked (with same version)

            // this validation avoids race conditions where a value/item
            // is temporarily cut out of the linked list of items,
            // despite being semantically present in the set.
            // (an alternative to validation is locking -- see below)
        }
#elif INDEX_READ_TECHNIQUE == IRT_LOCK
            lock_key(key);
            *item = (VALUE_TYPE) index->find(tid, key);
            unlock_key(key);
#elif INDEX_READ_TECHNIQUE == IRT_UNSAFE
            *item = (VALUE_TYPE) index->find(tid, key);
#endif
        INCREMENT_NUM_READS(tid);
        return RCOK;
    }
    // finds all keys in the set in [low, high],
    // saves the number N of keys in numResults,
    // saves the keys themselves in resultKeys[0...N-1],
    // and saves their values in resultValues[0...N-1].
    RC index_range_query(KEY_TYPE low, KEY_TYPE high, KEY_TYPE * resultKeys, VALUE_TYPE * resultValues, int * numResults, int part_id = -1) {
        *numResults = index->rangeQuery(tid, low, high, resultKeys, (VALUE_TYPE *) resultValues);
        INCREMENT_NUM_RQS(tid);
        return RCOK;
    }
    void initThread(const int tid) {
        index->initThread(tid);
    }
    void deinitThread(const int tid) {
        index->deinitThread(tid);
    }

    size_t getNodeSize() {
//        return sizeof(NODE_TYPE);
        return 0;
    }

    size_t getDescriptorSize() {
//        return sizeof(DESCRIPTOR_TYPE);
        return 0;
    }

    void print_stats(){
        index->printObjectSizes();
        index->printSummary();
    }
//        calculate_index_stats(index->debug_getEntryPoint(),0);
//        cout << "Nodes: "<< num_nodes << endl;
//        cout << "Leafs: "<< num_leafs << endl;
//        cout << "Keys: "<< num_keys << endl;
//        cout << "Node size: " << sizeof(NODE_TYPE) << endl;
//        cout << "Descriptor size: " << sizeof(DESCRIPTOR_TYPE) << endl;
//        cout << "Max path length: " << max_depth << endl;
//        cout << "Avg depth: " << (num_nodes?sum_nodes_depths/num_nodes:0) << endl;
//        cout << "Avg leaf depth: " << (num_leafs?sum_leaf_depths/num_leafs:0) << endl;
//        for (int i=0; i<8 ;i++){
//            cout << "Alignment " << i*8 << ": " << (alignment[i]/(double)num_nodes)*100 << "%" << endl;
//        }
};

#endif /* INDEX_WITH_RQ_H */
