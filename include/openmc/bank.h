#ifndef OPENMC_BANK_H
#define OPENMC_BANK_H

#include <cstdint>
#include <vector>

#include "openmc/shared_array.h"
#include "openmc/particle.h"
#include "openmc/position.h"

namespace openmc {

//==============================================================================
// Global variables
//==============================================================================

namespace simulation {

extern std::vector<Particle::Bank> source_bank;
#pragma omp declare target
extern Particle::Bank* device_source_bank;
#pragma omp end declare target

extern SharedArray<Particle::Bank> surf_source_bank;

#pragma omp declare target
extern SharedArray<Particle::Bank> fission_bank;
#pragma omp end declare target

extern std::vector<int64_t> progeny_per_particle;

#pragma omp declare target
extern int64_t* device_progeny_per_particle;
#pragma omp end declare target

} // namespace simulation

//==============================================================================
// Non-member functions
//==============================================================================

void sort_fission_bank();

void free_memory_bank();

void init_fission_bank(int64_t max);

} // namespace openmc

#endif // OPENMC_BANK_H
