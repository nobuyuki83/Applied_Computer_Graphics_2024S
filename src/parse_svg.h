//
// Created by Nobuyuki Umetani on 2024/03/06.
//

#ifndef PARSE_SVG_H_
#define PARSE_SVG_H_

#include <utility>
#include <vector>
#include <string>
#include <sstream>
#include <optional>
#include <map>
#include <iostream>
//
#include "Eigen/Dense"

namespace acg {

std::vector<std::string> split(const std::string &str, const char separator) {
  std::vector<std::string> out;
  std::stringstream ss(str);
  std::string buf;
  while (std::getline(ss, buf, separator)) {
    out.push_back(buf);
  }
  return out;
}

std::optional<std::vector<char>> get_file_content(
    const std::string &fpath) {
  size_t size;
  FILE *fp = fopen(fpath.c_str(), "rb");
  if (!fp) {
    return std::nullopt;
  }
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  std::vector<char> aC(size + 1);
  if (fread(aC.data(), 1, size, fp) != size) {
    fclose(fp);
    return std::nullopt;
  }
  aC[size] = '\0';  // Must be null terminated.
  fclose(fp);
  return std::make_optional(aC);
}

std::string remove_beginning(
    const std::string &str,
    const std::string &del) {
  const size_t n = str.size();
  const size_t ndel = del.size();
  //
  unsigned int istat = 0;
  for (unsigned int i = 0; i < n; ++i) {
    bool is_del = false;
    for (unsigned int idel = 0; idel < ndel; ++idel) {
      if (str[i] != del[idel]) { continue; }
      is_del = true;
      break;
    }
    if (is_del) continue;
    istat = i;
    break;
  }
  return {str.begin() + istat, str.end()};
}

std::vector<std::string> separate_tags(
    const std::vector<char> &input) {
  std::vector<std::string> aStr;
  std::vector<char> buffer = input;
  char *s = buffer.data();
  char *mark = s;
  int state = 1;
  while (*s) {
    if (*s == '<' && state == 1) {
      // Start of a tag
      *s++ = '\0';
      aStr.emplace_back(mark);
      mark = s;
      state = 0;
    } else if (*s == '>' && state == 0) {       // Start of a content or new tag.
      *s++ = '\0';
      aStr.emplace_back(mark);
      mark = s;
      state = 1;
    } else {
      s++;
    }
  }
  for (auto &is: aStr) {
    is = remove_beginning(is, " ");
  }
  return aStr;
}

std::vector<std::string> split_quote(
    const std::string &str,
    char delimiter,
    char quote) {
  std::vector<std::string> aToken;
  unsigned int is = 0;
  bool is_in = false;
  for (unsigned int ie = 0; ie < str.size(); ++ie) {
    if (ie == str.size() - 1) {
      aToken.emplace_back(str.data() + is, str.data() + ie + 1);
    }
    if (str[ie] == quote) { is_in = !is_in; }
    if (str[ie] == delimiter && !is_in) {
      if (str[is] != delimiter) { // skip the sequence of the delimiter
        aToken.emplace_back(str.data() + is, str.data() + ie);
      }
      is = ie + 1;
    }
  }
  return aToken;
}

std::string remove_quote(
    const std::string &str,
    char quat) {
  const size_t n = str.size();
  {
    int nq = 0;
    for (unsigned int i = 0; i < n; ++i) {
      if (str[i] == quat) { ++nq; }
    }
    if (nq < 2) { return str; }
  }
  unsigned int istat = 0;
  for (; istat < n; ++istat) {
    if (str[istat] == quat) { break; }
  }
  int iend = (int) n - 1;
  for (; iend >= 0; --iend) {
    if (str[iend] == quat) { break; }
  }
  return {str.begin() + istat + 1, str.begin() + iend};
}

std::map<std::string, std::string> parse_tag_contents(
    const std::string &input) {
  const std::vector<std::string> aS = split_quote(input, ' ', '\"');
  std::map<std::string, std::string> mapAttr;
  for (const auto &is: aS) {
    const std::vector<std::string> aS1 = split(is, '=');
    if (aS1.size() != 2) continue;
    const std::string s1 = remove_quote(aS1[1], '\"');
    mapAttr.insert(std::make_pair(aS1[0], s1));
  }
  return mapAttr;
}

std::tuple<int, int, std::string> svg_get_image_size_and_shape(const std::filesystem::path &file_path) {
  const std::optional<std::vector<char>> aC = get_file_content(file_path.string());
  if (!aC) {
    return std::make_tuple(0, 0, "");
  }
  unsigned int width = 0, height = 0;
  std::string shape;
  const std::vector<std::string> aStrTagContent = separate_tags(aC.value());
  for (const auto &str: aStrTagContent) {
    if (str.compare(0, 4, "svg ") == 0) {
      const std::map<std::string, std::string> mapAttr = parse_tag_contents(str);
      height = std::stoi(mapAttr.at("height"));
      width = std::stoi(mapAttr.at("width"));
    }
    if (str.compare(0, 5, "path ") == 0) {
      const std::map<std::string, std::string> mapAttr = parse_tag_contents(str);
      shape = mapAttr.at("d");
    }
  }
  return std::make_tuple(width, height, shape);
}

std::vector<std::string> svg_outline_path_from_shape(
    const std::string &s0) {
  unsigned int imark = 0;
  std::vector<std::string> aS;
  for (unsigned int i = 0; i < s0.size(); ++i) {
    if (std::isdigit(s0[i])) continue;
    if (s0[i] == ',') {
      std::string s1(s0.begin() + imark, s0.begin() + i);
      aS.push_back(s1);
      imark = i + 1;  // mark should be the beginning position of the string so move next
    }
    if (s0[i] == ' ') { // sometimes the space act as delimiter in the SVG (inkscape version)
      if (i > imark) {
        std::string s1(s0.begin() + imark, s0.begin() + i);
        aS.push_back(s1);
      }
      imark = i + 1; // mark should be the beginning position of the string so move next
    }
    if (s0[i] == '-') {
      if (i > imark) {
        std::string s1(s0.begin() + imark, s0.begin() + i);
        aS.push_back(s1);
      }
      imark = i;
    }
    if (std::isalpha(s0[i])) {
      if (i > imark) {
        std::string s1(s0.begin() + imark, s0.begin() + i);
        aS.push_back(s1);
      }
      const char s2[2] = {s0[i], '\0'};
      aS.emplace_back(s2);
      imark = i + 1;
    }
  }
  return aS;
}

class Edge {
 public:
  Eigen::Vector2f ps;
  Eigen::Vector2f pe;
  Eigen::Vector2f pc;
  bool is_bezier;
 public:
  Edge(
      Eigen::Vector2f _ps,
      Eigen::Vector2f _pe) : ps(std::move(_ps)), pe(std::move(_pe)), is_bezier(false) {}
  Edge(
      Eigen::Vector2f _ps,
      Eigen::Vector2f _pc,
      Eigen::Vector2f _pe) : ps(std::move(_ps)), pc(std::move(_pc)), pe(std::move(_pe)), is_bezier(true) {}
};

std::vector<std::vector<Edge>> svg_loops_from_outline_path(
    const std::vector<std::string> &strs) {
  std::vector< std::vector<Edge>> loops;
  std::vector<Edge> edges_buffer;
  assert(strs[0] == "M" || strs[0] == "m");
  assert(strs[strs.size() - 1] == "Z" || strs[strs.size() - 1] == "z");
  Eigen::Vector2f pos_cur(0., 0.);
  for (int is = 0;;) {
    // std::cout << is << " " << strs[is] << std::endl;
    if (strs[is] == "M") { // start absolute
      is++;
      pos_cur = Eigen::Vector2f(std::stof(strs[is + 0]), std::stof(strs[is + 1]));
      for (; is += 2;) {
        if (isalpha(strs[is][0])) { break; }
        auto p1 = Eigen::Vector2f(std::stof(strs[is + 0]), std::stof(strs[is + 1]));
        edges_buffer.emplace_back(pos_cur, p1);
        pos_cur = p1;
      }
    } else if (strs[is] == "m") { // start relative
      is++;
      pos_cur = pos_cur + Eigen::Vector2f(std::stof(strs[is + 0]), std::stof(strs[is + 1]));
      for (; is += 2;) {
        if (isalpha(strs[is][0])) { break; }
        Eigen::Vector2f p1 = pos_cur + Eigen::Vector2f(std::stof(strs[is + 0]), std::stof(strs[is + 1]));
        edges_buffer.emplace_back(pos_cur, p1);
        pos_cur = p1;
      }
    } else if (strs[is] == "l") { // line relative
      ++is;
      for (;;) {
        Eigen::Vector2f p1 = pos_cur + Eigen::Vector2f(std::stof(strs[is + 0]), std::stof(strs[is + 1]));
        edges_buffer.emplace_back(pos_cur, p1);
        pos_cur = p1;
        is += 2;
        if (isalpha(strs[is][0])) { break; }
      }
    } else if (strs[is] == "L") { // line absolute
      ++is;
      for (;;) {
        Eigen::Vector2f p1 = Eigen::Vector2f(std::stof(strs[is + 0]), std::stof(strs[is + 1]));
        edges_buffer.emplace_back(pos_cur, p1);
        pos_cur = p1;
        is += 2;
        if (isalpha(strs[is][0])) { break; }
      }
    } else if (strs[is] == "v") { // vertical relative
      Eigen::Vector2f p1 = pos_cur + Eigen::Vector2f(0., std::stof(strs[is + 1]));
      edges_buffer.emplace_back(pos_cur, p1);
      pos_cur = p1;
      is += 2;
    } else if (strs[is] == "V") { // vertical absolute
      Eigen::Vector2f p1 = Eigen::Vector2f(pos_cur[0], std::stof(strs[is + 1]));
      edges_buffer.emplace_back(pos_cur, p1);
      pos_cur = p1;
      is += 2;
    } else if (strs[is] == "H") { // horizontal absolute
      Eigen::Vector2f p1 = Eigen::Vector2f(std::stof(strs[is + 1]), pos_cur[1]);
      edges_buffer.emplace_back(pos_cur, p1);
      pos_cur = p1;
      is += 2;
    } else if (strs[is] == "h") { // horizontal relative
      float dh = std::stof(strs[is + 1]);
      Eigen::Vector2f p1 = pos_cur + Eigen::Vector2f(dh, 0.);
      // std::cout << "  " << dh << " " << pos_cur.x() << " " << pos_cur.y() << "   " << p1.x() << " " << p1.y() << std::endl;
      edges_buffer.emplace_back(pos_cur, p1);
      pos_cur = p1;
      is += 2;
    } else if (strs[is] == "q") {  // relative
      is++;
      for (;;) { // loop for poly-Bezeir curve
        Eigen::Vector2f pm0 = pos_cur + Eigen::Vector2f(std::stof(strs[is + 0]), std::stof(strs[is + 1]));
        Eigen::Vector2f p1 = pos_cur + Eigen::Vector2f(std::stof(strs[is + 2]), std::stof(strs[is + 3]));
        edges_buffer.emplace_back(pos_cur, pm0, p1);
        pos_cur = p1;
        is += 4;
        if (isalpha(strs[is][0])) { break; }
      }
    } else if (strs[is] == "Q") { // absolute
      is++;
      for (;;) { // loop for poly-Bezeir curve
        const Eigen::Vector2f pm0 = Eigen::Vector2f(std::stof(strs[is + 0]), std::stof(strs[is + 1]));
        const Eigen::Vector2f p1 = Eigen::Vector2f(std::stof(strs[is + 2]), std::stof(strs[is + 3]));
        edges_buffer.emplace_back(pos_cur, pm0, p1);
        pos_cur = p1;
        is += 4;
        if (isalpha(strs[is][0])) { break; }
      }
    } else if (strs[is] == "z" || strs[is] == "Z") {
      const Eigen::Vector2f pe = edges_buffer[0].ps;
      const Eigen::Vector2f ps = edges_buffer[edges_buffer.size() - 1].pe;
      double dist0 = (ps - pe).norm();
      if (dist0 > 1.0e-9) {
        edges_buffer.emplace_back(ps, pe);
      }
      loops.push_back(edges_buffer);
      edges_buffer.clear();
      // std::cout << is << " " << strs.size() << std::endl;
      if (is == strs.size()-1 ) { break; }
      ++is;
    } else {
      std::cout << "error!--> " << strs[is] << std::endl;
      break;
    }
  }
  return loops;
}

} // acg

#endif //PARSE_SVG_H_
