#include <iostream>
#include <filesystem>
#include <fstream>
#include <optional>
#include <chrono>
//
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "Eigen/Core"
#include "Eigen/Geometry"
//
#include "util.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <random>
std::mt19937 rndeng(std::random_device{}());

/**
 * Rotation matrix such that z-axis will be direction of `nrm`
 * @param nrm transfromed z-axis
 * @return 3x3 Rotation matrix
 */
auto local_to_world_vector_transformation(
    const Eigen::Vector3f &nrm) -> Eigen::Matrix3f {
  auto basis_x = Eigen::Vector3f(1.f, 0.f, 0.f);
  const auto basis_y = nrm.cross(basis_x).normalized();
  basis_x = basis_y.cross(nrm);
  Eigen::Matrix3f loc2world;
  loc2world << basis_x, basis_y, nrm; // initialization from 3 column vectors
  return loc2world;
}

/**
 * sample a point on a unit hemisphere
 * @param nrm up direction of the hemisphere
 * @return sampled direction and its PDF
 */
auto sample_hemisphere(
    const Eigen::Vector3f &nrm) -> std::pair<Eigen::Vector3f, float> {
  // const auto unirand = Eigen::Vector2f::Random() * 0.5f + Eigen::Vector2f(0.5, 0.5);
  auto dist_01 = std::uniform_real_distribution<float>(0.f, 1.f);
  const auto unirand = Eigen::Vector2f(dist_01(rndeng),dist_01(rndeng));
  const float phi = 2.f * float(M_PI) * unirand.y();

  // the code to uniformly sample hemisphere (z-up)
  const float z = unirand.x();
  const float r = std::sqrt(1.f - z * z);
  auto dir_loc = Eigen::Vector3f( // direction in normal coordinate
      r * std::cos(phi),
      r * std::sin(phi),
      z);
  float pdf = 0.5f / float(M_PI);

  // For Problem 4, write some code below to sample hemisphere with cosign weight
  // (i.e., the sampling frequency is higher at the top)


  // end of Problem 4. Do not modify the two lines below
  const auto dir_out = local_to_world_vector_transformation(nrm) * dir_loc; // rotate the sample (zup -> nrm)
  return {dir_out, pdf};
}

/**
 * ray triangle intersection
 * @param ray_org origin of the ray
 * @param ray_dir direction of the ray
 * @param i_tri index of the triangle
 * @param tri2vtx list of triangle index
 * @param vtx2xyz list of vertex coordinates
 * @return std::nullopt if there is no intersection, otherwise returns a pair of position and normal
 */
auto ray_triangle_intersection(
    const Eigen::Vector3f &ray_org,
    const Eigen::Vector3f &ray_dir,
    unsigned int i_tri,
    const Eigen::MatrixX3i &tri2vtx,
    const Eigen::MatrixX3f &vtx2xyz)
-> std::optional<std::pair<Eigen::Vector3f, Eigen::Vector3f>> {
  auto tet_volume = [](
      const Eigen::Vector3f &p0,
      const Eigen::Vector3f &p1,
      const Eigen::Vector3f &p2,
      const Eigen::Vector3f &p3) { return (p1 - p0).cross(p2 - p0).dot(p3 - p0) / 6.f; };
  auto p0 = vtx2xyz.row(tri2vtx(i_tri, 0));
  auto p1 = vtx2xyz.row(tri2vtx(i_tri, 1));
  auto p2 = vtx2xyz.row(tri2vtx(i_tri, 2));
  float v0 = tet_volume(p1, p2, ray_org + ray_dir, ray_org);
  float v1 = tet_volume(p2, p0, ray_org + ray_dir, ray_org);
  float v2 = tet_volume(p0, p1, ray_org + ray_dir, ray_org);
  if (v0 < 0.f || v1 < 0.f || v2 < 0.f) { return std::nullopt; }
  float sum_inv = 1.f / (v0 + v1 + v2);
  const Eigen::Vector3f q0 = (v0 * p0 + v1 * p1 + v2 * p2) * sum_inv;
  const Eigen::Vector3f n0 = (p1 - p0).cross(p2 - p0).normalized();
  return std::make_pair(q0, n0);
}

/**
 * search ray-triangle intersection in the triangles under the specified branch of BVH
 * @param[in,out] is_hit flag true if there is collision
 * @param[in,out] hit_depth update the minimum depth of the intersection location
 * @param[in, out] hit_pos update the position of the intersection location
 * @param[in, out] hit_normal update normal at the intersection location
 * @param[in] i_bvhnode index of BVH node of branch to search intersection
 * @param[in] ray_org ray origin
 * @param[in] ray_dir ray direction
 * @param[in] tri2vtx triangle index
 * @param[in] vtx2xyz list of coordinates
 * @param[in] bvhnodes list of BVH nodes
 */
void search_collision_in_bvh(
    bool &is_hit,
    float &hit_depth,
    Eigen::Vector3f &hit_pos,
    Eigen::Vector3f &hit_normal,
    unsigned int i_bvhnode,
    const Eigen::Vector3f &ray_org,
    const Eigen::Vector3f &ray_dir,
    const Eigen::MatrixX3i &tri2vtx,
    const Eigen::MatrixX3f &vtx2xyz,
    const std::vector<acg::BvhNode> &bvhnodes) {
  // For problem 2, implement some code here to evaluate BVH
  // hint: use following function
  //   bvhnodes[i_bvhnode].intersect_bv(ray_org, ray_dir)

  if (bvhnodes[i_bvhnode].is_leaf()) { // this is leaf node
    const unsigned int i_tri = bvhnodes[i_bvhnode].i_node_left;
    // do something
  } else { // this is branch node
    unsigned int i_node_right = bvhnodes[i_bvhnode].i_node_right;
    unsigned int i_node_left =bvhnodes[i_bvhnode].i_node_left;
    // do something (hint recursion)
  }
}

auto find_intersection_between_ray_and_triangle_mesh(
    const Eigen::Vector3f &ray_org,
    const Eigen::Vector3f &ray_dir,
    const Eigen::MatrixX3i &tri2vtx,
    const Eigen::MatrixX3f &vtx2xyz,
    const std::vector<acg::BvhNode> &bvhnodes)
-> std::optional<std::pair<Eigen::Vector3f, Eigen::Vector3f>> {
  bool is_hit = false;
  float hit_depth = 1000.;
  Eigen::Vector3f hit_pos;
  Eigen::Vector3f hit_normal;

  // for Problem 2,3,4, comment out from here
  for (unsigned int i_tri = 0; i_tri < tri2vtx.rows(); ++i_tri) {
    const auto res = ray_triangle_intersection(ray_org, ray_dir, i_tri, tri2vtx, vtx2xyz);
    if (!res) { continue; }
    const auto& [q0,n0] = res.value();
    const float depth = (q0 - ray_org).dot(ray_dir);
    if (hit_depth > depth) {
      is_hit = true;
      hit_depth = depth;
      hit_pos = q0;
      hit_normal = n0;
    }
  }
  // comment out end

  // do not edit from here
  search_collision_in_bvh(
      is_hit, hit_depth, hit_pos, hit_normal,
      0, // root node index
      ray_org, ray_dir, tri2vtx, vtx2xyz, bvhnodes);
  //
  if (!is_hit) { return std::nullopt; }
  return std::make_pair(hit_pos, hit_normal);
}

auto get_ray_from_camera(
    unsigned int width, unsigned int height,
    unsigned int iw, unsigned int ih) -> std::pair<Eigen::Vector3f, Eigen::Vector3f> {
  auto cam_ray_src = Eigen::Vector3f(0.5, 0.5, 2.0); // focus point
  float ndc_x = ((float(iw) + 0.5f) * 2.f / float(width) - 1.f); // normalized device x-coordinate [-1, +1]
  float ndc_y = (1.f - (float(ih) + 0.5f) * 2.f / float(height)); // normalized device y-coordinate [-1, +1]
  float sensor_size = 0.25;
  Eigen::Vector3f position_on_sensor = Eigen::Vector3f(ndc_x * sensor_size, ndc_y * sensor_size, -1.0) + cam_ray_src;
  Eigen::Vector3f cam_ray_dir = (position_on_sensor - cam_ray_src).normalized();
  return {cam_ray_src, cam_ray_dir};
}

int main() {
  Eigen::MatrixX3f vtx2xyz;
  Eigen::MatrixX3i tri2vtx;
  std::vector<acg::BvhNode> bvhnodes;
  acg::load_scene(vtx2xyz, tri2vtx, bvhnodes);

  const unsigned int img_width = 100;
  const unsigned int img_height = 100;
  //
  std::vector<float> img_data_nrm(img_height * img_width * 3, 0.f);
  std::vector<float> img_data_ao(img_height * img_width, 0.f);
  //
  std::chrono::system_clock::time_point start = std::chrono::system_clock::now(); // record starting time
  for (unsigned int iw = 0; iw < img_width; ++iw) {
    for (unsigned int ih = 0; ih < img_height; ++ih) {
      const auto[cam_ray_src, cam_ray_dir] = get_ray_from_camera(img_width, img_height, iw, ih);
      const auto& res = find_intersection_between_ray_and_triangle_mesh(
          cam_ray_src, cam_ray_dir, tri2vtx, vtx2xyz, bvhnodes);
      // draw normal map
      if (!res) {
        img_data_nrm[(ih * img_width + iw) * 3 + 0] = 0.f;
        img_data_nrm[(ih * img_width + iw) * 3 + 1] = 0.f;
        img_data_nrm[(ih * img_width + iw) * 3 + 2] = 0.f;
      } else {
        const auto&[pos, nrm] = res.value(); // position and normal of the first hit point
        img_data_nrm[(ih * img_width + iw) * 3 + 0] = nrm.x() * 0.5f + 0.5f;
        img_data_nrm[(ih * img_width + iw) * 3 + 1] = nrm.y() * 0.5f + 0.5f;
        img_data_nrm[(ih * img_width + iw) * 3 + 2] = nrm.z() * 0.5f + 0.5f;
      }
      continue; // comment out here for Problem 3,4
      //
      if (res) { // ambient occlusion computation
        const unsigned int num_sample_ao = 100;
        float sum = 0;
        for (unsigned int i_sample = 0; i_sample < num_sample_ao; ++i_sample) {
          const auto&[pos, nrm] = res.value(); // position and normal of the first hit point
          Eigen::Vector3f pos0 = pos + nrm * 0.001f; // offset the position in the direction of normal
          const auto[dir, pdf] = sample_hemisphere(nrm); // direction of the sampled light position and its PDF
          const auto res1 = find_intersection_between_ray_and_triangle_mesh(
              pos0, dir, tri2vtx, vtx2xyz, bvhnodes);
          if (!res1) { // if the ray doe not hit anything
            sum += 1.f; // Problem 3: This is a bug. write some correct code (hint: use `dir.dot(nrm)`, `pdf`, `M_PI`).
          }
        }
        img_data_ao[ih * img_width + iw] = sum / float(num_sample_ao); // do not change
      }
    }
  }
  std::chrono::system_clock::time_point end = std::chrono::system_clock::now(); // record end time
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  std::cout << "total computation time: " << elapsed << "ms" << std::endl;

  {
    std::vector<unsigned char> img_data_uchar(img_width * img_height * 3, 0);
    for (unsigned int i = 0; i < img_width * img_height; ++i) {
      img_data_uchar[i * 3 + 0] = static_cast<unsigned char>(img_data_nrm[i * 3 + 0] * 255.f);
      img_data_uchar[i * 3 + 1] = static_cast<unsigned char>(img_data_nrm[i * 3 + 1] * 255.f);
      img_data_uchar[i * 3 + 2] = static_cast<unsigned char>(img_data_nrm[i * 3 + 2] * 255.f);
    }
    stbi_write_png(
        (std::filesystem::path(PROJECT_SOURCE_DIR) / "normal_map.png").string().c_str(),
        img_width, img_height, 3, img_data_uchar.data(), 0);
  }
  {
    std::vector<unsigned char> img_data_uchar(img_width * img_height, 0);
    for (unsigned int i = 0; i < img_width * img_height; ++i) {
      img_data_uchar[i] = static_cast<unsigned char>(img_data_ao[i] * 255.f);
    }
    stbi_write_png(
        (std::filesystem::path(PROJECT_SOURCE_DIR) / "ao.png").string().c_str(),
        img_width, img_height, 1, img_data_uchar.data(), 0);
  }
}

