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
#include <dali-scene3d/dali-scene3d.h>
#include <dali-toolkit/dali-toolkit.h>
#include <dali-toolkit/devel-api/image-loader/texture-manager.h>
#include <dali/dali.h>
#include <dali/devel-api/rendering/frame-buffer-devel.h>
#include <dali/integration-api/debug.h>
#include <cstdio>
#include <string>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;
using namespace Dali::Scene3D::Loader;

namespace
{
const Vector3      CAMERA_DEFAULT_POSITION(0.0f, 0.0f, 3.5f);

const std::string RESOURCE_TYPE_DIRS[]{
  TEST_SCENE_DIR "environments/",
  TEST_SCENE_DIR "shaders/",
  TEST_SCENE_DIR "models/",
  TEST_SCENE_DIR "images/",
};

const int WINDOW_WIDTH(1800);
const int WINDOW_HEIGHT(1200);

struct ModelDetails
{
  std::string name;
  Vector3     position;
  float       scale;
  Quaternion  orientation;
};

const ModelDetails MODELS[] =
  {
    {"AntiqueCamera.usdz", Vector3(0.0f, -1.2f, 0.0f), 0.0035f, Quaternion(Radian(Degree(15.0f)), Vector3::YAXIS)},
    {"BarramundiFish.usdz", Vector3(0.0f, -1.0f, 0.0f), 0.08f, Quaternion(Radian(Degree(-90.0f)), Vector3::YAXIS)},
    {"CesiumMilkTruck.usdz", Vector3(0.0f, -0.5f, 0.0f), 0.005f, Quaternion(Radian(Degree(-40.0f)), Vector3::YAXIS)},
    {"Corset.usdz", Vector3(0.0f, -0.7f, 0.0f), 0.3f, Quaternion(Radian(Degree(30.0f)), Vector3::XAXIS)},
    {"WaterBottle.usdz", Vector3(0.0f, 0.0f, 0.0f), 0.08f, Quaternion(Radian(Degree(30.0f)), Vector3::XAXIS)},
};

const int MODELS_COUNT = sizeof(MODELS) / sizeof(MODELS[0]);

static int gTestStep = -1;

} // namespace

/**
 * @brief Tests USD model loading functionality
 */
class UsdModelTest : public VisualTest
{
public:
  UsdModelTest(Application& application)
  : mApplication(application)
  {
  }

  void OnInit(Application& application)
  {
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
    mSceneRender     = renderTasks.CreateTask();
    mSceneRender.SetCameraActor(mSceneCamera);
    mSceneRender.SetSourceActor(mSceneLayer);
    mSceneRender.SetClearColor(Color::WHITE);
    mSceneRender.SetClearEnabled(true);
    mSceneRender.SetRefreshRate(RenderTask::REFRESH_ALWAYS);
    mSceneRender.SetExclusive(true);

    mSceneFBO = FrameBuffer::New(WINDOW_WIDTH, WINDOW_HEIGHT, FrameBuffer::Attachment::COLOR_DEPTH);
    Texture depthTexture =
      Texture::New(TextureType::TEXTURE_2D, Pixel::DEPTH_FLOAT, WINDOW_WIDTH, WINDOW_HEIGHT);
    DevelFrameBuffer::AttachDepthTexture(mSceneFBO, depthTexture);
    mSceneRender.SetFrameBuffer(mSceneFBO);

    // Now render the color attachment to the main tree, but because fbo is
    // "upside-down" compared to loaded images we need to invert the image.
    auto offscreen                                 = mSceneFBO.GetColorTexture();
    auto offscreenUrl                              = Toolkit::TextureManager::AddTexture(offscreen);
    auto offscreenImage                            = Toolkit::ImageView::New(offscreenUrl);
    offscreenImage[Actor::Property::PARENT_ORIGIN] = ParentOrigin::CENTER;
    offscreenImage[Actor::Property::ANCHOR_POINT]  = AnchorPoint::CENTER;
    offscreenImage.SetResizePolicy(ResizePolicy::FILL_TO_PARENT,
                                   Dimension::ALL_DIMENSIONS);
    offscreenImage[Actor::Property::SCALE_Y] = -1; // Invert the image.
    window.Add(offscreenImage);

    // Start the test
    PrepareNextTest();
  }

private:
  void PrepareNextTest()
  {
    gTestStep++;

    const ModelDetails& nextModel = MODELS[gTestStep];

    if(mScene)
    {
      mSceneLayer.Remove(mScene);
    }

    mScene = LoadModel(nextModel.name, mSceneCamera);
    mScene.SetProperty(Dali::Actor::Property::SCALE, nextModel.scale);
    mScene.SetProperty(Dali::Actor::Property::POSITION, nextModel.position);
    mScene.SetProperty(Dali::Actor::Property::ORIENTATION, nextModel.orientation);

    mSceneLayer.Add(mScene);

    CaptureWindowAfterFrameRendered(mApplication.GetWindow());
  }

  void OnFinishedAnimation(Animation& animation)
  {
    CaptureWindowAfterFrameRendered(mApplication.GetWindow());
  }

  void PostRender(std::string outputFile, bool success)
  {
    CompareImageFile(TEST_IMAGE_DIR "usd-model/expected-result-" + std::to_string(gTestStep + 1) + ".png", outputFile, 0.98f);
    if(gTestStep + 1 < MODELS_COUNT)
    {
      PrepareNextTest();
    }
    else
    {
      mApplication.Quit();
    }
  }

  Actor LoadModel(std::string sceneName, CameraActor camera)
  {
    ResourceBundle::PathProvider pathProvider = [](ResourceType::Value type) {
      return RESOURCE_TYPE_DIRS[type];
    };

    auto sceneFile = pathProvider(ResourceType::Mesh) + sceneName;

    ResourceBundle                        resources;
    SceneDefinition                       scene;
    SceneMetadata                         metaData;
    std::vector<AnimationGroupDefinition> animGroups;
    std::vector<CameraParameters>         cameraParameters;
    std::vector<LightParameters>          lights;
    std::vector<AnimationDefinition>      animations;

    LoadResult output{resources, scene, metaData, animations, animGroups, cameraParameters, lights};

    Dali::Scene3D::Loader::ModelLoader modelLoader(sceneFile, pathProvider(ResourceType::Mesh) + "/", output);
    modelLoader.LoadModel(pathProvider, true);

    resources.GenerateResources();

    if(cameraParameters.empty())
    {
      cameraParameters.push_back(CameraParameters());
      cameraParameters[0].matrix.SetTranslation(CAMERA_DEFAULT_POSITION);
    }
    cameraParameters[0].ConfigureCamera(camera);
    SetActorCentered(camera);

    Scene3D::Loader::ShaderManagerPtr shaderManager  = new Scene3D::Loader::ShaderManager();
    ViewProjection                    viewProjection = cameraParameters[0].GetViewProjection();
    Transforms                        xforms{
      MatrixStack{},
      viewProjection};
    NodeDefinition::CreateParams nodeParams{
      resources,
      xforms,
      shaderManager,
      {},
      {},
      {}};
    Customization::Choices choices;

    Actor root = Actor::New();
    SetActorCentered(root);

    for(auto iRoot : scene.GetRoots())
    {
      auto resourceRefs = resources.CreateRefCounter();
      scene.CountResourceRefs(iRoot, choices, resourceRefs);
      resources.mReferenceCounts = std::move(resourceRefs);
      resources.CountEnvironmentReferences();
      resources.LoadResources(pathProvider);

      if(auto actor = scene.CreateNodes(iRoot, choices, nodeParams))
      {
        scene.ConfigureSkinningShaders(resources, actor, std::move(nodeParams.mSkinnables));

        DALI_ASSERT_ALWAYS(scene.ConfigureBlendshapeShaders(
          resources, actor, std::move(nodeParams.mBlendshapeRequests)));

        scene.ApplyConstraints(actor, std::move(nodeParams.mConstrainables));

        root.Add(actor);
      }
    }

    return root;
  }

private:
  Application& mApplication;
  CameraActor  mSceneCamera;
  Actor        mScene;
  Layer        mSceneLayer;
  RenderTask   mSceneRender;
  FrameBuffer  mSceneFBO;
  Animation    mAnimation;
};

DALI_VISUAL_TEST_WITH_WINDOW_SIZE(UsdModelTest, OnInit, WINDOW_WIDTH, WINDOW_HEIGHT)
