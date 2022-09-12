#include "openmc/tallies/filter.h"

#include <fmt/core.h>

#include "openmc/error.h"
#include "openmc/surface.h"
#include "openmc/xml_interface.h"

namespace openmc {

void
Filter::SurfaceFilter_from_xml(pugi::xml_node node)
{
  auto surfaces = get_node_array<int32_t>(node, "bins");

  // Convert surface IDs to indices of the global surfaces vector.
  for (auto& s : surfaces) {
    auto search = model::surface_map.find(s);
    if (search == model::surface_map.end()) {
      throw std::runtime_error{fmt::format(
        "Could not find surface {} specified on tally filter.", s)};
    }

    s = search->second;
  }

  this->set_surfaces(surfaces);
}

void
Filter::set_surfaces(gsl::span<int32_t> surfaces)
{
  // Clear existing surfaces
  surfaces_.clear();
  surfaces_.reserve(surfaces.size());
  map_.clear();

  // Update surfaces and mapping
  for (auto& index : surfaces) {
    Expects(index >= 0);
    Expects(index < model::surfaces.size());
    surfaces_.push_back(index);
    map_.insert({index, surfaces_.size() - 1});
  }

  map_.finalize();
  n_bins_ = surfaces_.size();
}

void
Filter::SurfaceFilter_get_all_bins(const Particle& p, TallyEstimator estimator,
                            FilterMatch& match) const
{
  auto search = map_.find(std::abs(p.surface_)-1);
  if (search != map_.end()) {
    //match.bins_.push_back(search->second);
    if (p.surface_ < 0) {
      //match.weights_.push_back(-1.0);
      match.push_back(search->second, -1.0);
    } else {
      //match.weights_.push_back(1.0);
      match.push_back(search->second, 1.0);
    }
  }
}

void
Filter::SurfaceFilter_to_statepoint(hid_t filter_group) const
{
  std::vector<int32_t> surface_ids;
  for (auto c : surfaces_) surface_ids.push_back(model::surfaces[c].id_);
  write_dataset(filter_group, "bins", surface_ids);
}

std::string
Filter::SurfaceFilter_text_label(int bin) const
{
  return fmt::format("Surface {}", model::surfaces[surfaces_[bin]].id_);
}

} // namespace openmc
