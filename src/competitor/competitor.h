#include"./indexInterface.h"
#include "./alex/alex.h"
// #include "./alexol/alex.h"
// #include "./artsync/artrowex.h"
// #include "./artsync/artolc.h"
// #include "./artsync/artunsync.h"
#include "./xindex/xindex.h"
#include "./btreeolc/btreeolc.h"
// #include "./hot/hot.h"
// #include "./hot/hotrowex.h"
#include "./lipp/lipp.h"
#include "./lippol/lippol.h"
// #include "pgm/pgm.h"
// #include "btree/btree.h"
// #include "wormhole/wormhole.h"
// #include "wormhole_u64/wormhole_u64.h"
// #include "masstree/masstree.h"
#include "finedex/finedex.h"
#include "iostream"

#include "../ours/empty_index.h"
#include "../ours/fh_index_ro_interface.h"
#include "./sali/sali.h"
#include "./fast/fast.h"
#include "./verlib/verlib_arttree.h"
#include "./verlib/verlib_btree.h"

template<class KEY_TYPE, class PAYLOAD_TYPE>
indexInterface<KEY_TYPE, PAYLOAD_TYPE> *get_index(std::string index_type) {
  indexInterface<KEY_TYPE, PAYLOAD_TYPE> *index;
  if (index_type == "alexol") {
    // index = new alexolInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if(index_type == "alex") {
    index = new alexInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if (index_type == "btreeolc") {
    index = new BTreeOLCInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  // else if (index_type == "wormhole") {
  //   index = new WormholeInterface<KEY_TYPE, PAYLOAD_TYPE>;
  // }
  else if (index_type == "wormhole_u64") {
    // index = new WormholeU64Interface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if( index_type == "hot") {
    // index = new HotInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if( index_type == "hotrowex") {
    // index = new HotRowexInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if (index_type == "masstree") {
    // index = new MasstreeInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if (index_type == "xindex") {
    index = new xindexInterface<KEY_TYPE, PAYLOAD_TYPE>;
    index->init();
  }
  else if (index_type == "pgm") {
    // index = new pgmInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if(index_type == "btree") {
    // index = new BTreeInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if (index_type == "artolc") {
    // index = new ARTOLCInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  // else if (index_type == "artrowex") {
  //   index = new ARTROWEXInterface<KEY_TYPE, PAYLOAD_TYPE>;
  // }
  else if (index_type == "artunsync") {
    // index = new ARTUnsynchronizedInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if (index_type == "lippol") {
    index = new LIPPOLInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if (index_type == "lipp") {
    index = new LIPPInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if (index_type == "finedex") {
    index = new finedexInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if (index_type == "fh_index_ro") {
    index = new FHIndexROInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if (index_type == "sali") {
    index = new SaliInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if (index_type == "empty") {
    index = new EmptyIndexInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if (index_type == "fast") {
    index = new FastInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if (index_type == "verlib_arttree") {
    index = new VerlibARTTreeInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else if (index_type == "verlib_btree") {
    index = new VerlibBTreeTreeInterface<KEY_TYPE, PAYLOAD_TYPE>;
  }
  else {
    std::cout << "Could not find a matching index called " << index_type << ".\n";
    exit(0);
  }

  return index;
}