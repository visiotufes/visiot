#include "image.hpp"

namespace is {

Image to_image(cv::Mat& mat) {
  Image image(
    mat.ptr(),
    mat.total()*mat.elemSize(),
    mat.rows,
    mat.cols,
    mat.type()
  );
  return image;
}

cv::Mat to_mat(Image& image) {
  int rows = image.rows;
  int cols = image.cols;
  int type = image.type;
  unsigned char* data = image.frame.data();
  int size = image.size;

  cv::Mat mat(rows, cols, type, data, size/rows);
  return mat.clone();  
}

} // ::is