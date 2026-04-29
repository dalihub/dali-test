/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include "image-util.h"
#include "visual-test.h"

#include <iostream>
#include <string>
#include <cstring>

// Suppress OpenCV warnings
#include <opencv2/core/utils/logger.hpp>

void PrintUsage(const char* programName)
{
  std::cout << "Usage: " << programName << " <image1_path> <image2_path> [--threshold <value>]" << std::endl;
  std::cout << "       " << programName << " [--threshold <value>] <image1_path> <image2_path>" << std::endl;
  std::cout << "       " << programName << " -h | --help" << std::endl;
  std::cout << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  --threshold <value>  Similarity threshold (default: " << DEFAULT_IMAGE_SIMILARITY_THRESHOLD << ")" << std::endl;
  std::cout << "  -h, --help           Show this help message" << std::endl;
  std::cout << std::endl;
  std::cout << "Description:" << std::endl;
  std::cout << "  Compares two images using SSIM (Structural Similarity Index) and returns" << std::endl;
  std::cout << "  0 if they are similar enough (above threshold), 1 otherwise." << std::endl;
  std::cout << "  The --threshold flag can appear anywhere in the argument list." << std::endl;
}

int main(int argc, char** argv)
{
  // Suppress OpenCV warnings
  cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);

  // Parse command-line arguments
  // Usage: <program> <image1_path> <image2_path> [--threshold <value>]
  // The --threshold flag can appear anywhere in the argument list

  std::string image1Path;
  std::string image2Path;
  float       threshold = DEFAULT_IMAGE_SIMILARITY_THRESHOLD;
  bool        extraArg  = false;

  // First pass: find --threshold value and collect image paths
  for(int i = 1; i < argc; ++i)
  {
    if(std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0)
    {
      PrintUsage(argv[0]);
      return 0;
    }
    else if(std::strcmp(argv[i], "--threshold") == 0 && i + 1 < argc)
    {
      threshold = std::stof(argv[i + 1]);
      i++; // Skip the threshold value
    }
    else if(image1Path.empty())
    {
      image1Path = argv[i];
    }
    else if(image2Path.empty())
    {
      image2Path = argv[i];
    }
    else
    {
      // Extra argument found after we already have both image paths
      extraArg = true;
    }
  }

  // Validate we have both image paths and no extra arguments
  if(image1Path.empty() || image2Path.empty() || extraArg)
  {
    PrintUsage(argv[0]);
    return 1;
  }

  cv::Scalar similarity;

  // Load the images
  cv::Mat matrixImg1 = cv::imread(image1Path);
  cv::Mat matrixImg2 = cv::imread(image2Path);

  // Check if images were loaded successfully
  if(matrixImg1.empty() || matrixImg2.empty())
  {
    std::cerr << "Error: Failed to load one or both images" << std::endl;
    return 1;
  }

  // Calculate SSIM for the full image (ignoring areaToCompare)
  similarity = ImageUtil::CalculateSSIM(matrixImg1, matrixImg2);

  // Check whether SSIM for all the three channels (RGB) are above the threshold
  bool passed = (similarity.val[0] >= threshold && similarity.val[1] >= threshold && similarity.val[2] >= threshold);

  printf(
    "Test similarity: R:%f G:%f B:%f\n"
    "Passed threshold of %f: %s\n",
    100.0f * similarity.val[0],
    100.0f * similarity.val[1],
    100.0f * similarity.val[2],
    100.0f * threshold,
    passed ? "TRUE" : "FALSE");

  // Return 0 if passed, 1 if failed (ignoring passedCount and totalCount)
  return passed ? 0 : 1;
}
