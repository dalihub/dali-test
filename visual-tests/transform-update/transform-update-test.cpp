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
#include <dali/dali.h>
#include <dali/integration-api/debug.h>
#include <dali-toolkit/dali-toolkit.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{

const unsigned int DEFAULT_DELAY_TIME = 200;

const char* const ICON_IMAGE(TEST_IMAGE_DIR "transform-update/circle.png");

const std::string FIRST_IMAGE_FILE = TEST_IMAGE_DIR "transform-update/expected-result-1.png";
const std::string SECOND_IMAGE_FILE = TEST_IMAGE_DIR "transform-update/expected-result-2.png";
const std::string THIRD_IMAGE_FILE = TEST_IMAGE_DIR "transform-update/expected-result-3.png";

const int WINDOW_WIDTH(480);
const int WINDOW_HEIGHT(800);

enum TestStep
{
  PRE_ANIMATION,
  START_ANIMATION,
  PAUSE_ANIMATION,
  CAPTURE_MID_ANIMATION,
  RESUME_ANIMATION,
  POST_ANIMATION,
  NUMBER_OF_STEPS
};

static int gTestStep = -1;

void AnimatingPositionConstraint(Vector3& current, const PropertyInputContainer& inputs)
{
  float   positionFactor(inputs[0]->GetFloat());
  Vector3 size(inputs[1]->GetVector3());
  current.x = size.x * (positionFactor - 0.5f);
}

}  // namespace

/**
 * @brief This is to test the use case in which render items should not be reused
 * after a transform update.
 */
class TransformUpdateTest: public VisualTest
{
 public:

  TransformUpdateTest( Application& application )
    : mApplication( application ),
      mAnimIcon(),
      mLayer(),
      mAnimation(),
      mPositionFactorIndex()
  {
  }

  void OnInit( Application& application )
  {
    Window window = mApplication.GetWindow();
    window.SetBackgroundColor(Color::WHITE);

    mLayer = Layer::New();
    mLayer.SetResizePolicy(ResizePolicy::FILL_TO_PARENT, Dimension::ALL_DIMENSIONS);
    mLayer.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
    window.Add(mLayer);

    mAnimIcon = ImageView::New(ICON_IMAGE);
    mAnimIcon.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
    mAnimIcon.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER);

    mPositionFactorIndex  = mAnimIcon.RegisterProperty("positionFactor", 0.2f);
    Constraint constraint = Constraint::New<Vector3>(mAnimIcon, Actor::Property::POSITION, AnimatingPositionConstraint);
    constraint.AddSource(Source(mAnimIcon, mPositionFactorIndex));
    constraint.AddSource(Source(mLayer, Actor::Property::SIZE));
    constraint.Apply();

    mLayer.Add(mAnimIcon);

    mAnimation = Animation::New(0.5f);

    Vector2 point1(0.25f, 0.05f), point2(0.75f, 0.95f);
    mAnimation.AnimateTo(Property(mAnimIcon, mPositionFactorIndex), 0.8f, AlphaFunction(point1, point2));

    // Start the test
    PrepareNextTest(DEFAULT_DELAY_TIME);
  }

private:

  void PrepareNextTest( unsigned int milliSecond )
  {
    gTestStep++;

    if ( milliSecond == 0u )
    {
      PerformTest();
    }
    else
    {
      mTimer = Timer::New( milliSecond );
      mTimer.TickSignal().Connect( this, &TransformUpdateTest::OnTimer);
      mTimer.Start();
    }
  }

  bool OnTimer()
  {
    PerformTest();
    return false;
  }

  void PerformTest()
  {
    Window window = mApplication.GetWindow();

    unsigned int delay = 0u;
    if (mAnimation)
    {
      delay = mAnimation.GetDuration() * 500;
    }

    switch (gTestStep)
    {
      case PRE_ANIMATION:
      {
        CaptureWindowAfterFrameRendered(window);
        break;
      }
      case START_ANIMATION:
      {
        if (mAnimation)
        {
          mAnimation.Play();
        }

        PrepareNextTest(delay); // play until half of the animation

        break;
      }
      case PAUSE_ANIMATION:
      {
        if (mAnimation)
        {
          mAnimation.Pause();
        }

        PrepareNextTest(DEFAULT_DELAY_TIME);

        break;
      }
      case CAPTURE_MID_ANIMATION:
      {
        CaptureWindowAfterFrameRendered(window);
        break;
      }
      case RESUME_ANIMATION:
      {
        if (mAnimation)
        {
          mAnimation.Play();
        }

        PrepareNextTest(delay + DEFAULT_DELAY_TIME);

        break;
      }
      case POST_ANIMATION:
      {
        CaptureWindowAfterFrameRendered(window);
        break;
      }
      default:
        break;
    }
  }

  void PostRender(std::string outputImage, bool success)
  {
    if (gTestStep == PRE_ANIMATION)
    {
      CompareImageFile(FIRST_IMAGE_FILE, outputImage, 0.99f);
      PrepareNextTest(DEFAULT_DELAY_TIME);
    }
    else if (gTestStep == CAPTURE_MID_ANIMATION)
    {
      CompareImageFile(SECOND_IMAGE_FILE, outputImage, 0.99f);
      PrepareNextTest(DEFAULT_DELAY_TIME);
    }
    else if ( gTestStep == POST_ANIMATION )
    {
      CompareImageFile(THIRD_IMAGE_FILE, outputImage, 0.99f);

      // The last check has been done, so we can quit the test
      mApplication.Quit();
    }
  }


private:
  Application&      mApplication;
  Timer             mTimer;
  ImageView         mAnimIcon;
  Layer             mLayer;
  Animation         mAnimation;
  Property::Index   mPositionFactorIndex;
};

DALI_VISUAL_TEST_WITH_WINDOW_SIZE ( TransformUpdateTest, OnInit, WINDOW_WIDTH, WINDOW_HEIGHT )
