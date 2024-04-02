//
// Created by Nobuyuki Umetani on 2024/03/12.
//

#ifndef READ_OBJ_FILE_H_
#define READ_OBJ_FILE_H_

#include <vector>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <tuple>
//
#include "Eigen/Dense"

namespace acg {

auto read_wavefrontobj_as_3d_triangle_mesh(
    const std::filesystem::path &file_path)
-> std::tuple<Eigen::Matrix<unsigned int, 3, Eigen::Dynamic>, Eigen::Matrix3Xf> {
  using myMatrix3Xui = Eigen::Matrix<unsigned int, 3, Eigen::Dynamic>;
  std::ifstream fin;
  fin.open(file_path);
  if (fin.fail()) {
    std::cout << "File Read Fail" << std::endl;
    Eigen::Matrix<unsigned int, 3, Eigen::Dynamic> b;
    Eigen::Matrix3Xf a;
    return std::make_tuple(b, a);
  }
  std::vector<float> vtx2xyz;
  std::vector<unsigned int> tri2vtx;
  constexpr unsigned int BUFF_SIZE = 256;
  char buff[BUFF_SIZE];
  while (fin.getline(buff, BUFF_SIZE)) {
    if (buff[0] == '#') { continue; }
    if (buff[0] == 'v' && buff[1] == ' ') {
      char str[256];
      float x, y, z;
      std::istringstream is(buff);
      is >> str >> x >> y >> z;
      vtx2xyz.push_back(x);
      vtx2xyz.push_back(y);
      vtx2xyz.push_back(z);
    }
    if (buff[0] == 'f') {
      std::vector<std::string> vec_str;
      {
        std::istringstream iss(buff);
        std::string s;
        bool is_init = true;
        while (iss >> s) {
          if (is_init) {
            is_init = false;
            continue;
          }
          vec_str.push_back(s);
        }
      }
      std::vector<int> aI;
      aI.reserve(4);
      for (auto str: vec_str) {
        for (char &i: str) {
          if (i == '/') {
            i = '\0';
            break;
          }
        }
        int i0 = std::stoi(str);
        aI.push_back(i0 - 1);
      }
      if (aI.size() == 3) {
        tri2vtx.push_back(aI[0]);
        tri2vtx.push_back(aI[1]);
        tri2vtx.push_back(aI[2]);
      }
      if (aI.size() == 4) {
        tri2vtx.push_back(aI[0]);
        tri2vtx.push_back(aI[1]);
        tri2vtx.push_back(aI[2]);
        //
        tri2vtx.push_back(aI[0]);
        tri2vtx.push_back(aI[2]);
        tri2vtx.push_back(aI[3]);
      }
    }
  }
  Eigen::Matrix3Xf a = Eigen::Map<Eigen::Matrix3Xf, Eigen::Unaligned>(vtx2xyz.data(), 3, vtx2xyz.size() / 3);
  myMatrix3Xui b = Eigen::Map<myMatrix3Xui, Eigen::Unaligned>(tri2vtx.data(), 3, tri2vtx.size() / 3);
  return std::make_tuple(b, a);
}

auto vertex_normals_of_triangle_mesh(
    const Eigen::Matrix<unsigned int, 3, Eigen::Dynamic> &tri2vtx,
    const Eigen::Matrix3Xf &vtx2xyz) -> Eigen::Matrix3Xf {
  const auto num_vtx = vtx2xyz.size() / 3;
  auto vtx2normal = std::vector<float>(num_vtx * 3, 0.f);
  for (unsigned int i_tri = 0; i_tri < tri2vtx.cols(); ++i_tri) {
    const unsigned int i0 = tri2vtx(0, i_tri);
    const unsigned int i1 = tri2vtx(1, i_tri);
    const unsigned int i2 = tri2vtx(2, i_tri);
    auto p0 = vtx2xyz.col(i0);
    auto p1 = vtx2xyz.col(i1);
    auto p2 = vtx2xyz.col(i2);
    Eigen::Vector3f un = (p1 - p0).cross(p2 - p0).normalized();
    for (int i_dim = 0; i_dim < 3; ++i_dim) {
      vtx2normal[i0 * 3 + i_dim] += un[i_dim];
      vtx2normal[i1 * 3 + i_dim] += un[i_dim];
      vtx2normal[i2 * 3 + i_dim] += un[i_dim];
    }
  }
  for (int i_vtx=0;i_vtx<num_vtx;++i_vtx){
    auto n = Eigen::Map<Eigen::Vector3f, Eigen::Unaligned>(vtx2normal.data() + i_vtx*3);
    n.normalize();
  }
  return Eigen::Map<Eigen::Matrix3Xf, Eigen::Unaligned>(vtx2normal.data(), 3, num_vtx);
}

} // namespace acg

#endif //READ_OBJ_FILE_H_
