/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali-toolkit/dali-toolkit.h>
#include <dali-toolkit/devel-api/controls/text-controls/text-editor-devel.h>
#include <dali-toolkit/devel-api/controls/text-controls/text-label-devel.h>
#include <dali-toolkit/devel-api/text/text-enumerations-devel.h>
#include <dali/dali.h>
#include <dali/integration-api/debug.h>
#include <string>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace {

const std::string IMAGE_FILE_HYPHEN =
    TEST_IMAGE_DIR "text-wrapping-hyphen/hyphen-expected-result.png";
const std::string IMAGE_FILE_MIXED =
    TEST_IMAGE_DIR "text-wrapping-hyphen/mixed-expected-result.png";
const std::string IMAGE_FILE_HYPHEN_EDITOR =
    TEST_IMAGE_DIR "text-wrapping-hyphen/hyphen-editor-expected-result.png";
const std::string IMAGE_FILE_MIXED_EDITOR =
    TEST_IMAGE_DIR "text-wrapping-hyphen/mixed-editor-expected-result.png";

enum TestStep {
  HYPHEN_WRAPPING_LABEL,
  MIXED_WRAPPING_LABEL,
  HYPHEN_WRAPPING_EDITOR,
  MIXED_WRAPPING_EDITOR,
  NUMBER_OF_STEPS
};

static int gTestStep = -1;

} // namespace

/**
 * @brief This is to test the functionality of text wrapping modes (hyphen,
 * mixed)
 */
class TextWrappingTest : public VisualTest {
public:
  TextWrappingTest(Application &application) : mApplication(application) {}

  void OnInit(Application &application) {
    mWindow = mApplication.GetWindow();
    mWindow.SetBackgroundColor(Color::WHITE);

    mTextLabel = TextLabel::New();
    mTextLabel.SetProperty(Actor::Property::PARENT_ORIGIN,
                           ParentOrigin::TOP_LEFT);
    mTextLabel.SetProperty(Actor::Property::ANCHOR_POINT,
                           AnchorPoint::TOP_LEFT);
    mTextLabel.SetProperty(Actor::Property::POSITION, Vector3(0.f, 0.0f, 0.f));
    mTextLabel.SetProperty(TextLabel::Property::POINT_SIZE, 200);
    mTextLabel.SetProperty(TextLabel::Property::ELLIPSIS, false);
    mTextLabel.SetProperty(TextLabel::Property::MULTI_LINE, true);

    // TODO : Since 2.3.32, we change the default value of REMOVE_FRONT_INSET and REMOVE_BACK_INSET as false.
    mTextLabel.SetProperty(DevelTextLabel::Property::REMOVE_FRONT_INSET, true);
    mTextLabel.SetProperty(DevelTextLabel::Property::REMOVE_BACK_INSET, true);

    mTextEditor = TextEditor::New();
    mTextEditor.SetProperty(Actor::Property::PARENT_ORIGIN,
                            ParentOrigin::TOP_LEFT);
    mTextEditor.SetProperty(Actor::Property::ANCHOR_POINT,
                            AnchorPoint::TOP_LEFT);
    mTextEditor.SetProperty(Actor::Property::POSITION, Vector3(0.f, 0.0f, 0.f));
    mTextEditor.SetProperty(TextEditor::Property::POINT_SIZE, 200);

    // TODO : Since 2.3.32, we change the default value of REMOVE_FRONT_INSET and REMOVE_BACK_INSET as false.
    mTextEditor.SetProperty(DevelTextEditor::Property::REMOVE_FRONT_INSET, true);
    mTextEditor.SetProperty(DevelTextEditor::Property::REMOVE_BACK_INSET, true);

    mWindow.Add(mTextLabel);
    PerformNextTest();
  }

private:
  void PerformNextTest()
  {
    gTestStep++;

    switch (gTestStep)
    {
    case HYPHEN_WRAPPING_LABEL:
    {
      HyphenWrappingTestLabel();
      break;
    }
    case MIXED_WRAPPING_LABEL:
    {
      MixedWrappingTestLabel();
      break;
    }
    case HYPHEN_WRAPPING_EDITOR:
    {
      mWindow.Remove(mTextLabel);
      mWindow.Add(mTextEditor);
      HyphenWrappingTestEditor();
      break;
    }
    case MIXED_WRAPPING_EDITOR:
    {
      MixedWrappingTestEditor();
      break;
    }
    default:
      break;
    }
    StartDrawTimer();
  }


  void HyphenWrappingTestLabel()
  {
    mTextLabel.SetProperty(TextLabel::Property::LINE_WRAP_MODE,
                           DevelText::LineWrap::HYPHENATION);
    mTextLabel.SetProperty(TextLabel::Property::TEXT, "Hi Experiment");
  }

  void MixedWrappingTestLabel()
  {
    mTextLabel.SetProperty(TextLabel::Property::LINE_WRAP_MODE,
                           DevelText::LineWrap::MIXED);
    mTextLabel.SetProperty(TextLabel::Property::TEXT, "Hi Experiment");
  }

  void HyphenWrappingTestEditor()
  {
    mTextEditor.SetProperty(TextEditor::Property::LINE_WRAP_MODE,
                            DevelText::LineWrap::HYPHENATION);
    mTextEditor.SetProperty(TextEditor::Property::TEXT, "Hi Experiment");
  }

  void MixedWrappingTestEditor()
  {
    mTextEditor.SetProperty(TextEditor::Property::LINE_WRAP_MODE,
                            DevelText::LineWrap::MIXED);
    mTextEditor.SetProperty(TextEditor::Property::TEXT, "Hi Experiment");
  }

  void StartDrawTimer()
  {
    Debug::LogMessage(Debug::INFO, "Starting draw and check()\n");

    Animation firstFrameAnimator = Animation::New(0);
    firstFrameAnimator.FinishedSignal().Connect(this, &TextWrappingTest::OnAnimationFinished1);
    firstFrameAnimator.Play();
  }

  void OnAnimationFinished1(Animation& /* not used */)
  {
    Debug::LogMessage(Debug::INFO, "First Update done()\n");
    Animation secondFrameAnimator = Animation::New(0);
    secondFrameAnimator.FinishedSignal().Connect(this, &TextWrappingTest::OnAnimationFinished2);
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
    const std::string images[] = {IMAGE_FILE_HYPHEN ,IMAGE_FILE_MIXED ,IMAGE_FILE_HYPHEN_EDITOR ,IMAGE_FILE_MIXED_EDITOR};
    CompareImageFile(images[gTestStep], outputFile, 0.95f);
    if(gTestStep < NUMBER_OF_STEPS-1)
    {
      PerformNextTest();
    }
    else
    {
      mApplication.Quit();
    }
  }

private:
  Application &mApplication;
  Dali::Window mWindow;
  TextLabel mTextLabel;
  TextEditor mTextEditor;
};

DALI_VISUAL_TEST_WITH_WINDOW_SIZE(TextWrappingTest, OnInit, 900, 900)
