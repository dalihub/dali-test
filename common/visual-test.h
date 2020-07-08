#ifndef VISUAL_TEST_H
#define VISUAL_TEST_H

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
#include <string>
#include <cstdlib>
#include <dali/dali.h>
#include <dali/devel-api/adaptor-framework/image-loading.h>

/**
 * DALI_VISUAL_TEST_WITH_WINDOW_SIZE is a wrapper for the boilerplate code to create
 * the main function of the visual test application with the given main window size.
 * @param[in] VisualTestName The class name of the visual test
 * @param[in] InitFunction The name of the callback function to connect with the application's InitSignal
 * @param[in] WindowWidth The width of the application's main window
 * @param[in] WindowHeight The height of the application's main window
 * @note This sets the DPI to be 96 for all tests so that text tests all produce the same output image
 */
#define DALI_VISUAL_TEST_WITH_WINDOW_SIZE( VisualTestName, InitFunction, WindowWidth, WindowHeight ) \
  int DALI_EXPORT_API main( int argc, char **argv ) \
  { \
    setenv( "DALI_DPI_HORIZONTAL", "96", true ); \
    setenv( "DALI_DPI_VERTICAL", "96", true ); \
    Application application = Application::New( &argc, &argv, "", Application::OPAQUE, Dali::Rect<int>(0, 0, WindowWidth, WindowHeight) ); \
    VisualTestName test( application ); \
    application.InitSignal().Connect( &test, &VisualTestName::InitFunction ); \
    application.MainLoop(); \
    return 0; \
  }

/**
 * DALI_VISUAL_TEST is a wrapper for the boilerplate code to create the main function
 * of the visual test application with the default main window size (i.e. 480 x 800).
 * @param[in] VisualTestName The class name of the visual test
 * @param[in] InitFunction The name of the callback function to connect with the application's InitSignal
 */
#define DALI_VISUAL_TEST( VisualTestName, InitFunction ) DALI_VISUAL_TEST_WITH_WINDOW_SIZE( VisualTestName, InitFunction, 480, 800 )

// The default threshold for image similarity
#define DEFAULT_IMAGE_SIMILARITY_THRESHOLD 1.0f

/**
 * @brief This class provides the functionality of visual test by capturing the content
 *        rendered by the GPU in the given window and compare it with a given image.
 */
class VisualTest : public Dali::ConnectionTracker
{
public:

  /**
   * @brief Constructor.
   */
  VisualTest();

protected:

  /**
   * @brief Default Destructor.
   */
  virtual ~VisualTest() = default;

  /**
   * @brief Capture the content of the given window rendered by GPU
   * @param[in] window The window to be captured
   */
  void CaptureWindow( Dali::Window window );

  /**
   * @brief Compare the result of the window capture with the given image file
   * @param[in] fileName The image file to be compared with
   * @param[in] similarityThreshold The threshold for similarity comparison (The default value is 1 which means all the pixels in the two images must be identical)
   * @param[in] areaToCompare The area in the image to be compared (The default value is an empty rectangle which means the whole area of the image will be compared)
   * @return Whether the similarity of the two images reaches the given threshold
   */
  bool CheckImage( const std::string fileName, const float similarityThreshold = DEFAULT_IMAGE_SIMILARITY_THRESHOLD, const Dali::Rect<uint16_t>& areaToCompare = Dali::Rect<uint16_t>(0u, 0u, 0u, 0u) );

private:

  /**
   * @brief This virtual function will be called after the offscreen window frame buffer has been rendered.
   * @note  The visual test case must implement this function to check the result of the offscreen frame buffer.
   */
  virtual void PostRender() = 0;

  /**
   * @brief Set up the native image for offscreen rendering.
   * @param[in] window The window to be rendered
   */
  void SetupNativeImage( Dali::Window window );

  /**
   * @brief Callback function when a RenderTask has finished
   * @param[in] task The render task
   */
  void OnOffscreenRenderFinished( Dali::RenderTask& task );

  /**
   * @brief Compare the given area in the two image files.
   * @param[in] fileName1 The first image file
   * @param[in] fileName2 The second image file
   * @param[in] similarityThreshold The threshold for similarity comparison
   * @param[in] areaToCompare The area to be compared
   * @return Whether the similarity of the given area in the two images reaches the given threshold
   */
  bool CompareImageFile( const std::string fileName1, const std::string fileName2, const float similarityThreshold, const Dali::Rect<uint16_t>& areaToCompare );

private:

  Dali::NativeImageSourcePtr mNativeImageSourcePtr;           ///< The pointer of the native image source
  Dali::Texture              mNativeTexture;                  ///< The texture for the native image
  Dali::FrameBuffer          mFrameBuffer;                    ///< The frame buffer for offscreen rendering

  Dali::RenderTask mOffscreenRenderTask;                      ///< The offscreen render task
  Dali::CameraActor mCameraActor;                             ///< The camera actor for the offscreen render task
  Dali::WeakHandle< Dali::Layer > mWindow;                    ///< The weak handle of the window to be rendered
};

#endif // VISUAL_TEST_H
