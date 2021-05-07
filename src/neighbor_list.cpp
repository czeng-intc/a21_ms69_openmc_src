#include "openmc/neighbor_list.h"

namespace openmc{
  
  NeighborList::NeighborList()
  {
    for(int i = 0; i < NEIGHBOR_SIZE; i++)
      list_[i] = -1;
  }
  
  void NeighborList::push_back(int32_t new_elem)
  {
    printf("pushing back %d\n", new_elem);
    for( int i = 0; i < NEIGHBOR_SIZE; i++)
    {
      // This line checks to see if the new_elem is already in the list
      int retrieved_id;

      // OpenMP 5.1
      /*
         #pragma omp atomic compare capture
         {
         retrieved_id = list_[i];
         if (list_[i] == -1)
         list_[i] = new_elem;
         }
         */

      // Compiler non-portable builtin atomic CAS. Unclear if this is sequentially consistent or not (needs to be!)
      //__sync_synchronize();
      retrieved_id = __sync_val_compare_and_swap(&list_[i], -1, new_elem);
      //__sync_synchronize();

      // Case 1: The element was not initialized yet, so the previous line had the effect of setting it to new_elem and returning -1.
      // Case 2: The element was already initialized to the current new_elem, so the atomicCAS call will return new_elem
      if( retrieved_id == -1 || retrieved_id == new_elem)
        return;

      // Case 3: The element was already initialized to a different cell_id, so it will return some other value != -1 and != new_elem
      // so, we continue reading through the list.
    }
  }

} // namespace openmc
