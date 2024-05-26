#ifndef UTIL_H_
#define UTIL_H_

#include <limits>

namespace acg {

class BvhNode {
 public:
  unsigned int i_node_left;
  unsigned int i_node_right;
  Eigen::Vector3f v_min;
  Eigen::Vector3f v_max;
 public:
  /**
   * check if this BVH node is leaf or not. If it is leaf, `i_node_left` will be the corresponding triangle index
   * @return true if its leaf
   */
  [[nodiscard]] bool is_leaf() const {
    return i_node_right == UINT_MAX;
  }
  /**
   * intersection of ray against the bounding volume
   * @param ray_org ray origin
   * @param ray_dir ray direction
   * @return true if there is intersection
   */
  [[nodiscard]] bool intersect_bv(
      const Eigen::Vector3f& ray_org,
      const Eigen::Vector3f& ray_dir) const {
    float tmin = -INFINITY;
    float tmax = INFINITY;
    for(int i_dim=0;i_dim<3;++i_dim) {
      if (std::fabs(ray_dir[i_dim]) > 1.0e-10f) {
        const float t1 = (v_min[i_dim] - ray_org[i_dim]) / ray_dir[i_dim];
        const float t2 = (v_max[i_dim] - ray_org[i_dim]) / ray_dir[i_dim];
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
      }
      else if( ray_org[i_dim] < v_min[i_dim] || ray_org[i_dim] > v_max[i_dim] ) {
        return false;
      }
    }
    return tmax >= tmin && tmax >= 0.0;
  }
};

// don't need to understand
void build_bvh_topology(
    unsigned int i_start,
    unsigned int i_end,
    bool is_right,
    std::vector<BvhNode> &bvhnodes,
    unsigned int num_branch) {
  if (i_end == i_start + 1) {
    if (is_right) {
      bvhnodes[i_start].i_node_left = num_branch + i_start;
      bvhnodes[i_start].i_node_right = num_branch + i_end;
    } else {
      bvhnodes[i_end].i_node_left = num_branch + i_start;
      bvhnodes[i_end].i_node_right = num_branch + i_end;
    }
    return;
  }
  unsigned int i_split = (i_start + i_end - 1) / 2;
  if (is_right) {
    bvhnodes[i_start].i_node_left = i_split;
    bvhnodes[i_start].i_node_right = i_split + 1;
  } else {
    bvhnodes[i_end].i_node_left = i_split;
    bvhnodes[i_end].i_node_right = i_split + 1;
  }
  build_bvh_topology(i_start, i_split, false, bvhnodes, num_branch);
  build_bvh_topology(i_split + 1, i_end, true, bvhnodes, num_branch);
}

// set the geometry to the bounding volume
void set_bvh_geometry(
    unsigned int i_node,
    std::vector<BvhNode> &bvhnodes,
    Eigen::MatrixX3i &tri2vtx,
    Eigen::MatrixX3f &vtx2xyz)
{
  if (bvhnodes[i_node].is_leaf()) {
    unsigned int i_tri = bvhnodes[i_node].i_node_left;
    auto p0 = vtx2xyz.row(tri2vtx(i_tri, 0));
    auto p1 = vtx2xyz.row(tri2vtx(i_tri, 1));
    auto p2 = vtx2xyz.row(tri2vtx(i_tri, 2));
    bvhnodes[i_node].v_min[0] = std::min({p0[0], p1[0], p2[0]});
    bvhnodes[i_node].v_min[1] = std::min({p0[1], p1[1], p2[1]});
    bvhnodes[i_node].v_min[2] = std::min({p0[2], p1[2], p2[2]});
    bvhnodes[i_node].v_max[0] = std::max({p0[0], p1[0], p2[0]});
    bvhnodes[i_node].v_max[1] = std::max({p0[1], p1[1], p2[1]});
    bvhnodes[i_node].v_max[2] = std::max({p0[2], p1[2], p2[2]});
  } else {
    set_bvh_geometry(bvhnodes[i_node].i_node_left, bvhnodes, tri2vtx, vtx2xyz);
    set_bvh_geometry(bvhnodes[i_node].i_node_right, bvhnodes, tri2vtx, vtx2xyz);
    auto min_left = bvhnodes[bvhnodes[i_node].i_node_left].v_min;
    auto max_left = bvhnodes[bvhnodes[i_node].i_node_left].v_max;
    auto min_right = bvhnodes[bvhnodes[i_node].i_node_right].v_min;
    auto max_right = bvhnodes[bvhnodes[i_node].i_node_right].v_max;
    bvhnodes[i_node].v_min[0] = std::min(min_left[0], min_right[0]);
    bvhnodes[i_node].v_min[1] = std::min(min_left[1], min_right[1]);
    bvhnodes[i_node].v_min[2] = std::min(min_left[2], min_right[2]);
    bvhnodes[i_node].v_max[0] = std::max(max_left[0], max_right[0]);
    bvhnodes[i_node].v_max[1] = std::max(max_left[1], max_right[1]);
    bvhnodes[i_node].v_max[2] = std::max(max_left[2], max_right[2]);
  }
}

// don't need to understand this...
uint16_t int_coord_from_morton(uint16_t i_quad) {
  return (i_quad & 0x0001)
      + ((i_quad & 0x0004) >> 1)
      + ((i_quad & 0x0010) >> 2)
      + ((i_quad & 0x0040) >> 3)
      + ((i_quad & 0x0100) >> 4)
      + ((i_quad & 0x0400) >> 5)
      + ((i_quad & 0x1000) >> 6)
      + ((i_quad & 0x4000) >> 7);
}

void load_scene(
    Eigen::MatrixX3f &vtx2xyz,
    Eigen::MatrixX3i &tri2vtx,
    std::vector<BvhNode> &bvhnodes)
{
  unsigned int num_lev = 7;
  auto num_div = static_cast<unsigned int>(pow(2, num_lev));
  std::cout << "number of elements on an edge of square mesh: " << num_div << std::endl;
  const unsigned int size = num_div + 1;
  const unsigned int num_vtx = size * size;
  vtx2xyz.resize(num_vtx, 3);
  float theta = -3.14 / 4.0;
  for (unsigned int i_h = 0; i_h < size; ++i_h) {
    float y = float(i_h) / float(num_div);
    for (unsigned int i_w = 0; i_w < size; ++i_w) {
      float x = float(i_w) / float(num_div);
      float r = std::sqrt((x-0.5f)*(x-0.5f)+(y-0.5f)*(y-0.5f));
      float z = 0.5f*exp(-10.f*r*r)*sin(30.f*r)-0.5f;
      vtx2xyz(i_h * size + i_w, 0) = x;
      vtx2xyz(i_h * size + i_w, 1) = std::cos(theta)*y - std::sin(theta)*z + 0.4f;
      vtx2xyz(i_h * size + i_w, 2) = std::sin(theta)*y + std::cos(theta)*z;
    }
  }
  const unsigned int num_quad = num_div * num_div;
  const unsigned int num_tri = num_quad * 2;
  tri2vtx.resize(num_tri, 3);
  for (unsigned int i_quad = 0; i_quad < num_quad; ++i_quad) {
    unsigned int i_w = int_coord_from_morton(i_quad);
    unsigned int i_h = int_coord_from_morton(i_quad>>1);
    unsigned int i0_tri = (i_h * num_div + i_w) * 2 + 0;
    tri2vtx(i0_tri, 0) = i_h * size + i_w;
    tri2vtx(i0_tri, 1) = i_h * size + (i_w + 1);
    tri2vtx(i0_tri, 2) = (i_h + 1) * size + (i_w + 1);
    unsigned int i1_tri = (i_h * num_div + i_w) * 2 + 1;
    tri2vtx(i1_tri, 0) = i_h * size + i_w;
    tri2vtx(i1_tri, 1) = (i_h + 1) * size + (i_w + 1);
    tri2vtx(i1_tri, 2) = (i_h + 1) * size + i_w;
  }
  bvhnodes.resize(num_quad * 2 - 1 + num_quad * 2);
  build_bvh_topology(0, num_quad - 1, true, bvhnodes, num_quad - 1);
  for (unsigned int i_quad = 0; i_quad < num_quad; ++i_quad) {
    unsigned int i_node = i_quad + num_quad - 1;
    bvhnodes[i_node].i_node_left = num_quad * 2 - 1 + i_quad * 2 + 0;
    bvhnodes[i_node].i_node_right = num_quad * 2 - 1 + i_quad * 2 + 1;
  }
  for (unsigned int i_tri = 0; i_tri < num_tri; ++i_tri) {
    unsigned int i_node = num_quad * 2 - 1 + i_tri;
    bvhnodes[i_node].i_node_left = i_tri;
    bvhnodes[i_node].i_node_right = -1;
  }
  set_bvh_geometry(0, bvhnodes, tri2vtx, vtx2xyz);

  /*
  for(int i_node=0;i_node<bvhnodes.size();++i_node) {
    const auto& node = bvhnodes[i_node];
    std::cout << i_node << " " << node.i_node_left << " " << node.i_node_right << "   ";
    std::cout << node.v_min[0] << " " << node.v_min[1] << "   ";
    std::cout << node.v_max[0] << " " << node.v_max[1] << std::endl;
  }
   */
}


}

#endif //UTIL_H_
