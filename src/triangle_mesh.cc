// Author: Marc Comino 2020

#include <triangle_mesh.h>

#include <algorithm>
#include <limits>

namespace data_representation {

TriangleMesh::TriangleMesh() { Clear(); }

void TriangleMesh::Clear() {
  vertices_.clear();
  faces_.clear();
  normals_.clear();

  min_ = Eigen::Vector3f(std::numeric_limits<float>::max(),
                         std::numeric_limits<float>::max(),
                         std::numeric_limits<float>::max());
  max_ = Eigen::Vector3f(std::numeric_limits<float>::lowest(),
                         std::numeric_limits<float>::lowest(),
                         std::numeric_limits<float>::lowest());
}

}  // namespace data_representation
