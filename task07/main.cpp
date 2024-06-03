#include <iostream>
#include <filesystem>
#include <limits>
#include <tuple>
#include <random>
#include <optional>
#include <cmath>
//
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Eigen/Core"
#include "Eigen/Geometry"

/**
 * ray - sphere intersection
 * @param ray_src
 * @param ray_dir
 * @param center center of sphere
 * @param rad radius of sphere
 * @return if it does not hit return nullopt, if hit return position normal, and depth of hit position
 */
auto intersection_ray_sphere(
    const Eigen::Vector3f &ray_src,
    const Eigen::Vector3f &ray_dir,
    const Eigen::Vector3f &center,
    float rad) -> std::optional<std::tuple<Eigen::Vector3f, Eigen::Vector3f, float>> {
  float depth0 = (center - ray_src).dot(ray_dir);
  if (depth0 < 0.f) { return {}; }
  float sqdist = (ray_src + depth0 * ray_dir - center).squaredNorm();
  if (rad * rad - sqdist < 0.f) { return {}; }
  float depth1 = depth0 - sqrt(rad * rad - sqdist);
  if (depth1 < 0.f) { return {}; }
  auto hit_pos = ray_src + depth1 * ray_dir;
  auto hit_normal = (hit_pos - center).normalized();
  return std::make_tuple(hit_pos, hit_normal, depth1);
}

auto local_to_world_vector_transformation(
    const Eigen::Vector3f &nrm) -> Eigen::Matrix3f {
  auto basis_x = Eigen::Vector3f(1.f, 0.f, 0.f);
  const auto basis_y = nrm.cross(basis_x).normalized();
  basis_x = basis_y.cross(nrm);
  Eigen::Matrix3f loc2world;
  loc2world << basis_x, basis_y, nrm; // initialization from 3 column vectors
  return loc2world;
}

auto sampling_brdf_lambert(
    const Eigen::Vector3f &nrm,
    const Eigen::Vector2f &unirand) -> std::pair<Eigen::Vector3f, float> {
  const float r = std::sqrt(unirand.x());
  const float phi = 2.f * float(M_PI) * unirand.y();
  const float z = std::sqrt(1.f - r * r);
  const auto dir_loc = Eigen::Vector3f( // direction in normal coordinate
      r * std::cos(phi), // this is std::sqrt(u0)*std::cos(phi)
      r * std::sin(phi), // this is std::sqrt(u0)*std::sin(phi)
      z); // this can be std::sqrt(1-u0)
  const Eigen::Matrix3f loc2world = local_to_world_vector_transformation(nrm);
  const Eigen::Vector3f dir_out = loc2world * dir_loc;
  return {dir_out, 1.f / float(M_PI)};
}

auto sampling_brdf_specular(
    const Eigen::Vector3f &nrm,
    const Eigen::Vector3f &dir_in,
    float shiness,
    const Eigen::Vector2f &unirand) -> std::pair<Eigen::Vector3f, float> {
  const Eigen::Vector3f dir_mirror = dir_in - 2.f * dir_in.dot(nrm) * nrm;
  const float phi = 2.f * float(M_PI) * unirand.y();
  const float cos_alpha = std::pow(1.f - unirand.x(), 1.f / (shiness + 1.f));
  const float sin_alpha = std::sqrt(std::max(0.f, 1.f - cos_alpha * cos_alpha));
  const auto dir_loc = Eigen::Vector3f(
      sin_alpha * std::cos(phi),
      sin_alpha * std::sin(phi),
      cos_alpha);
  const Eigen::Matrix3f loc2world = local_to_world_vector_transformation(dir_mirror);
  const Eigen::Vector3f dir_out = loc2world * dir_loc;
  assert(dir_out.dot(nrm) > 0.f);
  float brdf = std::pow(cos_alpha, shiness) * (shiness + 1.f) / (2.f * float(M_PI));
  return {dir_out, brdf};
}

/**
 * PDF in the BRDF sampling for Phong material
 * @param nrm normal
 * @param dir_in incoming light direction
 * @param dir_out outgoing light direction
 * @param ratio_diffuse how much incoming light diffuse
 * @param ratio_specular how much incoming light reflect as the specular light
 * @param shiness
 * @return probability density
 */
float pdf_brdf_phong(
    const Eigen::Vector3f &nrm,
    const Eigen::Vector3f &dir_in,
    const Eigen::Vector3f &dir_out,
    float ratio_diffuse,
    float ratio_specular,
    float shiness) {
  float pdf_diffuse = dir_out.dot(nrm) / float(M_PI);
  const Eigen::Vector3f dir_mirror = dir_in - 2.f * dir_in.dot(nrm) * nrm;
  float cos_alpha = dir_mirror.dot(dir_out);
  float pdf_specular = std::pow(cos_alpha, shiness) * (shiness + 1.f) / (2.f * float(M_PI));
  return (pdf_diffuse * ratio_diffuse + pdf_specular * ratio_specular) / (ratio_diffuse + ratio_specular);
}

class Sphere {
 public:
  const Eigen::Vector3f pos;
  const float rad;
  const float shiness;
  const float ratio_specular;
  const float ratio_diffuse;
  const float emission;
 public:
  /**
   * sampling the incoming light direction based on BRDF
   * @param nrm  normal of the surface
   * @param dir_in outgoing light direction
   * @param rdeng random number generator
   * @return incoming light direction
   */
  [[nodiscard]] auto sample_reflection_based_on_brdf(
      const Eigen::Vector3f &nrm,
      const Eigen::Vector3f &dir_out,
      std::mt19937& rdeng) const -> Eigen::Vector3f {
    float sum_ratio = ratio_specular + ratio_diffuse;
    if (ratio_specular <= 0.f && ratio_diffuse <= 0.f) { return {1., 0., 0,}; }
    auto udist01 = std::uniform_real_distribution<float>(0.f,1.f);
    const Eigen::Vector2f unirand(udist01(rdeng), udist01(rdeng));
    const float rnd0 = udist01(rdeng);

    Eigen::Vector3f dir_world(0., 0., 0.);
    if (rnd0 < ratio_diffuse / sum_ratio) { // diffuse
      auto hoge = sampling_brdf_lambert(nrm, unirand);
      dir_world = hoge.first;
    } else { // specular
      auto hoge = sampling_brdf_specular(nrm, dir_out, shiness, unirand);
      dir_world = hoge.first;
    }
    return dir_world;
  }
  /**
   * BRDF value
   * @param dir_in incoming light direction
   * @param dir_out outgoing light direction
   * @param dir_nrm normal direction
   * @return brdf value
   */
  [[nodiscard]] float brdf(
      const Eigen::Vector3f &dir_in,
      const Eigen::Vector3f &dir_out,
      const Eigen::Vector3f &dir_nrm) const {
    if (ratio_specular <= 0.f && ratio_diffuse <= 0.f) { return 0.f; }
    const Eigen::Vector3f dir_mirror = dir_in - 2.f * dir_in.dot(dir_nrm) * dir_nrm;
    float cos_alpha = dir_mirror.dot(dir_out);
    float brdf_specular = std::powf(cos_alpha, shiness) * (shiness + 1.f) / (2.f * float(M_PI));
    float brdf_diffuse = 1.f / float(M_PI);
    return brdf_specular * ratio_specular + brdf_diffuse * ratio_diffuse;
  }
  /**
   * Probability density function for BRDF sampling with specified given in/out directions
   * @param nrm normal of the surface
   * @param dir_in incoming light direction
   * @param dir_out outgoing light direction
   * @return PDF
   */
  [[nodiscard]]float pdf(
      const Eigen::Vector3f &nrm,
      const Eigen::Vector3f &dir_in,
      const Eigen::Vector3f &dir_out) const {
    return pdf_brdf_phong(
        nrm, dir_in, dir_out,
        ratio_diffuse, ratio_specular, shiness);
  }
};

// --------------------------------------

const Sphere spheres[4] = {
    {
        {1.0, 1.0, 0.0}, // position
        0.4, // rad
        2000.f, // shiness
        0.0f, // specular
        0.0f, // diffuse
        1.f // emission
    },
    {
        {-1.0, -1.0, -1.0}, // position
        1.0, // rad
        2000.f, // shiness
        0.8f, // specular
        0.2f, // diffuse
        0.f // emission
    },
    {
        {+1.0, -1.0, -1.0}, // position
        1.0,
        2000.f,
        0.5f,
        0.5f,
        0.f
    },
    {
      {-1.0, +1.0, -1.0}, // position
      1.0,
          2000.f,
          0.2f,
          0.8f,
          0.f
    }
};

/**
 * Search Ray and screen hit
 * @param ray_src
 * @param ray_dir
 * @return if hit return position, normal, and hit object index, otherwise return nuullopt
 */
auto hit_scene(
    const Eigen::Vector3f &ray_src,
    const Eigen::Vector3f &ray_dir)
-> std::tuple<Eigen::Vector3f, Eigen::Vector3f, unsigned int> {
  float min_depth = std::numeric_limits<float>::max();
  std::optional<std::tuple<Eigen::Vector3f, Eigen::Vector3f, float>> hit = std::nullopt;
  int i_object = -1;
  for( int i_sphere = 0; i_sphere < 4; ++i_sphere ) {
    auto hit0 = intersection_ray_sphere(
        ray_src, ray_dir,
        spheres[i_sphere].pos, spheres[i_sphere].rad);
    if (hit0 && std::get<2>(hit0.value()) < min_depth) {
      hit = hit0;
      min_depth = std::get<2>(hit0.value());
      i_object = i_sphere;
    }
  }
  if (!hit) { return {Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero(), -1}; }
  return {std::get<0>(hit.value()), std::get<1>(hit.value()), i_object};
}

/**
 * Light sampling
 * @param nrm normal (not used for light sampling)
 * @param pos position
 * @param dir_out outgoing light (not used for light sampling)
 * @param i_object index of sphere
 * @param rndeng random number generator
 * @return sampled direction
 */
auto sampling_light(
    const Eigen::Vector3f &nrm,
    const Eigen::Vector3f &pos,
    const Eigen::Vector3f &dir_out,
    unsigned int i_object,
    std::mt19937& rndeng) -> Eigen::Vector3f {
  if (i_object == 0) { return {1., 0., 0.,}; }
  auto udist01 = std::uniform_real_distribution<float>(0.f,1.f);
  const Eigen::Vector2f unirand(udist01(rndeng), udist01(rndeng));
  auto light_center = spheres[0].pos;
  float light_rad = spheres[0].rad;
  float sin_theta_max_squared = light_rad * light_rad / (light_center - pos).squaredNorm();
  assert(sin_theta_max_squared > 0.f && sin_theta_max_squared < 1.f);
  float cos_theta_max = std::sqrt(std::max(0.f, 1.f - sin_theta_max_squared));
  float cos_theta = 1.f - unirand.x() * (1.f - cos_theta_max);
  assert(cos_theta > 0.0);
  assert(cos_theta > cos_theta_max);
  assert(cos_theta <= 1.f);
  float sin_theta = std::sqrt(std::max(0.f, 1.f - cos_theta * cos_theta));
  float phi = 2.f * float(M_PI) * unirand.y();
  const auto dir_loc = Eigen::Vector3f(
      sin_theta * std::cos(phi),
      sin_theta * std::sin(phi),
      cos_theta);
  const Eigen::Matrix3f loc2world = local_to_world_vector_transformation((light_center - pos).normalized());
  return loc2world * dir_loc;
}

/**
 * PDF of the light sampling
 * @param nrm
 * @param pos
 * @param dir_in
 * @param dir_out
 * @param hit0_object
 * @return probability density function
 */
float pdf_light_sample(
    const Eigen::Vector3f &nrm,
    const Eigen::Vector3f &pos,
    const Eigen::Vector3f &dir_in,
    const Eigen::Vector3f &dir_out,
    unsigned int hit0_object) {
  if (hit0_object == 0) { return 1.0; }
  auto light_center = spheres[0].pos;
  float light_rad = spheres[0].rad;
  float sin_theta_max_squared = light_rad * light_rad / (light_center - pos).squaredNorm();
  assert(sin_theta_max_squared > 0.f && sin_theta_max_squared < 1.f);
  float cos_theta_max = std::sqrt(std::max(0.f, 1.f - sin_theta_max_squared));
  float pdf = 1.f / (2.f * float(M_PI) * (1.f - cos_theta_max));
  return pdf;
}

auto get_ray_from_camera(
    unsigned int width, unsigned int height,
    unsigned int iw, unsigned int ih) -> std::pair<Eigen::Vector3f, Eigen::Vector3f> {
  auto cam_ray_src = Eigen::Vector3f(0., 0., 2.0); // focus point
  float ndc_x = ((float(iw) + 0.5f) * 2.f / float(width) - 1.f); // normalized device x-coordinate [-1, +1]
  float ndc_y = (1.f - (float(ih) + 0.5f) * 2.f / float(height)); // normalized device y-coordinate [-1, +1]
  float sensor_size = 0.5;
  Eigen::Vector3f position_on_sensor(ndc_x * sensor_size, ndc_y * sensor_size, 1.0);
  Eigen::Vector3f cam_ray_dir = (position_on_sensor - cam_ray_src).normalized();
  return {cam_ray_src, cam_ray_dir};
}

void output_float_image(
    const char* fname,
    unsigned int img_width,
    unsigned int img_height,
    const std::vector<float>& img_data) {
  std::vector<unsigned char> img_u8(img_height * img_width, 0);
  for(int i=0;i<img_width*img_height;++i){
    float data = img_data[img_height+i];
    auto c = static_cast<unsigned char>(std::pow(data,1./2.2)*255.0); // gamma correction
    img_u8[i] = c;
  }
  stbi_write_png(
      fname,
      img_width, img_height, 1, img_u8.data(), img_width);
}

int main() {
  std::mt19937 rndeng(std::random_device{}());
  const unsigned int img_width = 300;
  const unsigned int img_height = 300;
  //
  std::vector<float> img_brdf(img_height * img_width, 0.0);
  std::vector<float> img_light(img_height * img_width, 0.0);
  std::vector<float> img_mis(img_height * img_width, 0.0);
  //
  for (unsigned int iw = 0; iw < img_width; ++iw) {
    for (unsigned int ih = 0; ih < img_height; ++ih) {
      const auto[cam_ray_src, cam_ray_dir] = get_ray_from_camera(img_width, img_height, iw, ih);
      //
      const auto[hit0_pos, hit0_normal, hit0_object]  = hit_scene(cam_ray_src, cam_ray_dir);
      if (hit0_object == -1) {continue;} // does not hit anything
      const int nsample = 100;
      // -----------------
      // light sampling
      img_light[(ih * img_width + iw)] += spheres[hit0_object].emission;
      for (int isample = 0; isample < nsample; ++isample) {
        // sampling light
        auto hit0_refl = sampling_light(hit0_normal, hit0_pos, cam_ray_dir, hit0_object, rndeng);
        // BRDF for sampled light direction
        float hit0_brdf = spheres[hit0_object].brdf(cam_ray_dir, hit0_refl, hit0_normal);
        if (hit0_brdf <= 0.f) { continue; }
        // PDF for sampled light direction
        float hit0_pdf = pdf_light_sample(hit0_normal, hit0_pos, cam_ray_dir, hit0_refl, hit0_object);
        // How much light sampled light direction has
        const auto[hit1_pos, hit1_normal, hit1_object]  = hit_scene(hit0_pos + hit0_normal * 0.01, hit0_refl);
        if (hit1_object == -1){ continue; }
        float hit1_rad = spheres[hit1_object].emission;
        // compute the contribution for this pixel
        float rad = 0.f; // replace this with some code
        img_light[ih * img_width + iw] += rad;
      }
      // -----------------
      // BRDF sampling
      img_brdf[(ih * img_width + iw)] += spheres[hit0_object].emission;
      for (int isample = 0; isample < nsample; ++isample) {
        // direction of reflected ray
        auto hit0_refl = spheres[hit0_object].sample_reflection_based_on_brdf(hit0_normal, cam_ray_dir, rndeng);
        // Brdf value for reflected ray
        float hit0_brdf = spheres[hit0_object].brdf(cam_ray_dir, hit0_refl, hit0_normal);
        if (hit0_brdf <= 0.f) { continue; }
        // PDF of the reflected ray
        const float hit0_pdf = spheres[hit0_object].pdf(hit0_normal, cam_ray_dir, hit0_refl);
        // how much light this reflected ray has
        const auto[hit1_pos, hit1_normal, hit1_object]  = hit_scene(hit0_pos + hit0_normal * 0.01, hit0_refl);
        if (hit1_object == -1){ continue; }
        float hit1_rad = spheres[hit1_object].emission;
        // compute the contribution for this pixel
        float rad = 0.f; // replace this with some code
        img_brdf[ih * img_width + iw] += rad;
      }
      // -----------------
      // Multiple importance sampling
      img_mis[(ih * img_width + iw)] += spheres[hit0_object].emission;
      int num_half_sample = nsample / 2;
      for (int isample = 0; isample < num_half_sample; ++isample) {
        // reflected ray direction
        auto hit0_refl = spheres[hit0_object].sample_reflection_based_on_brdf(hit0_normal, cam_ray_dir, rndeng);
        // Brdf of the reflected ray
        float hit0_brdf = spheres[hit0_object].brdf(cam_ray_dir, hit0_refl, hit0_normal);
        if (hit0_brdf <= 0.f) { continue; }
        const auto[hit1_pos, hit1_normal, hit1_object]  = hit_scene(hit0_pos + hit0_normal * 0.01, hit0_refl);
        if (hit1_object == -1){ continue; }
        float hit1_rad = spheres[hit1_object].emission;
        float hit0_pdf_brdf_sample = spheres[hit0_object].pdf(hit0_normal, cam_ray_dir, hit0_refl);
        float hit0_pdf_light_sample = pdf_light_sample(hit0_normal, hit0_pos, cam_ray_dir, hit0_refl, hit0_object);
        float rad = 0.f; // write some code
        img_mis[ih * img_width + iw] += rad;
      }
      for (int isample = 0; isample < nsample / 2; ++isample) {
        auto hit0_refl = sampling_light(hit0_normal, hit0_pos, cam_ray_dir, hit0_object, rndeng);
        float hit0_brdf = spheres[hit0_object].brdf(cam_ray_dir, hit0_refl, hit0_normal);
        if (hit0_brdf <= 0.f) { continue; }
        const auto[hit1_pos, hit1_normal, hit1_object]  = hit_scene(hit0_pos + hit0_normal * 0.01, hit0_refl);
        if (hit1_object == -1){ continue; }
        float hit1_rad = spheres[hit1_object].emission;
        float hit0_pdf_light_sample = pdf_light_sample(hit0_normal, hit0_pos, cam_ray_dir, hit0_refl, hit0_object);
        float hit0_pdf_brdf_sample = spheres[hit0_object].pdf(hit0_normal, cam_ray_dir, hit0_refl);
        float rad = 0.f; // write some code
        img_mis[ih * img_width + iw] += rad;
      }
    }
  }

  output_float_image(
      (std::filesystem::path(PROJECT_SOURCE_DIR) / "out_brdf.png").string().c_str(),
      img_width, img_height, img_brdf);
  output_float_image(
      (std::filesystem::path(PROJECT_SOURCE_DIR) / "out_light.png").string().c_str(),
      img_width, img_height, img_light);
  output_float_image(
      (std::filesystem::path(PROJECT_SOURCE_DIR) / "out_mis.png").string().c_str(),
      img_width, img_height, img_mis);


}
