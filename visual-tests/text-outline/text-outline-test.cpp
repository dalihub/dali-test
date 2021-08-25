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
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali-toolkit/dali-toolkit.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{

const std::string IMAGE_FILE_1 = TEST_IMAGE_DIR "text-outline/expected-result-1.png";
const std::string IMAGE_FILE_2 = TEST_IMAGE_DIR "text-outline/expected-result-2.png";

const int NUMBER_OF_TEXT_LABELS = 6;

enum TestStep
{
  ENGLISH_OUTLINE,
  MULTI_LANGUAGE_OUTLINE,
  NUMBER_OF_STEPS
};

static int gTestStep = -1;

}  // namespace

/**
 * @brief This is to test the functionality which allows the uploading of textures to the GPU
 * without rendering while the application is paused, and thus, have them available immediately
 * for rendering on resume.
 */
class TextOutlineTest: public VisualTest
{
 public:

  TextOutlineTest( Application& application )
    : mApplication( application )
  {
  }

  void OnInit( Application& application )
  {
    Dali::Window window = mApplication.GetWindow();
    window.SetBackgroundColor(Color::WHITE);

    Property::Map outlineMap;
    outlineMap["color"] = Color::RED;
    outlineMap["width"] = 1.0f;

    for ( int i = 0; i < NUMBER_OF_TEXT_LABELS; i++ )
    {
      mTextLabel[i] = TextLabel::New();
      mTextLabel[i].SetProperty( Actor::Property::PARENT_ORIGIN,ParentOrigin::TOP_LEFT);
      mTextLabel[i].SetProperty( Actor::Property::ANCHOR_POINT,AnchorPoint::TOP_LEFT);
      mTextLabel[i].SetProperty( TextLabel::Property::OUTLINE, outlineMap );
      window.Add( mTextLabel[i] );
    }

    PerformNextTest();
  }

private:

  void PerformNextTest()
  {
    gTestStep++;

    switch ( gTestStep )
    {
      case ENGLISH_OUTLINE:
      {
        EnglishOutlineTest();
        break;
      }
      case MULTI_LANGUAGE_OUTLINE:
      {
        MultiLanguageOutlineTest();
        break;
      }
      default:
        break;
    }
  }

  void EnglishOutlineTest()
  {
    mTextLabel[0].SetProperty( TextLabel::Property::TEXT, "uVWXYZ" );
    mTextLabel[0].SetProperty( Actor::Property::POSITION, Vector2( 80.f, 20.f ) );
    mTextLabel[0].SetProperty(TextLabel::Property::POINT_SIZE, 27.f);

    mTextLabel[1].SetProperty( TextLabel::Property::TEXT, "uVWXYZ" );
    mTextLabel[1].SetProperty( Actor::Property::POSITION, Vector2( 80.f, 100.f ) );
    mTextLabel[1].SetProperty(TextLabel::Property::POINT_SIZE, 38.f);

    mTextLabel[2].SetProperty( TextLabel::Property::TEXT, "uVWXYZ" );
    mTextLabel[2].SetProperty( Actor::Property::POSITION, Vector2( 80.f, 200.f ) );
    mTextLabel[2].SetProperty(TextLabel::Property::POINT_SIZE, 56.f);

    mTextLabel[3].SetProperty( TextLabel::Property::TEXT, "uVWXYZ" );
    mTextLabel[3].SetProperty( Actor::Property::POSITION, Vector2( 80.f, 330.f ) );
    mTextLabel[3].SetProperty(TextLabel::Property::POINT_SIZE, 58.f);

    mTextLabel[4].SetProperty( TextLabel::Property::TEXT, "uVWXYZ" );
    mTextLabel[4].SetProperty( Actor::Property::POSITION, Vector2( 80.f, 470.f ) );
    mTextLabel[4].SetProperty(TextLabel::Property::POINT_SIZE, 60.f);

    mTextLabel[5].SetProperty( TextLabel::Property::TEXT, "uVWXYZ" );
    mTextLabel[5].SetProperty( Actor::Property::POSITION, Vector2( 80.f, 605.f ) );
    mTextLabel[5].SetProperty(TextLabel::Property::POINT_SIZE, 80.f);

    CaptureWindow( mApplication.GetWindow() );
  }

  void MultiLanguageOutlineTest()
  {
    mTextLabel[0].SetProperty( TextLabel::Property::TEXT, "\xF0\x9F\x98\x81 A Quick Brown Fox Jumps Over The Lazy Dog" );
    mTextLabel[0].SetProperty( TextLabel::Property::FONT_FAMILY, "BreezeColorEmoji" );
    mTextLabel[0].SetProperty( Actor::Property::POSITION, Vector2( 20.f, 20.f ) );
    mTextLabel[0].SetProperty(TextLabel::Property::POINT_SIZE, 16.f);

    mTextLabel[1].SetProperty( TextLabel::Property::TEXT, "تَعْدِيلْ قِسْمْ Arabic كَلِمَة أَرْمَلَة" );
    mTextLabel[1].SetProperty( Actor::Property::POSITION, Vector2( 0.f, 100.f ) );
    mTextLabel[1].SetProperty(TextLabel::Property::POINT_SIZE, 18.f);

    mTextLabel[2].SetProperty( TextLabel::Property::TEXT, "ໄປຫາຕາຕະລາງທີ່ກຳນົດ" );
    mTextLabel[2].SetProperty( Actor::Property::POSITION, Vector2( 20.f, 200.f ) );
    mTextLabel[2].SetProperty(TextLabel::Property::POINT_SIZE, 20.f);

    mTextLabel[3].SetProperty( TextLabel::Property::TEXT, "ပန္စီစဥ္ရန္ အျမင္ပံုစံကို စိတ္ၾကိဳက္လုပ္ႏိုင္ေသာ ဂရ" );
    mTextLabel[3].SetProperty( Actor::Property::POSITION, Vector2( 20.f, 330.f ) );
    mTextLabel[3].SetProperty(TextLabel::Property::POINT_SIZE, 22.f);

    mTextLabel[4].SetProperty( TextLabel::Property::TEXT, "ضرورت گرڈ میں تبدیل کرنا ہوگا" );
    mTextLabel[4].SetProperty( Actor::Property::POSITION, Vector2( 0.f, 470.f ) );
    mTextLabel[4].SetProperty(TextLabel::Property::POINT_SIZE, 24.f);

    mTextLabel[5].SetProperty( TextLabel::Property::TEXT, "보기 방식을 격자 보기(직접 설정)로" );
    mTextLabel[5].SetProperty( Actor::Property::POSITION, Vector2( 20.f, 605.f ) );
    mTextLabel[5].SetProperty(TextLabel::Property::POINT_SIZE, 26.f);

    CaptureWindow( mApplication.GetWindow() );
  }

  void PostRender()
  {
    if( gTestStep == ENGLISH_OUTLINE )
    {
      if(CheckImage(IMAGE_FILE_1, 0.95f))
      {
        PerformNextTest();
      }
      else
      {
        mApplication.Quit();
      }
    }
    else if( gTestStep == MULTI_LANGUAGE_OUTLINE )
    {
      CheckImage( IMAGE_FILE_2, 0.95f ); // verify the similarity

      // The last check has been done, so we can quit the test
      mApplication.Quit();
    }
  }

private:
  Application&            mApplication;
  TextLabel               mTextLabel[6];
};

DALI_VISUAL_TEST_WITH_WINDOW_SIZE( TextOutlineTest, OnInit, 1000, 850 )
