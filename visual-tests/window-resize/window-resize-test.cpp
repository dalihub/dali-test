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
#include <dali/integration-api/debug.h>

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
  FULL_WINDOW_CAPTURE,
  PARTIAL_WINDOW_CAPTURE,
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
    PrepareNextTest();
  }

private:

  void PrepareNextTest()
  {
    gTestStep++;
    if(gTestStep == PARTIAL_WINDOW_CAPTURE)
    {
      // Resize the window
      Window window = mApplication.GetWindow();
      Window::WindowSize WINDOW_SIZE(300, 600);
      window.SetSize(WINDOW_SIZE);
    }
    CaptureWindowAfterFrameRendered(mApplication.GetWindow());
  }

  void PostRender(std::string outputFile, bool success)
  {
    std::string images[] = { FIRST_IMAGE_FILE, SECOND_IMAGE_FILE };

    CompareImageFile(images[gTestStep], outputFile, 0.98f);
    if(gTestStep + 1u == NUMBER_OF_STEPS)
    {
      mApplication.Quit();
    }
    else
    {
      PrepareNextTest();
    }
  }

private:
  Application& mApplication;
  Actor        mActor;
};

DALI_VISUAL_TEST(WindowResizeTest, OnInit)
