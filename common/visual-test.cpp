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

// INTERNAL INCLUDES
#include "visual-test.h"
#include "image-util.h"

using namespace Dali;

const char* gTempFilename="/tmp/temp.png";


/**
 * @brief Constructor.
 */
VisualTest::VisualTest()
: mNativeImageSourcePtr( nullptr ),
  mWindow()
{
}

void VisualTest::SetupNativeImage( Dali::Window window )
{
  if( !mOffscreenRenderTask || window.GetRootLayer() != mWindow.GetHandle() )
  {
    Layer rootLayer = window.GetRootLayer();
    mWindow = rootLayer;

    mNativeImageSourcePtr = NativeImageSource::New( window.GetSize().GetWidth(), window.GetSize().GetHeight(), NativeImageSource::COLOR_DEPTH_DEFAULT );
    mNativeTexture = Texture::New( *mNativeImageSourcePtr );

    mFrameBuffer = FrameBuffer::New( mNativeTexture.GetWidth(), mNativeTexture.GetHeight(), FrameBuffer::Attachment::NONE );
    mFrameBuffer.AttachColorTexture( mNativeTexture );

    mCameraActor = CameraActor::New( Vector2(window.GetSize().GetWidth(), window.GetSize().GetHeight()) );
    mCameraActor.SetProperty( Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER );
    mCameraActor.SetProperty( Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER );
    window.Add( mCameraActor );

    RenderTaskList taskList = window.GetRenderTaskList();
    if ( mOffscreenRenderTask )
    {
      taskList.RemoveTask( mOffscreenRenderTask );
    }
    mOffscreenRenderTask = taskList.CreateTask();
    mOffscreenRenderTask.SetSourceActor( window.GetRootLayer() );
    mOffscreenRenderTask.SetClearColor( window.GetBackgroundColor() );
    mOffscreenRenderTask.SetClearEnabled( true );
    mOffscreenRenderTask.SetCameraActor( mCameraActor );
    mOffscreenRenderTask.GetCameraActor().SetInvertYAxis( false );
    mOffscreenRenderTask.SetFrameBuffer( mFrameBuffer );
  }

  mOffscreenRenderTask.SetRefreshRate( RenderTask::REFRESH_ONCE );
}

void VisualTest::CaptureWindow( Dali::Window window )
{
  SetupNativeImage( window );

  mOffscreenRenderTask.FinishedSignal().Connect( this, &VisualTest::OnOffscreenRenderFinished );
}

void VisualTest::OnOffscreenRenderFinished( RenderTask& task )
{
  task.FinishedSignal().Disconnect( this, &VisualTest::OnOffscreenRenderFinished );

  PostRender();
}

bool VisualTest::CheckImage( const std::string fileName, const float similarityThreshold, const Rect<uint16_t>& areaToCompare )
{
  bool success = false;

  // Compare the image in the given area
  if ( mNativeImageSourcePtr->EncodeToFile( gTempFilename ) )
  {
    if ( CompareImageFile( fileName, gTempFilename, similarityThreshold, areaToCompare ) )
    {
      success = true;
    }
  }

  return success;
}

bool VisualTest::CompareImageFile( const std::string fileName1, const std::string fileName2, const float similarityThreshold, const Rect<uint16_t>& areaToCompare )
{
  cv::Scalar similarity;

  // Load the images
  cv::Mat matrixImg1 = cv::imread(fileName1);
  cv::Mat matrixImg2 = cv::imread(fileName2);

  if ( areaToCompare != Rect<uint16_t>( 0u, 0u, 0u, 0u ) )
  {
    // Crop the images
    cv::Rect roi;
    roi.x = areaToCompare.x;
    roi.y = areaToCompare.y;
    roi.width = areaToCompare.width;
    roi.height = areaToCompare.height;

    cv::Mat croppedMatrixImg1 = matrixImg1(roi);
    cv::Mat croppedMatrixImg2 = matrixImg2(roi);

    similarity = ImageUtil::CalculateSSIM( croppedMatrixImg1, croppedMatrixImg2 );
  }
  else
  {
    similarity = ImageUtil::CalculateSSIM( matrixImg1, matrixImg2 );
  }

  // Check whether SSIM for all the three channels (RGB) are above the threshold
  return ( similarity.val[0] >= similarityThreshold && similarity.val[1] >= similarityThreshold && similarity.val[2] >= similarityThreshold );
}
