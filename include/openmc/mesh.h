//! \file mesh.h
//! \brief Mesh types used for tallies, Shannon entropy, CMFD, etc.

#ifndef OPENMC_MESH_H
#define OPENMC_MESH_H

#include <memory> // for unique_ptr
#include <unordered_map>

#include "hdf5.h"
#include "pugixml.hpp"
#include <iostream>
#include "xtensor/xtensor.hpp"

#include "openmc/particle.h"
#include "openmc/position.h"
#include "openmc/vector.h"

#ifdef DAGMC
#include "moab/Core.hpp"
#include "moab/AdaptiveKDTree.hpp"
#include "moab/Matrix3.hpp"
#include "moab/GeomUtil.hpp"
#endif

namespace openmc {

//==============================================================================
// Global variables
//==============================================================================

class Mesh;

namespace model {

// TODO: Need to declare the mesh_map as target
extern std::unordered_map<int32_t, int32_t> mesh_map;
#pragma omp declare target
extern Mesh* meshes;
extern int32_t meshes_size;
#pragma omp end declare target

} // namespace model


class Mesh
{
public:
  // Constructors and destructor
  Mesh() = default;
  Mesh(pugi::xml_node node);
  ~Mesh() = default;

  // Methods

  #pragma omp declare target
  //! Determine which bins were crossed by a particle
  //
  //! \param[in] p Particle to check
  //! \param[out] bins Bins that were crossed
  //! \param[out] lengths Fraction of tracklength in each bin
  //virtual void bins_crossed(const Particle* p, std::vector<int>& bins,
                            //std::vector<double>& lengths) const = 0;
  void bins_crossed(const Particle& p, FilterMatch& match) const;

  //! Determine which surface bins were crossed by a particle
  //
  //! \param[in] p Particle to check
  //! \param[out] bins Surface bins that were crossed
  //virtual void
  //surface_bins_crossed(const Particle* p, std::vector<int>& bins) const = 0;
  void
  surface_bins_crossed(const Particle& p, FilterMatch& match) const;

  //! Get bin at a given position in space
  //
  //! \param[in] r Position to get bin for
  //! \return Mesh bin
  int get_bin(Position r) const;
  #pragma omp end declare target

  //! Get the number of mesh cells.
  int n_bins() const;

  //! Get the number of mesh cell surfaces.
  int n_surface_bins() const;

  //! Write mesh data to an HDF5 group
  //
  //! \param[in] group HDF5 group
  void to_hdf5(hid_t group) const;

  //! Find the mesh lines that intersect an axis-aligned slice plot
  //
  //! \param[in] plot_ll The lower-left coordinates of the slice plot.
  //! \param[in] plot_ur The upper-right coordinates of the slice plot.
  //! \return A pair of vectors indicating where the mesh lines lie along each
  //!   of the plot's axes.  For example an xy-slice plot will get back a vector
  //!   of x-coordinates and another of y-coordinates.  These vectors may be
  //!   empty for low-dimensional meshes.
  std::pair<std::vector<double>, std::vector<double>>
  plot(Position plot_ll, Position plot_ur) const;

  //! Get a label for the mesh bin
  std::string bin_label(int bin) const;

  //! Count number of bank sites in each mesh bin / energy bin
  //
  //! \param[in] Pointer to bank sites
  //! \param[in] Number of bank sites
  //! \param[out] Whether any bank sites are outside the mesh
  xt::xtensor<double, 1> count_sites(const Particle::Bank* bank,
                                     int64_t length, bool* outside) const;

  //! Get bin given mesh indices
  //
  //! \param[in] Array of mesh indices
  //! \return Mesh bin
  int get_bin_from_indices(const int* ijk) const;

  //! Get mesh indices given a position
  //
  //! \param[in] r Position to get indices for
  //! \param[out] ijk Array of mesh indices
  //! \param[out] in_mesh Whether position is in mesh
  void get_indices(Position r, int* ijk, bool* in_mesh) const;

  //! Get mesh indices corresponding to a mesh bin
  //
  //! \param[in] bin Mesh bin
  //! \param[out] ijk Mesh indices
  void get_indices_from_bin(int bin, int* ijk) const;

  //! Get mesh index in a particular direction
  //!
  //! \param[in] r Coordinate to get index for
  //! \param[in] i Direction index
  int get_index_in_direction(double r, int i) const;

  //! Check where a line segment intersects the mesh and if it intersects at all
  //
  //! \param[in,out] r0 In: starting position, out: intersection point
  //! \param[in] r1 Ending position
  //! \param[out] ijk Indices of the mesh bin containing the intersection point
  //! \return Whether the line segment connecting r0 and r1 intersects mesh
  bool intersects(Position& r0, Position r1, int* ijk) const;

  //! Get the coordinate for the mesh grid boundary in the positive direction
  //!
  //! \param[in] ijk Array of mesh indices
  //! \param[in] i Direction index
  double positive_grid_boundary(int* ijk, int i) const;

  //! Get the coordinate for the mesh grid boundary in the negative direction
  //!
  //! \param[in] ijk Array of mesh indices
  //! \param[in] i Direction index
  double negative_grid_boundary(int* ijk, int i) const;

  bool intersects_1d(Position& r0, Position r1, int* ijk) const;
  bool intersects_2d(Position& r0, Position r1, int* ijk) const;
  bool intersects_3d(Position& r0, Position r1, int* ijk) const;

  void copy_to_device();

  // Data members
  int id_ {-1};  //!< User-specified ID
  int n_dimension_; //!< Number of dimensions

  // Data members
  vector<double> lower_left_; //!< Lower-left coordinates of mesh
  vector<double> upper_right_; //!< Upper-right coordinates of mesh
  vector<int> shape_; //!< Number of mesh elements in each dimension

  double volume_frac_; //!< Volume fraction of each mesh element
  vector<double> width_; //!< Width of each mesh element
};


/*
class StructuredMesh : public Mesh {
public:
  StructuredMesh() = default;
  StructuredMesh(pugi::xml_node node) : Mesh {node} {};
  virtual ~StructuredMesh() = default;

  int get_bin(Position r) const override;

  int n_bins() const override;

  int n_surface_bins() const override;

  //void bins_crossed(const Particle& p, std::vector<int>& bins,
  //                  std::vector<double>& lengths) const override;
  void bins_crossed(const Particle& p, FilterMatch& match) const override;

  //! Count number of bank sites in each mesh bin / energy bin
  //
  //! \param[in] Pointer to bank sites
  //! \param[in] Number of bank sites
  //! \param[out] Whether any bank sites are outside the mesh
  xt::xtensor<double, 1> count_sites(const Particle::Bank* bank,
                                     int64_t length, bool* outside) const;

  //! Get bin given mesh indices
  //
  //! \param[in] Array of mesh indices
  //! \return Mesh bin
  virtual int get_bin_from_indices(const int* ijk) const;

  //! Get mesh indices given a position
  //
  //! \param[in] r Position to get indices for
  //! \param[out] ijk Array of mesh indices
  //! \param[out] in_mesh Whether position is in mesh
  virtual void get_indices(Position r, int* ijk, bool* in_mesh) const;

  //! Get mesh indices corresponding to a mesh bin
  //
  //! \param[in] bin Mesh bin
  //! \param[out] ijk Mesh indices
  virtual void get_indices_from_bin(int bin, int* ijk) const;

  //! Get mesh index in a particular direction
  //!
  //! \param[in] r Coordinate to get index for
  //! \param[in] i Direction index
  virtual int get_index_in_direction(double r, int i) const = 0;

  //! Check where a line segment intersects the mesh and if it intersects at all
  //
  //! \param[in,out] r0 In: starting position, out: intersection point
  //! \param[in] r1 Ending position
  //! \param[out] ijk Indices of the mesh bin containing the intersection point
  //! \return Whether the line segment connecting r0 and r1 intersects mesh
  virtual bool intersects(Position& r0, Position r1, int* ijk) const;

  //! Get the coordinate for the mesh grid boundary in the positive direction
  //!
  //! \param[in] ijk Array of mesh indices
  //! \param[in] i Direction index
  virtual double positive_grid_boundary(int* ijk, int i) const = 0;

  //! Get the coordinate for the mesh grid boundary in the negative direction
  //!
  //! \param[in] ijk Array of mesh indices
  //! \param[in] i Direction index
  virtual double negative_grid_boundary(int* ijk, int i) const = 0;

  //! Get a label for the mesh bin
  std::string bin_label(int bin) const override;

  // Data members
  xt::xtensor<double, 1> lower_left_; //!< Lower-left coordinates of mesh
  xt::xtensor<double, 1> upper_right_; //!< Upper-right coordinates of mesh
  xt::xtensor<int, 1> shape_; //!< Number of mesh elements in each dimension

protected:
  virtual bool intersects_1d(Position& r0, Position r1, int* ijk) const;
  virtual bool intersects_2d(Position& r0, Position r1, int* ijk) const;
  virtual bool intersects_3d(Position& r0, Position r1, int* ijk) const;
};

//==============================================================================
//! Tessellation of n-dimensional Euclidean space by congruent squares or cubes
//==============================================================================

class RegularMesh : public StructuredMesh
{
public:
  // Constructors
  RegularMesh() = default;
  RegularMesh(pugi::xml_node node);

  // Overriden methods

  //void bins_crossed(const Particle* p, std::vector<int>& bins,
  //                  std::vector<double>& lengths) const override;
  //void bins_crossed(const Particle& p, FilterMatch& match) const override;

  //void surface_bins_crossed(const Particle* p, std::vector<int>& bins)
  //const override;
  void surface_bins_crossed(const Particle& p, FilterMatch& match) const override;

  int get_index_in_direction(double r, int i) const override;

  double positive_grid_boundary(int* ijk, int i) const override;

  double negative_grid_boundary(int* ijk, int i) const override;

  std::pair<std::vector<double>, std::vector<double>>
  plot(Position plot_ll, Position plot_ur) const override;

  void to_hdf5(hid_t group) const override;

  // Data members

  double volume_frac_; //!< Volume fraction of each mesh element
  xt::xtensor<double, 1> width_; //!< Width of each mesh element
};


class RectilinearMesh : public StructuredMesh
{
public:
  // Constructors
  RectilinearMesh() = default;
  RectilinearMesh(pugi::xml_node node);

  // Overriden methods

  //void bins_crossed(const Particle* p, std::vector<int>& bins,
    //                std::vector<double>& lengths) const override;
  //void bins_crossed(const Particle& p, FilterMatch& match) const override;

  //void surface_bins_crossed(const Particle* p, std::vector<int>& bins)
  //const override;

  void surface_bins_crossed(const Particle& p, FilterMatch& match)
  const override;

  int get_index_in_direction(double r, int i) const override;

  double positive_grid_boundary(int* ijk, int i) const override;

  double negative_grid_boundary(int* ijk, int i) const override;

  std::pair<std::vector<double>, std::vector<double>>
  plot(Position plot_ll, Position plot_ur) const override;

  void to_hdf5(hid_t group) const override;

  std::vector<std::vector<double>> grid_;

  int set_grid();
};

#ifdef DAGMC

class UnstructuredMesh : public Mesh {

public:
  UnstructuredMesh() = default;
  UnstructuredMesh(pugi::xml_node);
  ~UnstructuredMesh() = default;

  //void bins_crossed(const Particle* p,
    //                std::vector<int>& bins,
      //              std::vector<double>& lengths) const override;
  void bins_crossed(const Particle& p, FilterMatch& match) const override;

  std::pair<std::vector<double>, std::vector<double>>
  plot(Position plot_ll, Position plot_ur) const override;

  //! Determine which surface bins were crossed by a particle.
  //
  //! \param[in] p Particle to check
  //! \param[out] bins Surface bins that were crossed
  //void surface_bins_crossed(const Particle* p, std::vector<int>& bins) const;
  void surface_bins_crossed(const Particle& p, FilterMatch& match) const;

  //! Write mesh data to an HDF5 group.
  //
  //! \param[in] group HDF5 group
  void to_hdf5(hid_t group) const;

  //! Get bin at a given position.
  //
  //! \param[in] r Position to get bin for
  //! \return Mesh bin
  int get_bin(Position r) const;

  int n_bins() const override;

  int n_surface_bins() const override;

  //! Retrieve a centroid for the mesh cell
  //
  // \param[in] tet MOAB EntityHandle of the tetrahedron
  // \return The centroid of the element
  Position centroid(moab::EntityHandle tet) const;

  //! Return a string represntation of the mesh bin
  //
  //! \param[in] bin Mesh bin to generate a label for
  std::string bin_label(int bin) const override;

  //! Add a score to the mesh instance
  void add_score(std::string score) const;

  //! Remove a score from the mesh instance
  void remove_score(std::string score) const;

  //! Set data for a score
  void set_score_data(const std::string& score,
                      std::vector<double> values,
                      std::vector<double> std_dev) const;

  //! Write the mesh with any current tally data
  void write(std::string base_filename) const;

  std::string filename_; //!< Path to unstructured mesh file

private:

  //! Find all intersections with faces of the mesh.
  //
  //! \param[in] start Staring location
  //! \param[in] dir Normalized particle direction
  //! \param[in] track_len length of particle track
  //! \param[out] Mesh intersections
  void
  intersect_track(const moab::CartVect& start,
                  const moab::CartVect& dir,
                  double track_len,
                  std::vector<double>& hits) const;

  //! Calculate the volume for a given tetrahedron handle.
  //
  // \param[in] tet MOAB EntityHandle of the tetrahedron
  double tet_volume(moab::EntityHandle tet) const;

  //! Find the tetrahedron for the given location if
  //! one exists
  //
  //! \param[in]
  //! \return MOAB EntityHandle of tet
  moab::EntityHandle get_tet(const Position& r) const;

  //! Return the containing tet given a position
  moab::EntityHandle get_tet(const moab::CartVect& r) const {
      return get_tet(Position(r[0], r[1], r[2]));
  };

  //! Check for point containment within a tet; uses
  //! pre-computed barycentric data.
  //
  //! \param[in] r Position to check
  //! \param[in] MOAB terahedron to check
  //! \return True if r is inside, False if r is outside
  bool point_in_tet(const moab::CartVect& r,
                    moab::EntityHandle tet) const;

  //! Compute barycentric coordinate data for all tetrahedra
  //! in the mesh.
  //
  //! \param[in] tets MOAB Range of tetrahedral elements
  void compute_barycentric_data(const moab::Range& tets);

  //! Translate a MOAB EntityHandle to its corresponding bin.
  //
  //! \param[in] eh MOAB EntityHandle to translate
  //! \return Mesh bin
  int get_bin_from_ent_handle(moab::EntityHandle eh) const;

  //! Translate a bin to its corresponding MOAB EntityHandle
  //! for the tetrahedron representing that bin.
  //
  //! \param[in] bin Bin value to translate
  //! \return MOAB EntityHandle of tet
  moab::EntityHandle get_ent_handle_from_bin(int bin) const;

  //! Get the bin for a given mesh cell index
  //
  //! \param[in] idx Index of the mesh cell.
  //! \return Mesh bin
  int get_bin_from_index(int idx) const;

  //! Get the mesh cell index for a given position
  //
  //! \param[in] r Position to get index for
  //! \param[in,out] in_mesh Whether position is in the mesh
  int get_index(const Position& r, bool* in_mesh) const;

  //! Get the mesh cell index from a bin
  //
  //! \param[in] bin Bin to get the index for
  //! \return Index of the bin
  int get_index_from_bin(int bin) const;

  //! Build a KDTree for all tetrahedra in the mesh. All
  //! triangles representing 2D faces of the mesh are
  //! added to the tree as well.
  //
  //! \param[in] all_tets MOAB Range of tetrahedra for the tree
  void build_kdtree(const moab::Range& all_tets);

  //! Get the tags for a score from the mesh instance
  //! or create them if they are not there
  //
  //! \param[in] score Name of the score
  //! \return The MOAB value and error tag handles, respectively
  std::pair<moab::Tag, moab::Tag>
  get_score_tags(std::string score) const;

  // data members
  moab::Range ehs_; //!< Range of tetrahedra EntityHandle's in the mesh
  moab::EntityHandle tetset_; //!< EntitySet containing all tetrahedra
  moab::EntityHandle kdtree_root_; //!< Root of the MOAB KDTree
  std::unique_ptr<moab::Interface> mbi_; //!< MOAB instance
  std::unique_ptr<moab::AdaptiveKDTree> kdtree_; //!< MOAB KDTree instance
  std::vector<moab::Matrix3> baryc_data_; //!< Barycentric data for tetrahedra
};

#endif
*/

//==============================================================================
// Non-member functions
//==============================================================================

//! Read meshes from either settings/tallies
//
//! \param[in] root XML node
void read_meshes(pugi::xml_node root);

//! Write mesh data to an HDF5 group
//
//! \param[in] group HDF5 group
void meshes_to_hdf5(hid_t group);

void free_memory_mesh();

} // namespace openmc

#endif // OPENMC_MESH_H
