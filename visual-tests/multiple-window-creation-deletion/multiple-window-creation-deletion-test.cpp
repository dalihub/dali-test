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
#include <dali-toolkit/dali-toolkit.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{

constexpr int MAX_WINDOW_COUNT = 30;
const PositionSize WINDOW_SIZE( 100, 100, 300, 300 );

}  // namespace

/**
 * @brief This test is to make sure we can create and delete multiple windows
 *        properly with no crash.
 */
class MultipleWIndowCreationDeletionTest: public VisualTest
{
public:

  MultipleWIndowCreationDeletionTest( Application& application )
  : mApplication( application )
  {
  }

  void OnInit( Application& application )
  {
    Dali::Window window = mApplication.GetWindow();
    window.SetBackgroundColor( Color::WHITE );

    mTextLabel = TextLabel::New( "Hello World" );
    mTextLabel.SetProperty( Actor::Property::ANCHOR_POINT, AnchorPoint::TOP_LEFT );
    window.Add( mTextLabel );

    // Start the test
    mTimer = Timer::New(100); // every 100ms
    mTimer.TickSignal().Connect(this, &MultipleWIndowCreationDeletionTest::OnTick);
    mTimer.Start();
  }

  void PostRender()
  {
    // We are not capturing any window, so this should not be called
  }

private:

  Dali::Window CreateNewWindow()
  {
    // Generate a window ID
    std::string index = std::to_string(++mWindowCount);

    // Create a text label
    Actor actor = TextLabel::New();
    actor.SetProperty(Control::Property::BACKGROUND, Color::RED);
    actor.SetProperty( Actor::Property::SIZE, Vector3( 100.0f, 100.0f, 0.0f ) );
    actor.SetProperty( Actor::Property::ANCHOR_POINT, AnchorPoint::TOP_LEFT );
    actor.SetProperty( Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_LEFT );
    actor.SetProperty(TextLabel::Property::TEXT, index.c_str());
    actor.SetProperty(TextLabel::Property::TEXT_COLOR, Color::WHITE);
    actor.SetProperty(TextLabel::Property::PIXEL_SIZE, 30);

    // Create a window and add the text label to it
    Window window = Dali::Window::New(WINDOW_SIZE, index.c_str(), "", Dali::Application::OPAQUE);
    window.SetBackgroundColor( Color::CYAN );
    window.Add(actor);

    return window;
  }

  bool OnTick()
  {
    // Stop generating window
    if (mWindowCount >= MAX_WINDOW_COUNT)
    {
      // Since we have created/deleted maximum number of new windows, quit the test
      mApplication.Quit();

      return false;
    }

    // Delete the created window
    mNewWindow.Reset();

    // Create a new window
    mNewWindow = CreateNewWindow();

    return true;
  }

private:

  Application& mApplication;
  Timer mTimer;
  Window mNewWindow;
  TextLabel mTextLabel;
  int mWindowCount = 0;
};

DALI_VISUAL_TEST( MultipleWIndowCreationDeletionTest, OnInit )

