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

// EXTERNAL INCLUDES
#include <cstdio>
#include <dali-scene3d/public-api/loader/animation-definition.h>
#include <dali-scene3d/public-api/loader/camera-parameters.h>
#include <dali-scene3d/public-api/loader/dli-loader.h>
#include <dali-scene3d/public-api/loader/light-parameters.h>
#include <dali-scene3d/public-api/loader/load-result.h>
#include <dali-scene3d/public-api/loader/node-definition.h>
#include <dali-scene3d/public-api/loader/scene-definition.h>
#include <dali-toolkit/dali-toolkit.h>
#include <dali-toolkit/devel-api/image-loader/texture-manager.h>
#include <dali/dali.h>
#include <dali/devel-api/rendering/frame-buffer-devel.h>
#include <string>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;
using namespace Dali::Scene3D::Loader;

namespace {

const Vector3 CAMERA_DEFAULT_POSITION(0.0f, 0.0f, 3.5f);
const unsigned int DEFAULT_DELAY_TIME = 100;

const std::string RESOURCE_TYPE_DIRS[]{
    TEST_SCENE_DIR "environments/",
    TEST_SCENE_DIR "shaders/",
    TEST_SCENE_DIR "models/",
    TEST_SCENE_DIR "images/",
};

const std::string FIRST_IMAGE_FILE =
    TEST_IMAGE_DIR "scene3d/expected-result-1.png";
const std::string SECOND_IMAGE_FILE =
    TEST_IMAGE_DIR "scene3d/expected-result-2.png";
const std::string THIRD_IMAGE_FILE =
    TEST_IMAGE_DIR "scene3d/expected-result-3.png";
const std::string FOURTH_IMAGE_FILE =
    TEST_IMAGE_DIR "scene3d/expected-result-4.png";
const std::string FIFTH_IMAGE_FILE =
    TEST_IMAGE_DIR "scene3d/expected-result-5.png";

const int WINDOW_WIDTH(480);
const int WINDOW_HEIGHT(800);

enum TestStep {
  LOAD_FIRST_SCENE_AND_CAPTURE,
  FIRST_SCENE_ANIMATION,
  FIRST_SCENE_SECOND_CAPTURE,
  LOAD_SECOND_SCENE_AND_CAPTURE,
  SECOND_SCENE_ANIMATION,
  SECOND_SCENE_SECOND_CAPTURE,
  LOAD_THIRD_SCENE,
  THIRD_SCENE_CAPTURE,
  NUMBER_OF_STEPS
};

static int gTestStep = -1;

} // namespace

/**
 * @brief Tests scene loading functionality
 *
 * This is made difficult by the native image renderer rendering upside down.
 * Consequently the actor tree looks like:
 *
 * Window Root Layer
 *   +
 *   +---- Scene Layer
 *   |     + --- SceneCamera
 *   |     + Scene
 *   +
 *
 */
class Scene3DTest : public VisualTest {
public:
  Scene3DTest(Application &application) : mApplication(application) {}

  void OnInit(Application &application) {
    Dali::Window window = mApplication.GetWindow();
    window.SetBackgroundColor(Color::WHITE);
    window.GetRootLayer().SetProperty(Layer::Property::BEHAVIOR,
                                      Layer::LAYER_3D);

    // Create a custom layer for rendering a 3D scene (depth testing is enabled
    // automatically)
    mSceneLayer = Layer::New();
    mSceneLayer.SetProperty(Actor::Property::PARENT_ORIGIN,
                            ParentOrigin::CENTER);
    mSceneLayer.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER);
    mSceneLayer.SetResizePolicy(ResizePolicy::FILL_TO_PARENT,
                                Dimension::ALL_DIMENSIONS);
    mSceneLayer[Layer::Property::BEHAVIOR] = Layer::LAYER_3D;
    window.Add(mSceneLayer);

    // Create a custom camera (will be modified by Scene3d::Loader, below)
    mSceneCamera = CameraActor::New();
    mSceneLayer.Add(mSceneCamera);

    // Create a custom render task that _exclusively_ renders to a framebuffer
    // with a depth attachment.
    auto renderTasks = window.GetRenderTaskList();
    mSceneRender = renderTasks.CreateTask();
    mSceneRender.SetCameraActor(mSceneCamera);
    mSceneRender.SetSourceActor(mSceneLayer);
    mSceneRender.SetClearColor(Color::WHITE);
    mSceneRender.SetClearEnabled(true);
    mSceneRender.SetRefreshRate(RenderTask::REFRESH_ALWAYS);
    mSceneRender.SetExclusive(true);

    mSceneFBO = FrameBuffer::New(WINDOW_WIDTH, WINDOW_HEIGHT,
                                 FrameBuffer::Attachment::COLOR_DEPTH);
    Texture depthTexture =
        Texture::New(TextureType::TEXTURE_2D, Pixel::DEPTH_FLOAT, WINDOW_WIDTH,
                     WINDOW_HEIGHT);
    DevelFrameBuffer::AttachDepthTexture(mSceneFBO, depthTexture);
    mSceneRender.SetFrameBuffer(mSceneFBO);

    // Now render the color attachment to the main tree, but because fbo is
    // "upside-down" compared to loaded images we need to invert the image.
    auto offscreen = mSceneFBO.GetColorTexture();
    auto offscreenUrl = Toolkit::TextureManager::AddTexture(offscreen);
    auto offscreenImage = Toolkit::ImageView::New(offscreenUrl);
    offscreenImage[Actor::Property::PARENT_ORIGIN] = ParentOrigin::CENTER;
    offscreenImage[Actor::Property::ANCHOR_POINT] = AnchorPoint::CENTER;
    offscreenImage.SetResizePolicy(ResizePolicy::FILL_TO_PARENT,
                                   Dimension::ALL_DIMENSIONS);
    offscreenImage[Actor::Property::SCALE_Y] = -1; // Invert the image.
    window.Add(offscreenImage);

    // Start the test
    gTestStep++;
    PerformTest();
  }

private:
  Actor LoadScene(std::string sceneName, CameraActor camera) {
    ResourceBundle::PathProvider pathProvider = [](ResourceType::Value type) {
      return RESOURCE_TYPE_DIRS[type];
    };

    auto path = pathProvider(ResourceType::Mesh) + sceneName;

    ResourceBundle resources;
    SceneDefinition scene;
    SceneMetadata metaData;
    std::vector<AnimationGroupDefinition> animGroups;
    std::vector<CameraParameters> cameraParameters;
    std::vector<LightParameters> lights;
    std::vector<AnimationDefinition> animations;

    DliLoader loader;
    DliLoader::InputParams input{
        pathProvider(ResourceType::Mesh), nullptr, {}, {}, nullptr,
    };
    LoadResult output{resources, scene, metaData, animations, animGroups,
                      cameraParameters, lights};
    DliLoader::LoadParams loadParams{input, output};
    DALI_ASSERT_ALWAYS(loader.LoadScene(path, loadParams));

    if (cameraParameters.empty()) {
      cameraParameters.push_back(CameraParameters());
      cameraParameters[0].matrix.SetTranslation(CAMERA_DEFAULT_POSITION);
    }

    cameraParameters[0].ConfigureCamera(camera);
    SetActorCentered(camera);

    ViewProjection viewProjection = cameraParameters[0].GetViewProjection();
    Transforms xforms{MatrixStack{}, viewProjection};
    NodeDefinition::CreateParams nodeParams{
        resources,
        xforms,
    };
    Customization::Choices choices;

    Actor sceneRoot = Actor::New();
    SetActorCentered(sceneRoot);

    for (auto root : scene.GetRoots()) {
      auto resourceRefs = resources.CreateRefCounter();
      scene.CountResourceRefs(root, choices, resourceRefs);
      resources.CountEnvironmentReferences(resourceRefs);

      resources.LoadResources(resourceRefs, pathProvider);

      if (auto actor = scene.CreateNodes(root, choices, nodeParams)) {
        scene.ConfigureSkeletonJoints(root, resources.mSkeletons, actor);
        scene.ConfigureSkinningShaders(resources, actor,
                                       std::move(nodeParams.mSkinnables));

        DALI_ASSERT_ALWAYS(scene.ConfigureBlendshapeShaders(
            resources, actor, std::move(nodeParams.mBlendshapeRequests)));

        scene.ApplyConstraints(actor, std::move(nodeParams.mConstrainables));

        sceneRoot.Add(actor);
      }
    }

    if (!animations.empty()) {
      auto getActor = [&sceneRoot](const Scene3D::Loader::AnimatedProperty& property) {
      return sceneRoot.FindChildByName(property.mNodeName);
      };

      mAnimation = animations[0].ReAnimate(getActor);
      mAnimation.SetLooping(false);
    } else {
      mAnimation.Reset();
    }

    return sceneRoot;
  }

  void WaitForNextTest(unsigned int milliSecond) {
    gTestStep++;

    if (milliSecond == 0u) {
      PerformTest();
    } else {
      mTimer = Timer::New(milliSecond); // ms
      mTimer.TickSignal().Connect(this, &Scene3DTest::OnTimer);
      mTimer.Start();
    }
  }

  bool OnTimer() {
    PerformTest();
    return false;
  }

  void PerformTest() {
    Window window = mApplication.GetWindow();

    unsigned int delay = 0u;
    if (mAnimation) {
      delay = mAnimation.GetDuration() * 1000 + DEFAULT_DELAY_TIME;
    }

    switch (gTestStep) {
    case LOAD_FIRST_SCENE_AND_CAPTURE: {
      mScene = LoadScene("exercise.dli", mSceneCamera);
      mSceneLayer.Add(mScene);

      CaptureWindow(window);
      break;
    }
    case FIRST_SCENE_ANIMATION: {
      if (mAnimation) {
        mAnimation.Play();
      }

      WaitForNextTest(delay);

      break;
    }
    case FIRST_SCENE_SECOND_CAPTURE: {
      CaptureWindow(window, mSceneCamera);
      break;
    }
    case LOAD_SECOND_SCENE_AND_CAPTURE: {
      UnparentAndReset(mScene);
      mScene = LoadScene("robot.dli", mSceneCamera);
      mSceneLayer.Add(mScene);

      CaptureWindow(window);

      break;
    }
    case SECOND_SCENE_ANIMATION: {
      if (mAnimation) {
        mAnimation.Play();
      }

      WaitForNextTest(delay);

      break;
    }
    case SECOND_SCENE_SECOND_CAPTURE: {
      CaptureWindow(window, mSceneCamera);
      break;
    }
    case LOAD_THIRD_SCENE: {
      UnparentAndReset(mScene);
      mScene = LoadScene("beer.dli", mSceneCamera);
      mSceneLayer.Add(mScene);

      WaitForNextTest(DEFAULT_DELAY_TIME * 2);

      break;
    }
    case THIRD_SCENE_CAPTURE: {
      CaptureWindow(window);
      break;
    }
    default:
      break;
    }
  }

  void PostRender() {
    if (gTestStep == LOAD_FIRST_SCENE_AND_CAPTURE) {
      if (CheckImage(FIRST_IMAGE_FILE, 0.98f)) {
        WaitForNextTest(DEFAULT_DELAY_TIME);
      } else {
        mApplication.Quit();
      }
    } else if (gTestStep == FIRST_SCENE_SECOND_CAPTURE) {
      if (CheckImage(SECOND_IMAGE_FILE, 0.98f)) {
        WaitForNextTest(DEFAULT_DELAY_TIME);
      } else {
        mApplication.Quit();
      }
    } else if (gTestStep == LOAD_SECOND_SCENE_AND_CAPTURE) {
      if (CheckImage(THIRD_IMAGE_FILE, 0.99f)) {
        WaitForNextTest(DEFAULT_DELAY_TIME);
      } else {
        mApplication.Quit();
      }
    } else if (gTestStep == SECOND_SCENE_SECOND_CAPTURE) {
      if (CheckImage(FOURTH_IMAGE_FILE, 0.99f)) {
        WaitForNextTest(DEFAULT_DELAY_TIME);
      } else {
        mApplication.Quit();
      }
    } else if (gTestStep == THIRD_SCENE_CAPTURE) {
      CheckImage(FIFTH_IMAGE_FILE, 0.97f);

      // The last check has been done, so we can quit the test
      mApplication.Quit();
    }
  }

private:
  Application &mApplication;
  Timer mTimer;
  CameraActor mSceneCamera;
  Actor mScene;
  Layer mSceneLayer;
  RenderTask mSceneRender;
  FrameBuffer mSceneFBO;
  Animation mAnimation;
};

DALI_VISUAL_TEST_WITH_WINDOW_SIZE(Scene3DTest, OnInit, WINDOW_WIDTH,
                                  WINDOW_HEIGHT)
