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
#include <dali/dali.h>
#include <dali/devel-api/adaptor-framework/image-loading.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/events/point.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <cstdlib>
#include <string>
#include <functional>

extern char* gTempFilename;
extern char* gTempDir;
extern bool  gFB;
extern int   gExitValue;

bool ParseEnvironment(int argc, char** argv, int width, int height);

/**
 * DALI_VISUAL_TEST_WITH_WINDOW_SIZE is a wrapper for the boilerplate code to create
 * the main function of the visual test application with the given main window size.
 * @param[in] VisualTestName The class name of the visual test
 * @param[in] InitFunction The name of the callback function to connect with the application's InitSignal
 * @param[in] WindowWidth The width of the application's main window
 * @param[in] WindowHeight The height of the application's main window
 * @note This sets the DPI to be 96 for all tests so that text tests all produce the same output image
 */
#define DALI_VISUAL_TEST_WITH_WINDOW_SIZE_AND_PREPROESS(VisualTestName, InitFunction, WindowWidth, WindowHeight, Preprocess)                  \
  int DALI_EXPORT_API main(int argc, char** argv)                                                                                             \
  {                                                                                                                                           \
    int n=asprintf(&gTempDir, "/tmp/dali-tests");                                                                                             \
    if(n>0)                                                                                                                                   \
    {                                                                                                                                         \
      bool cont = ParseEnvironment(argc, argv, WindowWidth, WindowHeight);                                                                    \
      if(!cont) return 0;                                                                                                                     \
      n=asprintf(&gTempFilename, "%s/%s", gTempDir, #VisualTestName);                                                                         \
      setenv("DALI_DPI_HORIZONTAL", "96", true);                                                                                              \
      setenv("DALI_DPI_VERTICAL", "96", true);                                                                                                \
      Preprocess();                                                                                                                           \
      Application    application = Application::New(&argc, &argv, "", Application::OPAQUE, Dali::Rect<int>(0, 0, WindowWidth, WindowHeight)); \
      VisualTestName test(application);                                                                                                       \
      application.InitSignal().Connect(&test, &VisualTestName::InitFunction);                                                                 \
      application.MainLoop();                                                                                                                 \
      return gExitValue;                                                                                                                      \
    }                                                                                                                                         \
  }

#define DALI_VISUAL_TEST_WITH_WINDOW_SIZE(VisualTestName, InitFunction, WindowWidth, WindowHeight) DALI_VISUAL_TEST_WITH_WINDOW_SIZE_AND_PREPROESS(VisualTestName, InitFunction, WindowWidth, WindowHeight, std::function<void()>([](){}))

/**
 * DALI_VISUAL_TEST is a wrapper for the boilerplate code to create the main function
 * of the visual test application with the default main window size (i.e. 480 x 800).
 * @param[in] VisualTestName The class name of the visual test
 * @param[in] InitFunction The name of the callback function to connect with the application's InitSignal
 */
#define DALI_VISUAL_TEST(VisualTestName, InitFunction) DALI_VISUAL_TEST_WITH_WINDOW_SIZE(VisualTestName, InitFunction, 480, 800)

// The default threshold for image similarity
#define DEFAULT_IMAGE_SIMILARITY_THRESHOLD 0.99f

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
   * @param[in] customCamera The custom camera to be used to render the offscreen frame buffer (or otherwise a default camera will be created and used )
   */
  void CaptureWindow(Dali::Window window, Dali::CameraActor customCamera = Dali::CameraActor());

  /**
   * @brief Capture the content of the given window rendered by GPU after 1 frame rendered
   * @param[in] window The window to be captured
   * @param[in] customCamera The custom camera to be used to render the offscreen frame buffer (or otherwise a default camera will be created and used )
   */
  void CaptureWindowAfterFrameRendered(Dali::Window window, Dali::CameraActor customCamera = Dali::CameraActor());

  /**
   * @brief Compare the given area in the two image files.
   * @param[in] fileName1 The first image file
   * @param[in] fileName2 The second image file
   * @param[in] similarityThreshold The threshold for similarity comparison
   * @param[in] areaToCompare The area to be compared
   * @return Whether the similarity of the given area in the two images reaches the given threshold
   */
  bool CompareImageFile(const std::string fileName1, const std::string fileName2, const float similarityThreshold, const Dali::Rect<uint16_t>& areaToCompare=Dali::Rect<uint16_t>(0u, 0u, 0u, 0u));

  /**
   * @brief Emits a single touch
   *
   * touchPoint is mutable.
   * touchPoint state of TouchPoint is irrelevant upon input, EmitTouch() sets it internally
   *
   * @param[in] touchPoint Valid TouchPoint structure
   */
  void EmitTouch(Dali::TouchPoint& touchPoint);

private:
  /**
   * @brief This virtual function will be called after the offscreen window frame buffer has been rendered.
   *
   * @param[in] outputFile The output file that the offscreen has been rendered to.
   * @param[in] writeSuccess True if the file was written successfully.
   *
   * @note  The visual test case must implement this function to check the result of the offscreen frame buffer.
   */
  virtual void PostRender(std::string outputFile, bool writeSuccess) = 0;

  /**
   * @brief Set up the offscreen render task for offscreen rendering.
   * @param[in] window The window to be rendered
   * @param[in] customCamera The custom camera to be used to render the offscreen frame buffer
   */
  void SetupOffscreenRenderTask(Dali::Window window, Dali::CameraActor customCamera);

  /**
   * @brief Callback function when a RenderTask has finished
   * @param[in] task The render task
   */
  void OnOffscreenRenderFinished(Dali::RenderTask& task);

  /**
   * @brief Callback function when a window has been resized
   * @param[in] window The window
   * @param[in] size The new size of the window
   */
  void OnWindowResized(Dali::Window window, Dali::Window::WindowSize size);

  void OnAnimationFinished1(Dali::Animation& /* not used */);
  void OnAnimationFinished2(Dali::Animation& /* not used */);
  void OnAnimationFinished3(Dali::Animation& /* not used */);
  void OnAnimationFinished4(Dali::Animation& /* not used */);

private:
  Dali::Texture     mTexture;     ///< The texture for the offscreen rendering
  Dali::FrameBuffer mFrameBuffer; ///< The frame buffer for offscreen rendering

  Dali::RenderTask              mOffscreenRenderTask; ///< The offscreen render task
  Dali::CameraActor             mCameraActor;         ///< The camera actor for the offscreen render task
  Dali::WeakHandle<Dali::Layer> mWindow;              ///< The weak handle of the window to be rendered

  Dali::Window      mCaptureRequestedWindow;
  Dali::CameraActor mCaptureRequestedCamera;
};

#endif // VISUAL_TEST_H
