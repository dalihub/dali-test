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
#include <dali/dali.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>
#include <dali-toolkit/dali-toolkit.h>
#include <dali/devel-api/adaptor-framework/window-devel.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{

const std::string IMAGE_FILE_1 = TEST_IMAGE_DIR "empty-scene-clear/expected-result-1.png";
const std::string IMAGE_FILE_2 = TEST_IMAGE_DIR "empty-scene-clear/expected-result-2.png";
const std::string IMAGE_FILE_3 = TEST_IMAGE_DIR "empty-scene-clear/expected-result-3.png";

enum TestStep
{
  FIRST_WINDOW,
  SECOND_WINDOW,
  THIRD_WINDOW,
  NUMBER_OF_STEPS
};

static int gTestStep = -1;

}  // namespace

/**
 * @brief This is to test the functionality which allows the uploading of textures to the GPU
 * without rendering while the application is paused, and thus, have them available immediately
 * for rendering on resume.
 */
class EmptySceneClearTest: public VisualTest
{
 public:

  EmptySceneClearTest( Application& application )
    : mApplication( application )
  {
  }

  void OnInit( Application& application )
  {
    Dali::Window window = mApplication.GetWindow();
    window.SetBackgroundColor(Color::WHITE);

    mTextLabel = TextLabel::New( "Hello World" );
    mTextLabel.SetProperty( Actor::Property::ANCHOR_POINT, AnchorPoint::TOP_LEFT );
    window.Add( mTextLabel );

    // Start the test
    PerformNextTest();
  }

private:

  Dali::Window CreateNewWindow()
  {
    PositionSize windowSize;
    windowSize.width = 480;
    windowSize.height = 800;
    return Dali::Window::New( windowSize, "New window", "", Dali::Application::OPAQUE );
  }

  void PerformNextTest()
  {
    gTestStep++;
    switch ( gTestStep )
    {
      case FIRST_WINDOW:
      {
        mTestWindow = mApplication.GetWindow();
        StartDrawTimer();
        break;
      }
      case SECOND_WINDOW:
      {
        // Create an empty window with no renderable actors
        mSecondWindow = CreateNewWindow();
        mSecondWindow.SetBackgroundColor( Color::CYAN );
        mTestWindow = mSecondWindow;
        StartDrawTimer();
        break;
      }
      case THIRD_WINDOW:
      {

        // Create another empty window with no renderable actors
        mThirdWindow = CreateNewWindow();
        mThirdWindow.SetBackgroundColor( Color::RED );
        mTestWindow = mThirdWindow;
        StartDrawTimer();
        break;
      }
      default:
        break;
    }
  }

  void StartDrawTimer()
  {
    Debug::LogMessage(Debug::INFO, "Starting draw and check()\n");

    Animation firstFrameAnimator = Animation::New(0);
    firstFrameAnimator.FinishedSignal().Connect(this, &EmptySceneClearTest::OnAnimationFinished1);
    firstFrameAnimator.Play();
  }

  void OnAnimationFinished1(Animation& /* not used */)
  {
    Debug::LogMessage(Debug::INFO, "First Update done()\n");
    Animation secondFrameAnimator = Animation::New(0);
    secondFrameAnimator.FinishedSignal().Connect(this, &EmptySceneClearTest::OnAnimationFinished2);
    secondFrameAnimator.Play();
  }

  void OnAnimationFinished2(Animation& /* not used */)
  {
    Debug::LogMessage(Debug::INFO, "Second Update done(). We can assume that at least 1 frame rendered now. Capturing window\n");
    CaptureWindow(mTestWindow);
  }

  void PostRender(std::string outputFile, bool success)
  {
    const std::string images[] = { IMAGE_FILE_1, IMAGE_FILE_2, IMAGE_FILE_3 };
    CompareImageFile(images[gTestStep], outputFile, 0.95f);
    if(gTestStep < THIRD_WINDOW)
    {
      PerformNextTest();
    }
    else
    {
      mApplication.Quit();
    }
  }

private:

  Application& mApplication;
  Dali::Window mTestWindow;
  Dali::Window mSecondWindow;
  Dali::Window mThirdWindow;
  TextLabel    mTextLabel;
};

DALI_VISUAL_TEST( EmptySceneClearTest, OnInit )
