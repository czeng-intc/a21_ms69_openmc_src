#include "openmc/tallies/filter.h"

#include <fmt/core.h>

#include "openmc/xml_interface.h"

namespace openmc {

void
Filter::ParticleFilter_from_xml(pugi::xml_node node)
{
  auto particles = get_node_array<std::string>(node, "bins");

  // Convert to vector of Particle::Type
  std::vector<Particle::Type> types;
  for (auto& p : particles) {
    types.push_back(str_to_particle_type(p));
  }
  this->set_particles(types);
}

void
Filter::set_particles(gsl::span<Particle::Type> particles)
{
  // Clear existing particles
  particles_.clear();
  particles_.reserve(particles.size());

  // Set particles and number of bins
  for (auto p : particles) {
    particles_.push_back(p);
  }
  n_bins_ = particles_.size();
}

void
Filter::ParticleFilter_get_all_bins(const Particle& p, TallyEstimator estimator,
                             FilterMatch& match) const
{
  for (auto i = 0; i < particles_.size(); i++) {
    if (particles_[i] == p.type_) {
      //match.bins_.push_back(i);
      //match.weights_.push_back(1.0);
      match.push_back(i, 1.0);
    }
  }
}

void
Filter::ParticleFilter_to_statepoint(hid_t filter_group) const
{
  std::vector<std::string> particles;
  for (auto p : particles_) {
    particles.push_back(particle_type_to_str(p));
  }
  write_dataset(filter_group, "bins", particles);
}

std::string
Filter::ParticleFilter_text_label(int bin) const
{
  const auto& p = particles_.at(bin);
  return fmt::format("Particle: {}", particle_type_to_str(p));
}

} // namespace openmc
