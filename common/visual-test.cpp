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

// INTERNAL INCLUDES
#include "visual-test.h"
#include "image-util.h"

// EXTERNAL INCLUDES
#include <Magick++.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <cerrno>

using namespace Dali;

const char* gTempFilename="/tmp/temp.png";
const char* gVirtualFramebuffer="/var/tmp/Xvfb_screen0";
bool gFB=false;
int gExitValue=1;
int gImageNumber=1;


/**
 * @brief Constructor.
 */
VisualTest::VisualTest()
: mWindow()
{
}

void VisualTest::SetupNativeImage( Dali::Window window, Dali::CameraActor customCamera )
{
  if( !mOffscreenRenderTask || window.GetRootLayer() != mWindow.GetHandle() )
  {
    Layer rootLayer = window.GetRootLayer();
    mWindow = rootLayer;

    mNativeImageSourcePtr = NativeImageSource::New( window.GetSize().GetWidth(), window.GetSize().GetHeight(), NativeImageSource::COLOR_DEPTH_DEFAULT );
    mNativeTexture = Texture::New( *mNativeImageSourcePtr );

    mFrameBuffer = FrameBuffer::New( mNativeTexture.GetWidth(), mNativeTexture.GetHeight(), FrameBuffer::Attachment::STENCIL );
    mFrameBuffer.AttachColorTexture( mNativeTexture );

    RenderTaskList taskList = window.GetRenderTaskList();
    if ( mOffscreenRenderTask )
    {
      taskList.RemoveTask( mOffscreenRenderTask );
    }
    mOffscreenRenderTask = taskList.CreateTask();
    mOffscreenRenderTask.SetSourceActor( window.GetRootLayer() );
    mOffscreenRenderTask.SetClearColor( window.GetBackgroundColor() );
    mOffscreenRenderTask.SetClearEnabled( true );
    mOffscreenRenderTask.SetFrameBuffer( mFrameBuffer );

    if (customCamera)
    {
      mOffscreenRenderTask.SetCameraActor( customCamera );
    }
    else
    {
      mCameraActor = CameraActor::New( Vector2(window.GetSize().GetWidth(), window.GetSize().GetHeight()) );
      mCameraActor.SetProperty( Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER );
      mCameraActor.SetProperty( Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER );
      mCameraActor.SetInvertYAxis( true );
      window.Add( mCameraActor );

      mOffscreenRenderTask.SetCameraActor( mCameraActor );
    }
  }

  mOffscreenRenderTask.SetRefreshRate( RenderTask::REFRESH_ONCE );
}

void VisualTest::CaptureWindow( Dali::Window window, Dali::CameraActor customCamera )
{
  if(gFB)
  {
    RenderTask renderTask = window.GetRenderTaskList().GetTask(0);
    renderTask.SetRefreshRate(RenderTask::REFRESH_ONCE);
    renderTask.FinishedSignal().Connect( this, &VisualTest::OnOffscreenRenderFinished );
  }
  else
  {
    SetupNativeImage( window, customCamera );
    mOffscreenRenderTask.FinishedSignal().Connect( this, &VisualTest::OnOffscreenRenderFinished );
  }
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
  char* imageName;

  // Ensure there's a directory to write to:
  struct stat statBuffer;
  errno=0;
  if(-1 == stat(TMPDIR, &statBuffer))
  {
    if(errno == ENOENT)
    {
      if(-1 == mkdir(TMPDIR, 0755))
      {
        fprintf(stderr, "%s\n", strerror(errno));
      }
    }
    else
    {
      fprintf(stderr, "%s\n", strerror(errno));
    }
  }

  asprintf(&imageName, "%s%02d.png", gTempFilename, gImageNumber);
  gImageNumber++;

  if(gFB)
  {
    Magick::Image image;
    image.magick("xwd");
    image.read(gVirtualFramebuffer);
    image.magick("png");
    image.write(imageName);
    success=true;
  }
  else
  {
    success = mNativeImageSourcePtr->EncodeToFile( imageName );
  }

  if(success)
  {
    success = CompareImageFile( fileName, imageName, similarityThreshold, areaToCompare );
  }

  free(imageName);

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
  bool passed = ( similarity.val[0] >= similarityThreshold && similarity.val[1] >= similarityThreshold && similarity.val[2] >= similarityThreshold );

  printf("Test similarity: R:%f G:%f B:%f\n"
         "Passed threshold of %f: %s\n",
         100.0f*similarity.val[0], 100.0f*similarity.val[1], 100.0f*similarity.val[2],
         100.0f*similarityThreshold, passed?"TRUE":"FALSE");


  gExitValue = 33.3f*(similarity.val[0]+similarity.val[1]+similarity.val[2]);
  if(passed) gExitValue=0;

  return passed;
}
