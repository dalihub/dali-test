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
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali-toolkit/dali-toolkit.h>
#include <dali-toolkit/devel-api/image-loader/texture-manager.h>
#include <dali-toolkit/devel-api/visuals/image-visual-properties-devel.h>
#include <dali-toolkit/devel-api/visual-factory/visual-factory.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{
// Resource for drawing
const std::string JPG_FILENAME = TEST_IMAGE_DIR "native-image-visual/gallery-medium-16.jpg";

// Resource for visual comparison
const std::string IMAGE_FILE   = TEST_IMAGE_DIR "native-image-visual/expected-result.png";

}  // namespace

/**
 * @brief This is to test the functionality of native image and image visual
 */
class NativeImageVisualTest: public VisualTest
{
public:

  NativeImageVisualTest( Application& application )
    : mApplication( application )
  {
  }

  ~NativeImageVisualTest()
  {
  }

  void OnInit( Application& application )
  {
    Window defaultWindow = mApplication.GetWindow();
    defaultWindow.SetBackgroundColor(Color::WHITE);
    CreateContentAreas();
    SetupNativeImage();

    // Start the test
    mTimer = Timer::New(1000);
    mTimer.TickSignal().Connect(this, &NativeImageVisualTest::OnTick);
    mTimer.Start();
  }

private:

  bool OnTick()
  {
    Window window = mApplication.GetWindow();
    CaptureWindow( window );
    return false;
  }

  void PostRender()
  {
    DALI_ASSERT_ALWAYS( CheckImage( IMAGE_FILE ) ); // should be identical
    mApplication.Quit();
  }

  void CreateContentAreas()
  {
    Window  window     = mApplication.GetWindow();
    Vector2 windowSize = window.GetSize();

    float contentHeight(windowSize.y / 2.0f);

    mTopContentArea = Actor::New();
    mTopContentArea[Actor::Property::SIZE] = Vector2(windowSize.x, contentHeight);
    mTopContentArea[Actor::Property::PARENT_ORIGIN] = ParentOrigin::TOP_CENTER;
    mTopContentArea[Actor::Property::ANCHOR_POINT] = AnchorPoint::TOP_CENTER;
    window.Add(mTopContentArea);

    mBottomContentArea = Actor::New();
    mBottomContentArea[Actor::Property::SIZE] = Vector2(windowSize.x, contentHeight);
    mBottomContentArea[Actor::Property::PARENT_ORIGIN] = ParentOrigin::BOTTOM_CENTER;
    mBottomContentArea[Actor::Property::ANCHOR_POINT] = AnchorPoint::BOTTOM_CENTER;
    window.Add(mBottomContentArea);

    mSourceActor = ImageView::New(JPG_FILENAME);
    mSourceActor[Actor::Property::PARENT_ORIGIN] = ParentOrigin::CENTER;
    mSourceActor[Actor::Property::ANCHOR_POINT] = AnchorPoint::CENTER;
    mTopContentArea.Add(mSourceActor);

    TextLabel textLabel1 = TextLabel::New("Source Image");
    textLabel1[Actor::Property::PARENT_ORIGIN] = ParentOrigin::TOP_CENTER;
    textLabel1[Actor::Property::ANCHOR_POINT] = AnchorPoint::TOP_CENTER;
    mTopContentArea.Add(textLabel1);

    TextLabel textLabel2 = TextLabel::New("Native Image");
    textLabel2[Actor::Property::PARENT_ORIGIN] = ParentOrigin::TOP_CENTER;
    textLabel2[Actor::Property::ANCHOR_POINT] = AnchorPoint::TOP_CENTER;
    textLabel2[Actor::Property::DRAW_MODE] = DrawMode::OVERLAY_2D;
    mBottomContentArea.Add(textLabel2);
  }

  void SetupNativeImage()
  {
    Window  window     = mApplication.GetWindow();
    Vector2 windowSize = window.GetSize();

    float   contentHeight(windowSize.y / 2.0f);
    Vector2 imageSize(windowSize.x, contentHeight);

    // Create a native image
    mNativeImageSource = NativeImageSource::New(imageSize.width, imageSize.height, NativeImageSource::COLOR_DEPTH_DEFAULT);
    mNativeTexture     = Texture::New(*mNativeImageSource);
    mNativeTextureUrl  = Toolkit::TextureManager::AddTexture(mNativeTexture);

    // Use native image as a backing for framebuffer
    mFrameBuffer = FrameBuffer::New(mNativeTexture.GetWidth(), mNativeTexture.GetHeight(), FrameBuffer::Attachment::NONE);
    mFrameBuffer.AttachColorTexture(mNativeTexture);

    mCameraActor = CameraActor::New(imageSize);
    mCameraActor[Actor::Property::PARENT_ORIGIN] = ParentOrigin::CENTER;
    mCameraActor[Actor::Property::ANCHOR_POINT] = AnchorPoint::CENTER;
    mTopContentArea.Add(mCameraActor);

    // Setup render task to render into framebuffer (and thus, native image)
    RenderTaskList taskList = window.GetRenderTaskList();
    mOffscreenRenderTask    = taskList.CreateTask();
    mOffscreenRenderTask.SetSourceActor(mSourceActor);
    mOffscreenRenderTask.SetClearColor(Color::WHITE);
    mOffscreenRenderTask.SetClearEnabled(true);
    mOffscreenRenderTask.SetCameraActor(mCameraActor);
    mOffscreenRenderTask.GetCameraActor().SetInvertYAxis(true);
    mOffscreenRenderTask.SetFrameBuffer(mFrameBuffer);
    mOffscreenRenderTask.SetRefreshRate(RenderTask::REFRESH_ALWAYS);

    // Setup image view of the native image in the bottom area
    mImageView = ImageView::New(mNativeTextureUrl);
    mImageView[Actor::Property::PARENT_ORIGIN] = ParentOrigin::CENTER;
    mImageView[Actor::Property::ANCHOR_POINT] = AnchorPoint::CENTER;
    mBottomContentArea.Add(mImageView);
  }

private:
  Application&         mApplication;
  std::string          mNativeTextureUrl;
  Texture              mNativeTexture;
  Timer                mTimer;
  Actor                mTopContentArea;
  Actor                mBottomContentArea;
  Actor                mSourceActor;
  RenderTask           mOffscreenRenderTask;
  FrameBuffer          mFrameBuffer;
  CameraActor          mCameraActor;
  ImageView            mImageView;
  NativeImageSourcePtr mNativeImageSource;
};

DALI_VISUAL_TEST_WITH_WINDOW_SIZE( NativeImageVisualTest, OnInit, 1024, 960 )
