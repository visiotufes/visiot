#ifndef __IMAGE_HPP__
#define __IMAGE_HPP__

#include <vector>
#include <msgpack.hpp>
#include "opencv2/core.hpp"

struct Image {
  int rows;
  int cols;
  int type;
  std::vector<unsigned char> frame;

  MSGPACK_DEFINE(rows, cols, type, frame);
};

Image to_image(cv::Mat& mat) {
  Image image {
    mat.rows,
    mat.cols,
    mat.type(),
    std::vector<unsigned char>(mat.datastart, mat.dataend)
  };
  return image;
}

cv::Mat to_mat(Image& image) {
  int rows = image.rows;
  int cols = image.cols;
  int type = image.type;
  unsigned char* data = image.frame.data();
  int size = image.frame.size();

  cv::Mat mat(rows, cols, type, data, size/rows);
  return mat.clone();  
}

#endif // __IMAGE_HPP__