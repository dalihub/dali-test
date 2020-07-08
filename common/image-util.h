#ifndef IMAGE_UTIL_H
#define IMAGE_UTIL_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// EXTERNAL INCLUDES
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

namespace ImageUtil
{

#define C1 (float) (0.01f * 255 * 0.01f * 255)
#define C2 (float) (0.03f * 255 * 0.03f * 255)

/**
 * @brief Calculate the structural similarity (SSIM) index for each channel of the two images.
 *        SSIM is used for measuring the similarity between two images, as described in:
 *        https://ece.uwaterloo.ca/~z70wang/publications/ssim.html
 * @param[in] image1 The matrix representation of the first image
 * @param[in] image2 The matrix representation of the second image
 * @return The SSIM for the RGB channels respectively (The value for each channel is between 0 and 1, and the closer to 1 the more similar)
 */
cv::Scalar CalculateSSIM( const cv::Mat& image1, const cv::Mat& image2 )
{
  // Initialization

  int d = CV_32F;

  cv::Mat I1, I2;
  image1.convertTo( I1, d ); // cannot calculate on one byte large values
  image2.convertTo( I2, d );

  cv::Mat I2_2 = I2.mul( I2 );  // I2^2
  cv::Mat I1_2 = I1.mul( I1 );  // I1^2
  cv::Mat I1_I2 = I1.mul( I2 ); // I1 * I2

  // Preliminary computing

  cv::Mat mu1, mu2;   //
  GaussianBlur( I1, mu1, cv::Size( 11, 11 ), 1.5f );
  GaussianBlur( I2, mu2, cv::Size( 11, 11 ), 1.5f );

  cv::Mat mu1_2 = mu1.mul( mu1 );
  cv::Mat mu2_2 = mu2.mul( mu2 );
  cv::Mat mu1_mu2 = mu1.mul( mu2 );

  cv::Mat sigma1_2, sigma2_2, sigma12;

  GaussianBlur( I1_2, sigma1_2, cv::Size( 11, 11 ), 1.5f );
  sigma1_2 -= mu1_2;

  GaussianBlur( I2_2, sigma2_2, cv::Size( 11, 11 ), 1.5f );
  sigma2_2 -= mu2_2;

  GaussianBlur( I1_I2, sigma12, cv::Size( 11, 11 ), 1.5 );
  sigma12 -= mu1_mu2;

  // Formula

  cv::Mat t1, t2, t3;

  t1 = 2 * mu1_mu2 + C1;
  t2 = 2 * sigma12 + C2;
  t3 = t1.mul( t2 );          // t3 = ((2 * mu1_mu2 + C1) * (2 * sigma12 + C2))

  t1 = mu1_2 + mu2_2 + C1;
  t2 = sigma1_2 + sigma2_2 + C2;
  t1 = t1.mul( t2 );          // t1 = ((mu1_2 + mu2_2 + C1) * (sigma1_2 + sigma2_2 + C2))

  cv::Mat ssim_map;
  divide( t3, t1, ssim_map ); // ssim_map =  t3./t1;

  return mean( ssim_map );    // average of ssim map
}

}

#endif // IMAGE_UTIL_H
