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
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>
#include <dali-toolkit/dali-toolkit.h>
#include <dali-toolkit/devel-api/visuals/image-visual-properties-devel.h>
#include <dali-toolkit/devel-api/visual-factory/visual-factory.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{

const std::string IMAGE_FILE = TEST_IMAGE_DIR "remote-download/expected-result-1.png";
const int NUMBER_OF_IMAGES(4);
struct ImageData
{
  std::string url;
  Vector2 position;
};
const ImageData IMAGES[NUMBER_OF_IMAGES] = {
    {"https://raw.githubusercontent.com/dalihub/dali-test/refs/heads/master/visual-tests/remote-download/images/rockstar.jpg", Vector2(0, 50)},
    {"https://raw.githubusercontent.com/dalihub/dali-test/refs/heads/master/visual-tests/remote-download/images/stars.jpg", Vector2(400, 100)},
    {"https://raw.githubusercontent.com/dalihub/dali-test/refs/heads/master/visual-tests/remote-download/images/stormtrooper.jpg", Vector2(0, 400)},
    {"http://static.midomi.com/h/images/w/weather_sunny.png", Vector2(800, 100)}
};


}  // namespace

/**
 * @brief This is to test the functionality which allows the uploading of textures to the GPU
 * without rendering while the application is paused, and thus, have them available immediately
 * for rendering on resume.
 */
class RemoteDownloadTest: public VisualTest
{
public:

  RemoteDownloadTest( Application& application )
    : mApplication( application )
  {
  }

  void OnInit( Application& application )
  {
    Window defaultWindow = mApplication.GetWindow();
    defaultWindow.SetBackgroundColor(Color::WHITE);

    int i=0;
    for( auto& image : IMAGES )
    {
      mImageViews[i] = Toolkit::ImageView::New(image.url);
      mImageViews[i].SetProperty( Actor::Property::PARENT_ORIGIN,ParentOrigin::TOP_LEFT);
      mImageViews[i].SetProperty( Actor::Property::ANCHOR_POINT,AnchorPoint::TOP_CENTER);
      mImageViews[i].SetProperty(Actor::Property::POSITION_USES_ANCHOR_POINT, false);
      mImageViews[i].SetProperty( Actor::Property::POSITION, image.position);
      mImageViews[i].SetBackgroundColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
      mImageViews[i].ResourceReadySignal().Connect(this, &RemoteDownloadTest::OnReady);
      defaultWindow.Add(mImageViews[i]);
      ++i;
    }
  }

private:

  void OnReady(Control control)
  {
    static int readyCounter=0;
    readyCounter++;
    if(readyCounter == NUMBER_OF_IMAGES)
    {
      StartDrawTimer();
    }
  }

  void StartDrawTimer()
  {
    Debug::LogMessage(Debug::INFO, "Starting draw and check()\n");

    Animation firstFrameAnimator = Animation::New(0);
    firstFrameAnimator.FinishedSignal().Connect(this, &RemoteDownloadTest::OnAnimationFinished1);
    firstFrameAnimator.Play();
  }

  void OnAnimationFinished1(Animation& /* not used */)
  {
    Debug::LogMessage(Debug::INFO, "First Update done()\n");
    Animation secondFrameAnimator = Animation::New(0);
    secondFrameAnimator.FinishedSignal().Connect(this, &RemoteDownloadTest::OnAnimationFinished2);
    secondFrameAnimator.Play();
  }

  void OnAnimationFinished2(Animation& /* not used */)
  {
    Window window = mApplication.GetWindow();
    Debug::LogMessage(Debug::INFO, "Second Update done(). We can assume that at least 1 frame rendered now. Capturing window\n");
    CaptureWindow(window);
  }

  void PostRender(std::string outputFile, bool success)
  {
    // All steps will have same result.
    CompareImageFile(IMAGE_FILE, outputFile, 0.98f);
    mApplication.Quit();
  }

private:
  Application&   mApplication;
  ImageView      mImageViews[NUMBER_OF_IMAGES];
};

DALI_VISUAL_TEST_WITH_WINDOW_SIZE( RemoteDownloadTest, OnInit, 1024, 960 )
