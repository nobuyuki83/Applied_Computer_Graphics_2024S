#include <iostream>
#include <cassert>
#include <filesystem>
// #include <experimental/filesystem> // uncomment here if the <filesystem> cannot be included above
#include <vector>
//
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Eigen/Core"
#include "Eigen/Geometry"


/**
 * Compute 4x4 homogeneous transformation matrix for camera
 * this transformation matrix transform from global coordinate to normalized device coordinates (i.e., [-1,+1]^3 )
 * @return
 */
Eigen::Matrix4f camera_transformation() {
  const float near_clipping_dist = 0.5;
  const float far_clipping_dist = 4.0;
  const float frustrum_near_size = 0.55;
  Eigen::Matrix4f transform = Eigen::Matrix4f::Zero();
  transform(0, 0) = near_clipping_dist / frustrum_near_size;
  transform(1, 1) = near_clipping_dist / frustrum_near_size;
  transform(2, 2) = (far_clipping_dist + near_clipping_dist) / (far_clipping_dist - near_clipping_dist);
  transform(2, 3) = 2.f * far_clipping_dist * near_clipping_dist / (far_clipping_dist - near_clipping_dist);
  transform(3, 2) = -1.f;
  Eigen::Translation3f transl(0., 0., -2.0);
  return transform * Eigen::Affine3f(transl).matrix();
}

/**
 * draw one 3D triangle with texture
 * @param q0 homogeneous coordinate of 3D point 0
 * @param uv0 uv coordinate of point 0
 * @param img_data_out output image data
 * @param img_data_tex texture image data
 */
void draw_3d_triangle_with_texture(
    const Eigen::Vector4f &q0,
    const Eigen::Vector4f &q1,
    const Eigen::Vector4f &q2,
    const Eigen::Vector2f &uv0,
    const Eigen::Vector2f &uv1,
    const Eigen::Vector2f &uv2,
    unsigned int width_out,
    unsigned int height_out,
    std::vector<unsigned char> &img_data_out,
    unsigned int width_tex,
    unsigned int height_tex,
    std::vector<unsigned char> &img_data_tex) {
  for (unsigned int ih = 0; ih < height_out; ++ih) {
    for (unsigned int iw = 0; iw < width_out; ++iw) {
      const auto s = Eigen::Vector2f( // coordinate of the pixel in the normalized device coordinate [-1,1]^2
          ((float(iw) + 0.5f) * 2.f) / float(width_out) - 1.f,
          1.f - ((float(ih) + 0.5f) * 2.f) / float(height_out));
      const auto r0 = q0.hnormalized()({0,1}); // coordinate of the point 0 in the normalized device coordinate [-1,1]^2
      const auto r1 = q1.hnormalized()({0,1});
      const auto r2 = q2.hnormalized()({0,1});
      const float area0 = (r1 - s).cross(r2 - s);  // area of the 2d triangle connecting (s, r1, r2)
      const float area1 = (r2 - s).cross(r0 - s);
      const float area2 = (r0 - s).cross(r1 - s);
      if (area0 < 0. || area1 < 0. || area2 < 0.) { continue; } // the pixel is outside the triangle (r0, r1, r2)
      Eigen::Vector3f bc = Eigen::Vector3f(area0, area1, area2) / (area0 + area1 + area2); // barycentric coordinate on screen
      // `bc` gives the barycentric coordinate **on the screen** and it is distorted.
      // Compute the barycentric coordinate ***on the 3d triangle** below that gives the correct texture mapping.
      // (Hint: formulate a linear system with 4x4 coefficient matrix and solve it to get the barycentric coordinate)
      Eigen::Matrix4f coeff;
      Eigen::Vector4f rhs;

      // do not change below
      auto uv = uv0 * bc[0] + uv1 * bc[1] + uv2 * bc[2]; // uv coordinate of the pixel
      // compute pixel coordinate of the texture
      auto iw_tex = static_cast<unsigned int>((uv[0] - std::floor(uv[0])) * float(width_tex));
      auto ih_tex = static_cast<unsigned int>((1.0 - uv[1] - std::floor(uv[1])) * float(height_tex));
      // write to the output image
      img_data_out[(ih * width_out + iw) * 3 + 0] = img_data_tex[(ih_tex * width_tex + iw_tex) * 3 + 0];
      img_data_out[(ih * width_out + iw) * 3 + 1] = img_data_tex[(ih_tex * width_tex + iw_tex) * 3 + 1];
      img_data_out[(ih * width_out + iw) * 3 + 2] = img_data_tex[(ih_tex * width_tex + iw_tex) * 3 + 2];
    }
  }
}

int main() {
  // texture image data
  int width_tex, height_tex;
  std::vector<unsigned char> img_data_tex;
  { // read texture image
    int bitdepth0;
    const auto input_tex_path = std::filesystem::path(PROJECT_SOURCE_DIR) / ".." / "asset" / "uv.png";
    unsigned char *ptr = stbi_load(input_tex_path.c_str(), &width_tex, &height_tex, &bitdepth0, 0);
    img_data_tex = std::vector<unsigned char>(ptr, ptr + width_tex * height_tex * bitdepth0);
    free(ptr);
  }
  const auto p0 = Eigen::Vector3f(-1.0, -1.0, +1.0); // 3d coordinate of point 0
  const auto p1 = Eigen::Vector3f(+1.0, -1.0, +1.0);
  const auto p2 = Eigen::Vector3f(+1.0, +1.0, -1.0);
  const auto p3 = Eigen::Vector3f(-1.0, +1.0, -1.0);
  const auto uv0 = Eigen::Vector2f(0.0, 0.0); // texture coordinate of point0 (bottom-left of texture image)
  const auto uv1 = Eigen::Vector2f(1.0, 0.0); // texture coordinate of point1 (bottom-right of texture image)
  const auto uv2 = Eigen::Vector2f(1.0, 1.0); // texture coordinate of point2 (top-right of texture image)
  const auto uv3 = Eigen::Vector2f(0.0, 1.0); // texture coordinate of point3 (top-left of texture image)
  // ndc: normalized device coordinate
  Eigen::Matrix4f transform_xyz2ndc = camera_transformation(); // Homography transformation matrix for camera view
  auto q0 = transform_xyz2ndc * p0.homogeneous(); // transformed position of point 0 in homogeneous coordinate
  auto q1 = transform_xyz2ndc * p1.homogeneous();
  auto q2 = transform_xyz2ndc * p2.homogeneous();
  auto q3 = transform_xyz2ndc * p3.homogeneous();
  // output image data
  const unsigned int width_img = 300;
  const unsigned int height_img = 300;
  std::vector<unsigned char> img_data(height_img * width_img * 3, 0); // grayscale image initialized white
  // draw first triangle connecting point 0,1,2
  draw_3d_triangle_with_texture(
      q0, q1, q2,
      uv0, uv1, uv2,
      width_img, height_img, img_data,
      width_tex, height_tex, img_data_tex);
  // draw second triangle connecting point 0,2,3
  draw_3d_triangle_with_texture(
      q0, q2, q3,
      uv0, uv2, uv3,
      width_img, height_img, img_data,
      width_tex, height_tex, img_data_tex);
  // write output image
  const auto output_file_path = std::filesystem::path(PROJECT_SOURCE_DIR) / "output.png";
  stbi_write_png(output_file_path.string().c_str(), width_img, height_img, 3, img_data.data(), width_img * 3);
}
