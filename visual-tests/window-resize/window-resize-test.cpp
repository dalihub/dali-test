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
#include <dali-toolkit/dali-toolkit.h>
#include <dali/dali.h>

#include <string>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{
const std::string ICON_FILE =
  TEST_IMAGE_DIR "window-resize/circle.png";
const std::string FIRST_IMAGE_FILE =
  TEST_IMAGE_DIR "window-resize/expected-result-1.png";
const std::string SECOND_IMAGE_FILE =
  TEST_IMAGE_DIR "window-resize/expected-result-2.png";

enum TestStep
{
  FIRST_CAPTURE,
  WINDOW_RESIZE,
  SECOND_CAPTURE,
  NUMBER_OF_STEPS
};

static int gTestStep = -1;

} // namespace

/**
 * @brief This is to test the use case that when the window is resized the
 * local matrix of the child nodes should be updated and therefore the position
 * of the child actors should be recomputed.
 */
class WindowResizeTest : public VisualTest
{
public:
  WindowResizeTest(Application& application)
  : mApplication(application)
  {
  }

  void OnInit(Application& application)
  {
    Window window = mApplication.GetWindow();
    window.SetBackgroundColor(Color::WHITE);

    mActor = ImageView::New(ICON_FILE);
    mActor.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::TOP_LEFT);
    window.Add(mActor);

    // Start the test
    WaitForNextTest();
  }

private:
  void WaitForNextTest()
  {
    gTestStep++;

    mTimer = Timer::New(200); // ms
    mTimer.TickSignal().Connect(this, &WindowResizeTest::OnTimer);
    mTimer.Start();
  }

  bool OnTimer()
  {
    PerformTest();
    return false;
  }

  void PerformTest()
  {
    Window window = mApplication.GetWindow();

    switch(gTestStep)
    {
      case FIRST_CAPTURE:
      {
        CaptureWindow(window);

        WaitForNextTest();
        break;
      }
      case WINDOW_RESIZE:
      {
        // Resize the window
        Window::WindowSize WINDOW_SIZE(300, 600);
        window.SetSize(WINDOW_SIZE);

        WaitForNextTest();
        break;
      }
      case SECOND_CAPTURE:
      {
        CaptureWindow(window);
        break;
      }
      default:
        break;
    }
  }

  void PostRender()
  {
    if(gTestStep == SECOND_CAPTURE)
    {
      CheckImage(SECOND_IMAGE_FILE); // should be identical

      // The last check has been done, so we can quit the test
      mApplication.Quit();
    }
    else
    {
      if(!CheckImage(FIRST_IMAGE_FILE)) // should be identical
      {
        mTimer.Stop();
        mApplication.Quit();
      }
    }
  }

private:
  Application& mApplication;
  Actor        mActor;
  Timer        mTimer;
};

DALI_VISUAL_TEST(WindowResizeTest, OnInit)
