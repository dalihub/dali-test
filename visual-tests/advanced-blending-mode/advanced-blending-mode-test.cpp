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

#include <dali-toolkit/dali-toolkit.h>
#include <dali/devel-api/actors/actor-devel.h>

#include <dali/devel-api/common/capabilities.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using Dali::Toolkit::TextLabel;

const char* BLEND_IMAGE=TEST_IMAGE_DIR "advanced-blending-mode/gallery-large-19.jpg";
const char* EXPECTED_IMAGE_FILE=TEST_IMAGE_DIR "advanced-blending-mode/expected-result.png";

class AdvancedBlendingModeTest : public VisualTest
{
public:
  AdvancedBlendingModeTest(Application& application)
  : mApplication(application)
  {
  }

  ~AdvancedBlendingModeTest() = default; // Nothing to do in destructor

  // The Init signal is received once (only) during the Application lifetime
  void OnInit(Application& application)
  {
    // Get a handle to the stage
    Window window = application.GetWindow();
    window.SetBackgroundColor(Color::BLACK);

    Toolkit::ImageView imageView = Toolkit::ImageView::New();
    Property::Map      imagePropertyMap;
    imagePropertyMap.Insert(Toolkit::Visual::Property::TYPE, Toolkit::Visual::IMAGE);
    imagePropertyMap.Insert(Toolkit::ImageVisual::Property::URL, BLEND_IMAGE );
    imageView.SetProperty(Toolkit::ImageView::Property::IMAGE, imagePropertyMap);
    imageView.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_CENTER);
    imageView.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::TOP_CENTER);
    imageView.SetProperty(Actor::Property::SIZE, Vector2(600, 600));
    window.Add(imageView);
    imageView.ResourceReadySignal().Connect(this, &AdvancedBlendingModeTest::OnReady);

    Toolkit::Control control_1 = Toolkit::Control::New();
    Property::Map    colorVisualMap_1;
    colorVisualMap_1.Insert(Toolkit::Visual::Property::TYPE, Toolkit::Visual::COLOR);
    colorVisualMap_1.Insert(Toolkit::ColorVisual::Property::MIX_COLOR, Dali::Color::RED);
    colorVisualMap_1.Insert(Toolkit::Visual::Property::PREMULTIPLIED_ALPHA, true);
    control_1.SetProperty(Toolkit::Control::Property::BACKGROUND, colorVisualMap_1);
    control_1.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_CENTER);
    control_1.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::TOP_CENTER);
    control_1.SetProperty(Actor::Property::POSITION, Vector2(0, 0));
    control_1.SetProperty(Actor::Property::SIZE, Vector2(600, 200));
    if(Dali::Capabilities::IsBlendEquationSupported(Dali::DevelBlendEquation::SCREEN))
    {
      control_1.SetProperty(Dali::DevelActor::Property::BLEND_EQUATION, Dali::DevelBlendEquation::LUMINOSITY);
    }
    window.Add(control_1);

    Toolkit::Control control_2 = Toolkit::Control::New();
    Property::Map    colorVisualMap_2;
    colorVisualMap_2.Insert(Toolkit::Visual::Property::TYPE, Toolkit::Visual::COLOR);
    colorVisualMap_2.Insert(Toolkit::ColorVisual::Property::MIX_COLOR, Dali::Color::GREEN);
    colorVisualMap_2.Insert(Toolkit::Visual::Property::PREMULTIPLIED_ALPHA, true);
    control_2.SetProperty(Toolkit::Control::Property::BACKGROUND, colorVisualMap_2);
    control_2.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_CENTER);
    control_2.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::TOP_CENTER);
    control_2.SetProperty(Actor::Property::POSITION, Vector2(0, 200));
    control_2.SetProperty(Actor::Property::SIZE, Vector2(600, 200));
    if(Dali::Capabilities::IsBlendEquationSupported(Dali::DevelBlendEquation::SCREEN))
    {
      control_2.SetProperty(Dali::DevelActor::Property::BLEND_EQUATION, Dali::DevelBlendEquation::LUMINOSITY);
    }
    window.Add(control_2);

    Toolkit::Control control_3 = Toolkit::Control::New();
    Property::Map    colorVisualMap_3;
    colorVisualMap_3.Insert(Toolkit::Visual::Property::TYPE, Toolkit::Visual::COLOR);
    colorVisualMap_3.Insert(Toolkit::ColorVisual::Property::MIX_COLOR, Dali::Color::BLUE);
    colorVisualMap_3.Insert(Toolkit::Visual::Property::PREMULTIPLIED_ALPHA, true);
    control_3.SetProperty(Toolkit::Control::Property::BACKGROUND, colorVisualMap_3);
    control_3.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_CENTER);
    control_3.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::TOP_CENTER);
    control_3.SetProperty(Actor::Property::POSITION, Vector2(0, 400));
    control_3.SetProperty(Actor::Property::SIZE, Vector2(600, 200));
    if(Dali::Capabilities::IsBlendEquationSupported(Dali::DevelBlendEquation::SCREEN))
    {
      control_3.SetProperty(Dali::DevelActor::Property::BLEND_EQUATION, Dali::DevelBlendEquation::LUMINOSITY);
    }
    window.Add(control_3);
  }

  void OnReady(Dali::Toolkit::Control control)
  {
    Window window = mApplication.GetWindow();
    Debug::LogMessage(Debug::INFO, "Resource loaded\n");

    CaptureWindowAfterFrameRendered(window);
  }

  void PostRender(std::string outputFile, bool success)
  {
    CompareImageFile(EXPECTED_IMAGE_FILE, outputFile, 0.98f);
    mApplication.Quit();
  }

private:
  Application& mApplication;
};

DALI_VISUAL_TEST_WITH_WINDOW_SIZE( AdvancedBlendingModeTest, OnInit, 720, 800 )
