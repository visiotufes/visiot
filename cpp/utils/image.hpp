#ifndef __IMAGE_HPP__
#define __IMAGE_HPP__

#include <vector>
#include <msgpack.hpp>
#include "opencv2/core.hpp"

namespace is {

struct Image {

  size_t size;
  size_t rows;
  size_t cols;
  size_t type;
  std::vector<unsigned char> frame;
  int64_t timestamp;

  Image() {}

  Image(const unsigned char* data, 
        const size_t& size,
        const size_t& rows, 
        const size_t& cols, 
        const size_t& type,
        const int64_t& timestamp) {

    this->size = size;
    this->rows = rows;
    this->cols = cols;
    this->type = type;
    this->frame.reserve(size);
    std::copy(data, data+size, std::back_inserter(this->frame));
    this->timestamp = timestamp;
  }

  Image(const unsigned char* data, 
        const size_t& size,
        const size_t& rows, 
        const size_t& cols, 
        const size_t& type) {

    this->size = size;
    this->rows = rows;
    this->cols = cols;
    this->type = type;
    this->frame.reserve(size);
    std::copy(data, data+size, std::back_inserter(this->frame));
  }

  MSGPACK_DEFINE(size, rows, cols, type, frame, timestamp);
};
/*
struct Image {
  int rows;
  int cols;
  int type;
  std::vector<unsigned char> frame;

  MSGPACK_DEFINE(rows, cols, type, frame);
};
*/

Image to_image(cv::Mat& mat);
cv::Mat to_mat(Image& image);

} // ::is

#endif // __IMAGE_HPP__