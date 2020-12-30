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
#include <dali-toolkit/dali-toolkit.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{

const std::string IMAGE_FILE_1 = TEST_IMAGE_DIR "clipping-mode/expected-result-1.png";
const std::string IMAGE_FILE_2 = TEST_IMAGE_DIR "clipping-mode/expected-result-2.png";
const std::string IMAGE_FILE_3 = TEST_IMAGE_DIR "clipping-mode/expected-result-3.png";
const std::string IMAGE_FILE_4 = TEST_IMAGE_DIR "clipping-mode/expected-result-4.png";

enum TestStep
{
  CLIP_CHILDREN_01,
  CLIP_CHILDREN_02,
  CLIP_TO_BOUNDING_BOX_01,
  CLIP_TO_BOUNDING_BOX_02,
  NUMBER_OF_STEPS
};

static int gTestStep = -1;

}  // namespace

/**
 * @brief This is to test clipping modes.
 */
class ClippingModeTest: public VisualTest
{
 public:

  ClippingModeTest( Application& application )
    : mApplication( application )
  {
  }

  void OnInit( Application& application )
  {
    Dali::Window window = mApplication.GetWindow();
    window.SetBackgroundColor(Color::WHITE);

    // Start the test
    PerformNextTest();
  }

private:

  void CreateSimpleScene(ClippingMode::Type clippingMode)
  {
    mContainer = Actor::New();
    mContainer.SetResizePolicy(ResizePolicy::FILL_TO_PARENT, Dimension::ALL_DIMENSIONS);
    mContainer[Actor::Property::PARENT_ORIGIN] = ParentOrigin::CENTER;
    mApplication.GetWindow().Add(mContainer);

    Control A = Control::New();
    A[Actor::Property::ANCHOR_POINT] = AnchorPoint::TOP_LEFT;
    A[Actor::Property::SIZE] = Vector2(200.0f, 200.0f);
    A[Actor::Property::POSITION] = Vector2(100.0f, 100.0f);
    A[Actor::Property::CLIPPING_MODE] = clippingMode;
    A[Control::Property::BACKGROUND] = Color::RED;
    mContainer.Add(A);

    Control B = Control::New();
    B[Actor::Property::ANCHOR_POINT] = AnchorPoint::TOP_LEFT;
    B[Actor::Property::SIZE] = Vector2(200.0f, 200.0f);
    B[Actor::Property::POSITION] = Vector2(50.0f, 50.0f);
    B[Actor::Property::CLIPPING_MODE] = clippingMode;
    B[Control::Property::BACKGROUND] = Color::BLUE;
    A.Add(B);

    Control C = Control::New();
    C[Actor::Property::ANCHOR_POINT] = AnchorPoint::TOP_LEFT;
    C[Actor::Property::SIZE] = Vector2(200.0f, 200.0f);
    C[Actor::Property::POSITION] = Vector2(100.0f, -50.0f);
    C[Control::Property::BACKGROUND] = Color::YELLOW;
    A.Add(C);
  }

  void CreateComplextScene(ClippingMode::Type clippingMode)
  {
    mContainer = Actor::New();
    mContainer.SetResizePolicy(ResizePolicy::FILL_TO_PARENT, Dimension::ALL_DIMENSIONS);
    mContainer[Actor::Property::PARENT_ORIGIN] = ParentOrigin::CENTER;
    mApplication.GetWindow().Add(mContainer);

    /*
              A(clip,red)     E(magenta)
             /          \
          B(clip,green) F(cyan)
           /          \
      C(clip,yellow)  D(blue)
        /        \
      G(black)   H(grey)
    */
    Control A = Control::New();
    A[Actor::Property::ANCHOR_POINT] = AnchorPoint::TOP_LEFT;
    A[Actor::Property::SIZE] = Vector2(300.0f, 300.0f);
    A[Actor::Property::POSITION] = Vector2(100.0f, 100.0f);
    A[Actor::Property::CLIPPING_MODE] = clippingMode;
    A[Control::Property::BACKGROUND] = Color::RED;
    mContainer.Add(A);

    Control B = Control::New();
    B[Actor::Property::ANCHOR_POINT] = AnchorPoint::TOP_LEFT;
    B[Actor::Property::SIZE] = Vector2(300.0f, 300.0f);
    B[Actor::Property::POSITION] = Vector2(50.0f, 50.0f);
    B[Actor::Property::CLIPPING_MODE] = clippingMode;
    B[Control::Property::BACKGROUND] = Color::GREEN;
    A.Add(B);

    Control C = Control::New();
    C[Actor::Property::ANCHOR_POINT] = AnchorPoint::TOP_LEFT;
    C[Actor::Property::SIZE] = Vector2(250.0f, 250.0f);
    C[Actor::Property::POSITION] = Vector2(50.0f, 50.0f);
    C[Actor::Property::CLIPPING_MODE] = clippingMode;
    C[Control::Property::BACKGROUND] = Color::YELLOW;
    B.Add(C);

    Control D = Control::New();
    D[Actor::Property::ANCHOR_POINT] = AnchorPoint::TOP_LEFT;
    D[Actor::Property::SIZE] = Vector2(200.0f, 200.0f);
    D[Actor::Property::POSITION] = Vector2(-50.0f, -50.0f);
    D[Control::Property::BACKGROUND] = Color::BLUE;
    B.Add(D);

    Control E = Control::New();
    E[Actor::Property::ANCHOR_POINT] = AnchorPoint::TOP_LEFT;
    E[Actor::Property::SIZE] = Vector2(100.0f, 100.0f);
    E[Actor::Property::POSITION] = Vector2(170.0f, 320.0f);
    E[Control::Property::BACKGROUND] = Color::MAGENTA;
    mContainer.Add(E);

    Control F = Control::New();
    F[Actor::Property::ANCHOR_POINT] = AnchorPoint::TOP_LEFT;
    F[Actor::Property::SIZE] = Vector2(150.0f, 180.0f);
    F[Actor::Property::POSITION] = Vector2(120.0f, -50.0f);
    F[Control::Property::BACKGROUND] = Color::CYAN;
    A.Add(F);

    Control G = Control::New();
    G[Actor::Property::ANCHOR_POINT] = AnchorPoint::TOP_LEFT;
    G[Actor::Property::SIZE] = Vector2(200.0f, 100.0f);
    G[Actor::Property::POSITION] = Vector2(50.0f, 150.0f);
    G[Control::Property::BACKGROUND] = Color::BLACK;
    C.Add(G);

    Control H = Control::New();
    H[Actor::Property::ANCHOR_POINT] = AnchorPoint::TOP_LEFT;
    H[Actor::Property::SIZE] = Vector2(30.0f, 150.0f);
    H[Actor::Property::POSITION] = Vector2(150.0f, 50.0f);
    H[Control::Property::BACKGROUND] = Color::GREY;
    C.Add(H);
  }

  void RemoveScene()
  {
    mContainer.Unparent();
  }

  void PerformNextTest()
  {
    Window window = mApplication.GetWindow();

    gTestStep++;
    switch(gTestStep)
    {
      case CLIP_CHILDREN_01:
      {
        CreateSimpleScene(ClippingMode::CLIP_CHILDREN);
        CaptureWindow(window);
        break;
      }
      case CLIP_CHILDREN_02:
      {
        RemoveScene();
        CreateComplextScene(ClippingMode::CLIP_CHILDREN);
        CaptureWindow(window);
        break;
      }
      case CLIP_TO_BOUNDING_BOX_01:
      {
        RemoveScene();
        CreateSimpleScene(ClippingMode::CLIP_TO_BOUNDING_BOX);
        CaptureWindow(window);
        break;
      }
      case CLIP_TO_BOUNDING_BOX_02:
      {
        RemoveScene();
        CreateComplextScene(ClippingMode::CLIP_TO_BOUNDING_BOX);
        CaptureWindow(window);
        break;
      }
      default:
      {
        break;
      }
    }
  }

  void PostRender()
  {
    if( gTestStep == CLIP_CHILDREN_01 )
    {
      DALI_ASSERT_ALWAYS( CheckImage( IMAGE_FILE_1) ); // ensure identical
      PerformNextTest();
    }
    else if( gTestStep == CLIP_CHILDREN_02 )
    {
      DALI_ASSERT_ALWAYS( CheckImage( IMAGE_FILE_2 ) ); // ensure identical
      PerformNextTest();
    }
    else if( gTestStep == CLIP_TO_BOUNDING_BOX_01 )
    {
      DALI_ASSERT_ALWAYS( CheckImage( IMAGE_FILE_3 ) ); // ensure identical
      PerformNextTest();
    }
    else if( gTestStep == CLIP_TO_BOUNDING_BOX_02 )
    {
      DALI_ASSERT_ALWAYS( CheckImage( IMAGE_FILE_4 ) ); // ensure identical

      // The last check has been done, so we can quit the test
      mApplication.Quit();
    }
  }

private:

  Application& mApplication;
  Actor mContainer;
};

DALI_VISUAL_TEST( ClippingModeTest, OnInit )
