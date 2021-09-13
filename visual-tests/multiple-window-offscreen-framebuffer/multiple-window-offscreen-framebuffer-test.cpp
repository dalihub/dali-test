/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali-toolkit/dali-toolkit.h>
#include <dali-toolkit/devel-api/image-loader/texture-manager.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{
constexpr const Vector2 TARGET_SIZE_1(320.f, 170.f);
constexpr const Vector2 TARGET_SIZE_2(640.f, 720.f);
constexpr const unsigned int NUMBER_OF_IMAGES = 1;

constexpr const char* TEST_IMAGE_FILE_1 = TEST_IMAGE_DIR "multiple-window-offscreen-framebuffer/dali-logo.png";
constexpr const char* TEST_IMAGE_FILE_2 = TEST_IMAGE_DIR "multiple-window-offscreen-framebuffer/book-landscape-cover.jpg";
constexpr const char* RESULT_IMAGE_FILE_1 = TEST_IMAGE_DIR "multiple-window-offscreen-framebuffer/expected-result-1.png";
constexpr const char* RESULT_IMAGE_FILE_2 = TEST_IMAGE_DIR "multiple-window-offscreen-framebuffer/expected-result-2.png";

enum TestStep
{
  CHECK_DEFAULT_WINDOW,
  CHECK_SECOND_WINDOW,
  NUMBER_OF_STEPS
};

static int gTestStep = -1;
} // namespace

/**
 * @brief This is to test the functionality which allows the uploading of textures to the GPU
 * without rendering while the application is paused, and thus, have them available immediately
 * for rendering on resume.
 */
class MultipleWindowOffscreenFramebufferTest: public VisualTest
{
public:

  MultipleWindowOffscreenFramebufferTest(Application& application)
  : mApplication(application)
  {
  }

  void OnInit( Application& application )
  {
    mDefaultWindow = mApplication.GetWindow();
    mDefaultWindow.SetBackgroundColor(Color::WHITE);

    mSecondWindow = CreateNewWindow();
    mSecondWindow.SetBackgroundColor(Color::WHITE);

    CreateRenderTaskAndScene(mDefaultWindow, TEST_IMAGE_FILE_1, TARGET_SIZE_1, Vector4(1., 1., 1., 1.));
    CreateRenderTaskAndScene(mSecondWindow, TEST_IMAGE_FILE_2, TARGET_SIZE_2, Vector4(1., 0., 0., 1.));

    // Start the test
    WaitForNextTest();
  }

private:

  void CreateRenderTaskAndScene(Window window, const std::string& url, const Vector2 targetSize, const Vector4 clearColor)
  {
    auto rootActor = window.GetRootLayer();

    auto cameraActor = CameraActor::New(targetSize);
    cameraActor.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
    cameraActor.SetInvertYAxis(true);
    rootActor.Add(cameraActor);

    // create actor to render input
    Toolkit::ImageView actorForInput = Toolkit::ImageView::New(url);
    actorForInput.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
    actorForInput.SetProperty(Actor::Property::SIZE, targetSize);

    rootActor.Add(actorForInput);

    RenderTaskList taskList = window.GetRenderTaskList();

    // perform a horizontal blur targeting the internal buffer
    auto renderTask = taskList.CreateTask();
    renderTask.SetRefreshRate(RenderTask::REFRESH_ALWAYS);
    renderTask.SetSourceActor(actorForInput);
    renderTask.SetExclusive(true);
    renderTask.SetInputEnabled(false);
    renderTask.SetClearColor(clearColor);
    renderTask.SetClearEnabled(true);
    renderTask.SetCameraActor(cameraActor);

    Texture outputTexture = Texture::New(TextureType::TEXTURE_2D,
                                         Pixel::RGB888,
                                         unsigned(targetSize.width),
                                         unsigned(targetSize.height));
    auto framebuffer = FrameBuffer::New(targetSize.width, targetSize.height, FrameBuffer::Attachment::NONE);
    framebuffer.AttachColorTexture(outputTexture);

    renderTask.SetFrameBuffer(framebuffer);

    Toolkit::ImageView imageView = Toolkit::ImageView::New(Dali::Toolkit::TextureManager::AddTexture(outputTexture));

    imageView.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
    imageView.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER);
    rootActor.Add(imageView);
  }

  Dali::Window CreateNewWindow()
  {
    PositionSize windowSize;
    windowSize.width = 480;
    windowSize.height = 800;
    return Dali::Window::New( windowSize, "New window", "", Dali::Application::OPAQUE );
  }

  void WaitForNextTest()
  {
    gTestStep++;
    printf("\n\n\nWaitForNextTest: %d\n\n\n", gTestStep);
    mTimer = Timer::New( 500 ); // ms
    mTimer.TickSignal().Connect( this, &MultipleWindowOffscreenFramebufferTest::OnTimer);
    mTimer.Start();
  }

  bool OnTimer()
  {
    PerformTest();
    return false;
  }

  void PerformTest()
  {printf("\n\n\nPerformTest: %d\n\n\n", gTestStep);
    switch ( gTestStep )
    {
      case CHECK_DEFAULT_WINDOW:
      {
        CaptureWindow( mDefaultWindow );
        break;
      }
      case CHECK_SECOND_WINDOW:
      {
        CaptureWindow( mSecondWindow );
        break;
      }
      default:
        break;
    }
  }

  void PostRender()
  {printf("\n\n\nPostRender: %d\n\n\n", gTestStep);
    if ( gTestStep == CHECK_DEFAULT_WINDOW )
    {
      DALI_ASSERT_ALWAYS( CheckImage( RESULT_IMAGE_FILE_1 ) ); // should be identical
      WaitForNextTest();
    }
    else
    {
      DALI_ASSERT_ALWAYS( CheckImage( RESULT_IMAGE_FILE_2 ) ); // should be identical

      // The last check has been done, so we can quit the test
      mApplication.Quit();
    }
  }

private:
  Application&   mApplication;
  Window         mDefaultWindow;
  Window         mSecondWindow;
  Timer          mTimer;
};

DALI_VISUAL_TEST(MultipleWindowOffscreenFramebufferTest, OnInit)
