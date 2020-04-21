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
    mTextLabel.SetAnchorPoint( AnchorPoint::TOP_LEFT );
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
        CaptureWindow( mApplication.GetWindow() );
        break;
      }
      case SECOND_WINDOW:
      {
        // Create an empty window with no renderable actors
        mSecondWindow = CreateNewWindow();
        mSecondWindow.SetBackgroundColor( Color::CYAN );

        CaptureWindow( mSecondWindow );
        break;
      }
      case THIRD_WINDOW:
      {

        // Create another empty window with no renderable actors
        mThirdWindow = CreateNewWindow();
        mThirdWindow.SetBackgroundColor( Color::RED );

        CaptureWindow( mThirdWindow );
        break;
      }
      default:
        break;
    }
  }

  void PostRender()
  {
    if ( gTestStep == FIRST_WINDOW )
    {
      DALI_ASSERT_ALWAYS( CheckImage( IMAGE_FILE_1, 0.95f ) ); // verify the similarity
      PerformNextTest();
    }
    else if ( gTestStep == SECOND_WINDOW )
    {
      DALI_ASSERT_ALWAYS( CheckImage( IMAGE_FILE_2 ) ); // ensure identical
      PerformNextTest();
    }
    else if ( gTestStep == THIRD_WINDOW )
    {
      DALI_ASSERT_ALWAYS( CheckImage( IMAGE_FILE_3 ) ); // ensure identical

      // The last check has been done, so we can quit the test
      mApplication.Quit();
    }
  }

private:

  Application& mApplication;
  Dali::Window mSecondWindow;
  Dali::Window mThirdWindow;
  TextLabel    mTextLabel;
};

DALI_VISUAL_TEST( EmptySceneClearTest, OnInit )

