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
#include <dali-toolkit/devel-api/visuals/image-visual-properties-devel.h>
#include <dali-toolkit/devel-api/visual-factory/visual-factory.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{

const char* IMAGE_PATH[] = {
    TEST_IMAGE_DIR "resource-uploading/initial-image.jpg",
    TEST_IMAGE_DIR "resource-uploading/large-image-01.png",
    TEST_IMAGE_DIR "resource-uploading/large-image-02.png",
    TEST_IMAGE_DIR "resource-uploading/large-image-03.png",
    TEST_IMAGE_DIR "resource-uploading/large-image-04.png"
};

const std::string FIRST_IMAGE_FILE = TEST_IMAGE_DIR "resource-uploading/expected-result-1.png";
const std::string SECOND_IMAGE_FILE = TEST_IMAGE_DIR "resource-uploading/expected-result-2.png";

const unsigned int NUMBER_OF_IMAGES = 2;

enum TestStep
{
  IMAGE_CHANGE,
  ADAPTOR_PAUSE,
  ADAPTOR_RESUME,
  NUMBER_OF_STEPS
};

static int gTestStep = -1;

}  // namespace

/**
 * @brief This is to test the functionality which allows the uploading of textures to the GPU
 * without rendering while the application is paused, and thus, have them available immediately
 * for rendering on resume.
 */
class ResourceUploadingTest: public VisualTest
{
 public:

  ResourceUploadingTest( Application& application )
    : mApplication( application ),
      mImageIndex( 0 )
  {
  }

  void OnInit( Application& application )
  {
    Window defaultWindow = mApplication.GetWindow();
    defaultWindow.SetBackgroundColor(Color::WHITE);

    // Create a container to layout the images.
    FlexContainer container = FlexContainer::New();
    container.SetProperty( Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER );
    container.SetProperty( Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER );
    container.SetResizePolicy( ResizePolicy::SIZE_RELATIVE_TO_PARENT, Dimension::ALL_DIMENSIONS );
    Vector3 offset( 0.9f, 0.70f, 0.0f );
    container.SetProperty( Actor::Property::SIZE_MODE_FACTOR, offset );
    container.SetProperty( FlexContainer::Property::FLEX_DIRECTION, FlexContainer::ROW );
    defaultWindow.Add( container );

    for( unsigned int x = 0; x < NUMBER_OF_IMAGES; x++ )
    {
      mImageViews[x] = ImageView::New( );
      Property::Map imagePropertyMap;
      imagePropertyMap.Insert( Visual::Property::TYPE, Toolkit::Visual::IMAGE );
      imagePropertyMap.Insert( ImageVisual::Property::URL, IMAGE_PATH[0] );
      mImageViews[x].SetProperty(ImageView::Property::IMAGE, imagePropertyMap );
      mImageViews[x].SetProperty( Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER );
      mImageViews[x].SetProperty( Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER );
      mImageViews[x].SetProperty( FlexContainer::ChildProperty::FLEX, 0.5f );
      mImageViews[x].SetResizePolicy( ResizePolicy::FILL_TO_PARENT, Dimension::HEIGHT );
      container.Add( mImageViews[x] );
    }

    // Start the test
    WaitForNextTest();
  }

private:

  void WaitForNextTest()
  {
    gTestStep++;

    mTimer = Timer::New( 1000 ); // ms
    mTimer.TickSignal().Connect( this, &ResourceUploadingTest::OnTimer);
    mTimer.Start();
  }

  bool OnTimer()
  {
    PerformTest();
    return false;
  }

  void PerformTest()
  {
    Window window = mApplication.GetWindow();

    switch ( gTestStep )
    {
      case IMAGE_CHANGE:
      {
        // Change the images
        ChangeImage();

        // We check the content of the window immediately after the changing the images in the image view.
        // As the images are very large, it will take some time for the textures to be uploaded to GPU
        // before rendering,　so the new images will not be rendered yet when we check the content of
        // the window　here.
        CaptureWindow( window );

        WaitForNextTest();
        break;
      }
      case ADAPTOR_PAUSE:
      {
        // Pause the adaptor
        Adaptor::Get().Pause();

        // Change the images in the image view again when adaptor is paused
        ChangeImage();

        // After the adaptor is paused, we must wait for a while before resuming the adaptor
        // This will allow enough time for the textures to be uploaded while the adaptor is paused
        WaitForNextTest();
        break;
      }
      case ADAPTOR_RESUME:
      {
        // Resume the adaptor
        Adaptor::Get().Resume();

        // We check the content of the window immediately after the adaptor is resumed.
        // The textures should be uploaded already when the adaptor is paused, so the
        // rendering of the images should be instant.
        CaptureWindow( window );
        break;
      }
      default:
        break;
    }
  }

  void ChangeImage()
  {
    int index = ( mImageIndex % 2 ) * NUMBER_OF_IMAGES + 1;

    for ( unsigned int i = 0; i < NUMBER_OF_IMAGES; i++ )
    {
      if ( mImageViews[i].GetProperty< bool >( Actor::Property::CONNECTED_TO_SCENE ) )

      {
        Property::Map imagePropertyMap;
        imagePropertyMap.Insert( Visual::Property::TYPE,  Toolkit::Visual::IMAGE );
        imagePropertyMap.Insert( ImageVisual::Property::URL,  IMAGE_PATH[i + index] );
        mImageViews[i].SetProperty( ImageView::Property::IMAGE , imagePropertyMap );
      }
    }

    mImageIndex++;
  }

  void PostRender()
  {
    if ( gTestStep == ADAPTOR_RESUME )
    {
      CheckImage( SECOND_IMAGE_FILE ); // should be identical

      // The last check has been done, so we can quit the test
      mApplication.Quit();
    }
    else
    {
      if(!CheckImage( FIRST_IMAGE_FILE)) // should be identical
      {
        mTimer.Stop();
        mApplication.Quit();
      }
    }
  }

private:
  Application&   mApplication;
  ImageView      mImageViews[NUMBER_OF_IMAGES];
  int            mImageIndex;
  Timer          mTimer;
};

DALI_VISUAL_TEST( ResourceUploadingTest, OnInit )
