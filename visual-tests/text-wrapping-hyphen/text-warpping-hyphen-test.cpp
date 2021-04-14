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
#include <dali-toolkit/devel-api/text/text-enumerations-devel.h>
#include <dali/dali.h>
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

    mTextEditor = TextEditor::New();
    mTextEditor.SetProperty(Actor::Property::PARENT_ORIGIN,
                            ParentOrigin::TOP_LEFT);
    mTextEditor.SetProperty(Actor::Property::ANCHOR_POINT,
                            AnchorPoint::TOP_LEFT);
    mTextEditor.SetProperty(Actor::Property::POSITION, Vector3(0.f, 0.0f, 0.f));
    mTextEditor.SetProperty(TextEditor::Property::POINT_SIZE, 200);

    mWindow.Add(mTextLabel);
    PerformNextTest();
  }

private:
  void PerformNextTest() {
    gTestStep++;

    switch (gTestStep) {
    case HYPHEN_WRAPPING_LABEL: {
      HyphenWrappingTestLabel();
      break;
    }
    case MIXED_WRAPPING_LABEL: {
      MixedWrappingTestLabel();
      break;
    }
    case HYPHEN_WRAPPING_EDITOR: {
      HyphenWrappingTestEditor();
      break;
    }
    case MIXED_WRAPPING_EDITOR: {
      MixedWrappingTestEditor();
      break;
    }
    default:
      break;
    }
  }

  void HyphenWrappingTestLabel() {
    mTextLabel.SetProperty(TextLabel::Property::LINE_WRAP_MODE,
                           DevelText::LineWrap::HYPHENATION);
    mTextLabel.SetProperty(TextLabel::Property::TEXT, "Hi Experiment");
    CaptureWindow(mWindow);
  }

  void MixedWrappingTestLabel() {
    mTextLabel.SetProperty(TextLabel::Property::LINE_WRAP_MODE,
                           DevelText::LineWrap::MIXED);
    mTextLabel.SetProperty(TextLabel::Property::TEXT, "Hi Experiment");
    CaptureWindow(mWindow);
  }

  void HyphenWrappingTestEditor() {
    mTextEditor.SetProperty(TextEditor::Property::LINE_WRAP_MODE,
                            DevelText::LineWrap::HYPHENATION);
    mTextEditor.SetProperty(TextEditor::Property::TEXT, "Hi Experiment");
    CaptureWindow(mWindow);
  }

  void MixedWrappingTestEditor() {
    mTextEditor.SetProperty(TextEditor::Property::LINE_WRAP_MODE,
                            DevelText::LineWrap::MIXED);
    mTextEditor.SetProperty(TextEditor::Property::TEXT, "Hi Experiment");
    CaptureWindow(mWindow);
  }

  void PostRender() {
    if (gTestStep == HYPHEN_WRAPPING_LABEL) {
      DALI_ASSERT_ALWAYS(
          CheckImage(IMAGE_FILE_HYPHEN, 0.98f)); // verify the similarity
      PerformNextTest();
    } else if (gTestStep == MIXED_WRAPPING_LABEL) {
      DALI_ASSERT_ALWAYS(
          CheckImage(IMAGE_FILE_MIXED, 0.98f)); // verify the similarity
      mWindow.Remove(mTextLabel);
      mWindow.Add(mTextEditor);
      PerformNextTest();
    } else if (gTestStep == HYPHEN_WRAPPING_EDITOR) {
      DALI_ASSERT_ALWAYS(
          CheckImage(IMAGE_FILE_HYPHEN_EDITOR, 0.98f)); // verify the similarity
      PerformNextTest();
    } else if (gTestStep == MIXED_WRAPPING_EDITOR) {
      DALI_ASSERT_ALWAYS(
          CheckImage(IMAGE_FILE_MIXED_EDITOR, 0.98f)); // verify the similarity

      // The last check has been done, so we can quit the test
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
