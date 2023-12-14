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
#include <dali-toolkit/devel-api/controls/table-view/table-view.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{
const int DRAW_TIME{1000};

constexpr const char* IMAGES[] = {
  TEST_IMAGE_DIR "clipping-draw-order/gallery-small-1.jpg",
  TEST_IMAGE_DIR "clipping-draw-order/gallery-small-2.jpg",
  TEST_IMAGE_DIR "clipping-draw-order/gallery-small-3.jpg",
  TEST_IMAGE_DIR "clipping-draw-order/gallery-small-4.jpg",
  TEST_IMAGE_DIR "clipping-draw-order/gallery-small-5.jpg"
};
constexpr auto NUMBER_OF_IMAGES = sizeof(IMAGES) / sizeof(IMAGES[0]);
constexpr auto NUMBER_OF_IMAGE_VIEWS = NUMBER_OF_IMAGES * 2;

constexpr const char* EXPECTED_RESULT_IMAGE = TEST_IMAGE_DIR "clipping-draw-order/expected-result-1.png";

constexpr int WINDOW_WIDTH = 480;
constexpr int WINDOW_HEIGHT = 800;
} // namespace

/**
 * @brief This is to test the CLIP_CHILDREN feature.
 */
class ClippingDrawOrderVerification: public VisualTest
{
public:

  ClippingDrawOrderVerification(Application& application)
  : mApplication(application)
  {
  }

  void OnInit( Application& application )
  {
    Window defaultWindow = mApplication.GetWindow();
    defaultWindow.SetBackgroundColor(Color::WHITE);

    /*
     * Create a 2-row TableView.
     * It will be segmented as follows:
     *
     *    +---------------+
     *    |               |
     *    |     Tree      |
     *    |    Without    |
     *    |   Clipping    |
     *    |               |
     *    +---------------+
     *    |               |
     *    |     Tree      |
     *    |     With      |
     *    |   Clipping    |
     *    |               |
     *    +---------------+
     */
    TableView view = TableView::New(2, 1);
    view.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER);
    view.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
    view.SetResizePolicy(ResizePolicy::FILL_TO_PARENT, Dimension::ALL_DIMENSIONS);
    view.SetCellAlignment(Toolkit::TableView::CellPosition(0, 0), HorizontalAlignment::CENTER, VerticalAlignment::CENTER);
    view.SetCellAlignment(Toolkit::TableView::CellPosition(1, 0), HorizontalAlignment::CENTER, VerticalAlignment::CENTER);
    view.SetCellPadding(Vector2(14.0f, 7.0f));
    view.SetRelativeWidth(0u, 1.0f);
    view.SetRelativeHeight(0u, 0.5f);
    view.SetRelativeHeight(0u, 0.5f);

    /*
       For each of the 2 tree views, we create a small tree of actors as follows:
       ( Note: Clipping is only enabled for B on the bottom tree ).

                             A
                            / \
       Clipping enabled -> B   D
                           |   |
                           C   E

       The correct draw order is "ABCDE" (the same as if clipping was not enabled).
    */
    const float treeYStart = 0.12f;
    const float depthGap   = 0.35f;

    for(int tree = 0; tree < 2; ++tree)
    {
      Control container = Control::New();
      container.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::TOP_CENTER);
      container.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_CENTER);
      container.SetResizePolicy(ResizePolicy::FILL_TO_PARENT, Dimension::ALL_DIMENSIONS);
      Vector4 backgroundColor = tree == 0 ? Vector4(0.77f, 1.0f, 0.77f, 1.0f) : Vector4(0.8f, 0.8f, 1.0f, 1.0f);
      container.SetProperty(Control::Property::BACKGROUND, backgroundColor);
      ImageView image[5];

      // Loop for each of the images
      for(auto i = 0u; i < NUMBER_OF_IMAGES; ++i)
      {
        image[i] = ImageView::New(IMAGES[i]);
        image[i].ResourceReadySignal().Connect(this, &ClippingDrawOrderVerification::OnReady);
        image[i].SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::TOP_CENTER);

        if(i == 0)
        {
          image[i].SetProperty(Actor::Property::PARENT_ORIGIN, Vector3(0.5f, treeYStart, 0.5f));
        }
        else
        {
          float b = i > 2 ? 1.0f : -1.0f;
          image[i].SetProperty(Actor::Property::PARENT_ORIGIN, Vector3(0.5f + (0.2f * b), depthGap, 0.5f));
        }
      }

      // Enable clipping for the 2nd tree.
      if(tree == 1)
      {
        image[1].SetProperty(Actor::Property::CLIPPING_MODE, ClippingMode::CLIP_CHILDREN);
      }

      // Build the tree structure.
      container.Add(image[0]);

      image[0].Add(image[1]);
      image[1].Add(image[2]);

      image[0].Add(image[3]);
      image[3].Add(image[4]);

      // Add the finished tree to the TableView.
      view.AddChild(container, TableView::CellPosition(tree, 0u));
    }

    // Add the finished TableView to the window.
    defaultWindow.Add(view);
  }

private:

  void OnReady(Control control)
  {
    static int readyCounter=0;
    readyCounter++;
    if(readyCounter == NUMBER_OF_IMAGE_VIEWS)
    {
      mTimer = Timer::New(DRAW_TIME);
      mTimer.TickSignal().Connect(this, &ClippingDrawOrderVerification::OnTimer);
      mTimer.Start();
    }
  }

  bool OnTimer()
  {
    Window window = mApplication.GetWindow();
    CaptureWindow( window );
    return false;
  }

  void PostRender(std::string outputFile, bool success)
  {
    CompareImageFile(EXPECTED_RESULT_IMAGE, outputFile, 0.98f);
    mApplication.Quit();
  }

private:
  Application&   mApplication;
  Timer mTimer;
};

DALI_VISUAL_TEST_WITH_WINDOW_SIZE(ClippingDrawOrderVerification, OnInit, WINDOW_WIDTH, WINDOW_HEIGHT)
