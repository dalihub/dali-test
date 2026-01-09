/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali-toolkit/devel-api/controls/control-devel.h>
#include <dali-toolkit/devel-api/visuals/visual-properties-devel.h>
#include <dali-toolkit/devel-api/visuals/image-visual-properties-devel.h>
#include <dali-toolkit/devel-api/visuals/animated-image-visual-actions-devel.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{
// Resource for drawing
const std::string JPG_FILENAME           = TEST_IMAGE_DIR "corner-radius-visual/gallery-medium-16.jpg";
const std::string TRANSLUCENT_FILENAME   = TEST_IMAGE_DIR "alpha-blending-cpu/people-medium-7-masked.png";
const std::string PRE_MULTIPLIED_FILENAME   = TEST_IMAGE_DIR "pre-multiply-alpha-policy/fxh.png";
const std::string ANIMATED_WEBP_FILENAME = TEST_IMAGE_DIR "corner-radius-visual/dog-anim.webp";
const std::string ANIMATED_GIF_FILENAME = TEST_IMAGE_DIR "pre-multiply-alpha-policy/dali-logo-anim.gif";

// Resource for visual comparison
const std::string EXPECTED_IMAGE_FILE = TEST_IMAGE_DIR "pre-multiply-alpha-policy/expected-result.png";

constexpr static int VISUAL_SIZE    = 200;
constexpr static int MARGIN_VISUALS = 2;

constexpr static int TESTSET_VISUAL_SIZE =  VISUAL_SIZE + MARGIN_VISUALS * 2;

// Want to test propeties about pre-multiplied alpha policy list, which will show same visual result with standard result.
const static Property::Value PRE_MULTIPLIED_ALPHA_POLICY_LIST[] =
{
  Property::Value(DevelImageVisual::PreMultiplyAlphaPolicyType::FOLLOW_VISUAL_PROPERTY),
  Property::Value(DevelImageVisual::PreMultiplyAlphaPolicyType::FOLLOW_VISUAL_TYPE),
  Property::Value(DevelImageVisual::PreMultiplyAlphaPolicyType::MULTIPLY_ON_LOAD_AND_AUTO_RENDER),
  Property::Value(DevelImageVisual::PreMultiplyAlphaPolicyType::MULTIPLY_OFF_LOAD_AND_OFF_RENDER),
};
constexpr static int NUMBER_OF_PROPERTY_TYPES = sizeof(PRE_MULTIPLIED_ALPHA_POLICY_LIST) / sizeof(PRE_MULTIPLIED_ALPHA_POLICY_LIST[0]);

// Want to test propeties about pre-multiplied alpha policy list, which will change the visual result as standard result.
const static Property::Value IRREGULAR_PRE_MULTIPLIED_ALPHA_POLICY_LIST[] =
{
  Property::Value(DevelImageVisual::PreMultiplyAlphaPolicyType::MULTIPLY_OFF_LOAD_AND_ON_RENDER),
};
constexpr static int NUMBER_OF_IRREGULAR_PROPERTY_TYPES = sizeof(IRREGULAR_PRE_MULTIPLIED_ALPHA_POLICY_LIST) / sizeof(IRREGULAR_PRE_MULTIPLIED_ALPHA_POLICY_LIST[0]);

// Valid visual type list
const static std::string IMAGE_URL_LIST[] =
{
  JPG_FILENAME,
  TRANSLUCENT_FILENAME,
  PRE_MULTIPLIED_FILENAME,
  ANIMATED_WEBP_FILENAME,
  ANIMATED_GIF_FILENAME,
};
constexpr static int NUMBER_OF_IMAGE_TYPES = sizeof(IMAGE_URL_LIST) / sizeof(IMAGE_URL_LIST[0]);

constexpr static int TOTAL_RESOURCES = (1 + NUMBER_OF_IRREGULAR_PROPERTY_TYPES) * NUMBER_OF_IMAGE_TYPES; // Total amount of resource to ready

enum TestStep
{
  CREATE_FOLLOW_VISUAL_PROPERTY_PREMULTIPLIED_STEP,
  CREATE_FOLLOW_VISUAL_TYPE_PREMULTIPLIED_STEP,
  CREATE_MULTIPLY_ON_LOAD_AND_AUTO_RENDER_PREMULTIPLIED_STEP,
  CREATE_MULTIPLY_OFF_LOAD_AND_OFF_RENDER_PREMULTIPLIED_STEP,
  CREATE_FOLLOW_VISUAL_PROPERTY_NO_PREMULTIPLIED_STEP,
  CREATE_FOLLOW_VISUAL_TYPE_NO_PREMULTIPLIED_STEP,
  CREATE_MULTIPLY_ON_LOAD_AND_AUTO_RENDER_NO_PREMULTIPLIED_STEP,
  CREATE_MULTIPLY_OFF_LOAD_AND_OFF_RENDER_NO_PREMULTIPLIED_STEP,
  NUMBER_OF_STEPS
};
constexpr static int TERMINATE_RUNTIME = 10 * 1000; // 10 seconds

static int gTestStep = -1;
static int gResourceReadyCount = 0;
static bool gTermiatedTest = false;

}  // namespace

/**
 * @brief This is to test the functionality of native image and image visual
 */
class PreMultiplyAlphaPolicyTest: public VisualTest
{
public:

  PreMultiplyAlphaPolicyTest( Application& application )
    : mApplication( application )
  {
  }

  ~PreMultiplyAlphaPolicyTest()
  {
  }

  void OnInit( Application& application )
  {
    mWindow = mApplication.GetWindow();
    mWindow.SetBackgroundColor(Color::GRAY);

    mTerminateTimer = Timer::New(TERMINATE_RUNTIME);
    mTerminateTimer.TickSignal().Connect(this, &PreMultiplyAlphaPolicyTest::OnTerminateTimer);
    mTerminateTimer.Start();

    PrepareNextTest();
  }

private:

  bool OnTerminateTimer()
  {
    // Visual Test Timout!
    printf("TIMEOUT pre-multiply-alpha-policy.test spend more than %d ms\n",TERMINATE_RUNTIME);

    gTermiatedTest = true;
    gExitValue = -1;
    mApplication.Quit();

    exit(gExitValue);

    return false;
  }

  void PrepareNextTest()
  {
    Window window = mApplication.GetWindow();

    gTestStep++;

    if(gTestStep >= NUMBER_OF_STEPS)
    {
      return;
    }

    CreateVisuals(gTestStep % NUMBER_OF_PROPERTY_TYPES, (gTestStep < NUMBER_OF_PROPERTY_TYPES));
  }

  void OnReady(Dali::Toolkit::Control control)
  {
    // Resource ready done. Check we need to go to next step
    gResourceReadyCount++;
    if(gResourceReadyCount == TOTAL_RESOURCES)
    {
      CaptureWindowAfterFrameRendered(mApplication.GetWindow());
    }
  }

  void PostRender(std::string outputFile, bool success)
  {
    CompareImageFile(EXPECTED_IMAGE_FILE, outputFile, 0.98f);
    if(gTestStep < NUMBER_OF_STEPS-1)
    {
      UnparentAllControls();
      PrepareNextTest();
    }
    else
    {
      // The last check has been done, so we can quit the test
      mTerminateTimer.Stop();
      mApplication.Quit();
    }
  }

private:
  void CreateVisuals(int propertyTestType, bool requiredPreMulitpliedAlpha)
  {
    // Reset resource ready count
    gResourceReadyCount = 0;

    // Create Visuals for each testset types.
    for(std::uint32_t imageTypeIndex = 0; imageTypeIndex < NUMBER_OF_IMAGE_TYPES; ++imageTypeIndex)
    {
      CreateTestSet(imageTypeIndex, 0, PRE_MULTIPLIED_ALPHA_POLICY_LIST[propertyTestType], requiredPreMulitpliedAlpha);
      for(std::uint32_t irregularPropertyTestTypeIndex = 0; irregularPropertyTestTypeIndex < NUMBER_OF_IRREGULAR_PROPERTY_TYPES; ++irregularPropertyTestTypeIndex)
      {
        CreateTestSet(imageTypeIndex, irregularPropertyTestTypeIndex + 1, IRREGULAR_PRE_MULTIPLIED_ALPHA_POLICY_LIST[irregularPropertyTestTypeIndex], requiredPreMulitpliedAlpha);
      }
    }
  }

  // Main setup here!
  // Costumize here if you want
  void CreateTestSet(std::uint32_t imageTypeIndex, std::uint32_t xPosition, const Property::Value& preMultiplyAlphaPolicy, bool requiredPreMulitpliedAlpha)
  {
    // Calculate controls size and position from TOP_LEFT
    Vector2 controlSize     = Vector2(VISUAL_SIZE, VISUAL_SIZE);
    Vector2 controlPosition = Vector2(xPosition * TESTSET_VISUAL_SIZE + MARGIN_VISUALS, imageTypeIndex * TESTSET_VISUAL_SIZE + MARGIN_VISUALS);
    // Create new Control and setup default data
    ImageView control = ImageView::New();
    control[Actor::Property::PARENT_ORIGIN] = ParentOrigin::TOP_LEFT;
    control[Actor::Property::ANCHOR_POINT]  = AnchorPoint::TOP_LEFT;
    control[Actor::Property::SIZE]          = controlSize;
    control[Actor::Property::POSITION]      = controlPosition;

    // Attach resource ready signal
    control.ResourceReadySignal().Connect(this, &PreMultiplyAlphaPolicyTest::OnReady);

    // Set image
    control[ImageView::Property::IMAGE] = CreateImageVisualMap(imageTypeIndex, preMultiplyAlphaPolicy);

    // Send STOP action for animate image
    DevelControl::DoAction(control, ImageView::Property::IMAGE, DevelAnimatedImageVisual::Action::STOP, Property::Value());

    // Set pre-multiplied alpha value for ImageView.
    control[ImageView::Property::PRE_MULTIPLIED_ALPHA] = requiredPreMulitpliedAlpha;

    mWindow.Add(control);
    // For clean up
    mControlList.emplace_back(control);
  }

  void UnparentAllControls()
  {
    for(auto&& actor : mControlList)
    {
      actor.Unparent();
      actor.Reset();
    }
    mControlList.clear();
  }

private:
  Property::Map CreateImageVisualMap(std::uint32_t imageTypeIndex, const Property::Value& preMultiplyAlphaPolicy)
  {
    Property::Map visualMap;
    visualMap[Visual::Property::TYPE]     = Visual::IMAGE;
    visualMap[ImageVisual::Property::URL] = IMAGE_URL_LIST[imageTypeIndex];
    visualMap[DevelImageVisual::Property::PRE_MULTIPLY_ALPHA_POLICY] = preMultiplyAlphaPolicy;

    // We will control the animation by StopAction.
    visualMap[Toolkit::DevelImageVisual::Property::STOP_BEHAVIOR] = DevelImageVisual::StopBehavior::FIRST_FRAME;
    return visualMap;
  }

private:
  Application&         mApplication;
  Window               mWindow;
  Timer                mTerminateTimer;
  Animation            mAnimation;
  std::vector<Control> mControlList;
};

DALI_VISUAL_TEST_WITH_WINDOW_SIZE( PreMultiplyAlphaPolicyTest, OnInit, TESTSET_VISUAL_SIZE * (1 + NUMBER_OF_IRREGULAR_PROPERTY_TYPES), TESTSET_VISUAL_SIZE * NUMBER_OF_IMAGE_TYPES )
