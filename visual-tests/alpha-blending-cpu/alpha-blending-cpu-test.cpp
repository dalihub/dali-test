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
#include <dali/integration-api/debug.h>
#include <dali-toolkit/dali-toolkit.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{
constexpr const char* IMAGES[] =
{
  TEST_IMAGE_DIR "alpha-blending-cpu/people-small-7b.jpg", // 100x100
  TEST_IMAGE_DIR "alpha-blending-cpu/people-medium-7.jpg",
  TEST_IMAGE_DIR "alpha-blending-cpu/people-medium-7-rgb565.png", // is compressed
  TEST_IMAGE_DIR "alpha-blending-cpu/people-medium-7-masked.png"  // has alpha channel
};
constexpr auto IMAGE_COUNT = sizeof(IMAGES) / sizeof(IMAGES[0]);

struct MaskMode
{
  const char* url              = nullptr;
  const float maskContentScale = 1.0f;
  const bool  cropToMask       = false;
};
constexpr MaskMode MASK_MODES[] =
{
  {},                                          // No mask
  {TEST_IMAGE_DIR "alpha-blending-cpu/store_mask_profile_n.png"}, // 300x300 mask with default scale & crop
  {TEST_IMAGE_DIR "alpha-blending-cpu/store_mask_profile_f.png", 1.6f, true},
};
constexpr auto  MASK_MODE_COUNT = sizeof(MASK_MODES) / sizeof(MASK_MODES[0]);
constexpr float IMAGE_SIZE = 200.0f;
constexpr const char* EXPECTED_IMAGE_FILE = TEST_IMAGE_DIR "alpha-blending-cpu/expected-result-1.png";

constexpr auto NUMBER_OF_IMAGES = IMAGE_COUNT * MASK_MODE_COUNT;
constexpr int WINDOW_WIDTH = MASK_MODE_COUNT * IMAGE_SIZE;
constexpr int WINDOW_HEIGHT = IMAGE_COUNT * IMAGE_SIZE;
} // namespace

/**
 * @brief This is to test the functionality which allows the uploading of textures to the GPU
 * without rendering while the application is paused, and thus, have them available immediately
 * for rendering on resume.
 */
class AlphaBlendingCpuTest: public VisualTest
{
public:

  AlphaBlendingCpuTest(Application& application)
  : mApplication(application)
  {
  }

  void OnInit( Application& application )
  {
    Window defaultWindow = mApplication.GetWindow();
    defaultWindow.SetBackgroundColor(Color::WHITE);

    float yPosition = 0.0f;
    for(auto image = 0u; image < IMAGE_COUNT; ++image, yPosition += IMAGE_SIZE)
    {
      float xPosition = 0.0f;
      for(auto mask = 0u; mask < MASK_MODE_COUNT; ++mask, xPosition += IMAGE_SIZE)
      {
        Toolkit::ImageView imageView = Toolkit::ImageView::New();

        imageView.SetProperty(Actor::Property::SIZE, Vector2(IMAGE_SIZE, IMAGE_SIZE));
        imageView.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_LEFT);
        imageView.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::TOP_LEFT);
        imageView.SetProperty(Actor::Property::POSITION_Y, yPosition);
        imageView.SetProperty(Actor::Property::POSITION_X, xPosition);
        defaultWindow.Add(imageView);

        Property::Map map;
        const char*   maskUrl  = MASK_MODES[mask].url;
        const char*   imageUrl = IMAGES[image];
        map.Add(Toolkit::Visual::Property::TYPE, Toolkit::Visual::Type::IMAGE);
        map.Add(Toolkit::ImageVisual::Property::URL, imageUrl);
        if(maskUrl)
        {
          map.Add(Toolkit::ImageVisual::Property::ALPHA_MASK_URL, maskUrl);
          map.Add(Toolkit::ImageVisual::Property::MASK_CONTENT_SCALE, MASK_MODES[mask].maskContentScale);
          map.Add(Toolkit::ImageVisual::Property::CROP_TO_MASK, MASK_MODES[mask].cropToMask);
        }
        imageView.SetProperty(Toolkit::ImageView::Property::IMAGE, map);
        imageView.ResourceReadySignal().Connect(this, &AlphaBlendingCpuTest::OnReady);
      }
    }
  }

private:

  void OnReady(Control control)
  {
    static int readyCounter=0;
    readyCounter++;
    if(readyCounter == NUMBER_OF_IMAGES)
    {
      Debug::LogMessage(Debug::INFO, "Starting draw and check()\n");

      Animation firstFrameAnimator = Animation::New(0);
      firstFrameAnimator.FinishedSignal().Connect(this, &AlphaBlendingCpuTest::OnAnimationFinished1);
      firstFrameAnimator.Play();
    }
  }

  void OnAnimationFinished1(Animation& /* not used */)
  {
    Debug::LogMessage(Debug::INFO, "First Update done()\n");
    Animation secondFrameAnimator = Animation::New(0);
    secondFrameAnimator.FinishedSignal().Connect(this, &AlphaBlendingCpuTest::OnAnimationFinished2);
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
    CompareImageFile(EXPECTED_IMAGE_FILE, outputFile, 0.98f);
    mApplication.Quit();
  }

private:
  Application& mApplication;
};

DALI_VISUAL_TEST_WITH_WINDOW_SIZE(AlphaBlendingCpuTest, OnInit, WINDOW_WIDTH, WINDOW_HEIGHT)
