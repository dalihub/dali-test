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

#include <dali/devel-api/adaptor-framework/application-devel.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{
const std::string EXPECTED_IMAGE_FILE = TEST_IMAGE_DIR "preinitialize-vk/expected-result.png";
}  // namespace

/**
 * @brief This test is to make sure we can create PreInitialize application with Vulkan backend.
 */
class PreInitialieTestVulkan: public VisualTest
{
public:

  PreInitialieTestVulkan( Application& application )
  : mApplication( application )
  {
  }

  void OnInit( Application& application )
  {
    Dali::Window window = mApplication.GetWindow();
    window.SetBackgroundColor( Color::WHITE );

    Control simpleControl = Control::New();
    simpleControl[Actor::Property::PARENT_ORIGIN] = ParentOrigin::CENTER;
    simpleControl[Actor::Property::SIZE] = Vector2(100.0f, 100.0f);
    simpleControl[Control::Property::BACKGROUND] = Color::RED;

    window.Add(simpleControl);

    // Start the test
    PerformNextTest();
  }

  void PostRender(std::string outputFile, bool success) override
  {
    CompareImageFile(EXPECTED_IMAGE_FILE, outputFile, 0.98f);
    mApplication.Quit();
  }

  void PerformNextTest()
  {
    Window window = mApplication.GetWindow();
    CaptureWindowAfterFrameRendered(window);
  }

private:
  Application& mApplication;
};

void PreInitialize()
{
  // Set preferred backend as Vulkan.
  setenv("DALI_GRAPHICS_BACKEND", "VK", 1);

  printf("ApplicationPreInitialize\n");
  ApplicationPreInitialize(nullptr, nullptr);
  printf("ApplicationPreInitialize done\n");

  // Forcibly set backend as Vulkan.
  Dali::Graphics::SetGraphicsBackend(Dali::Graphics::Backend::VULKAN);
}

DALI_VISUAL_TEST_WITH_WINDOW_SIZE_AND_PREPROESS( PreInitialieTestVulkan, OnInit, 200, 200, PreInitialize )
