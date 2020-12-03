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
#include <dali-toolkit/dali-toolkit.h>
#include <dali-scene-loader/public-api/animation-definition.h>
#include <dali-scene-loader/public-api/scene-definition.h>
#include <dali-scene-loader/public-api/node-definition.h>
#include <dali-scene-loader/public-api/camera-parameters.h>
#include <dali-scene-loader/public-api/light-parameters.h>
#include <dali-scene-loader/public-api/dli-loader.h>
#include <dali-scene-loader/public-api/load-result.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;
using namespace Dali::SceneLoader;

namespace
{

const Vector3 CAMERA_DEFAULT_POSITION(0.0f, 0.0f, 3.5f);
const unsigned int DEFAULT_DELAY_TIME = 100;

const std::string RESOURCE_TYPE_DIRS[]{
  TEST_SCENE_DIR "environments/",
  TEST_SCENE_DIR "shaders/",
  TEST_SCENE_DIR "models/",
  TEST_SCENE_DIR "images/",
};

const std::string FIRST_IMAGE_FILE = TEST_IMAGE_DIR "scene-loader/expected-result-1.png";
const std::string SECOND_IMAGE_FILE = TEST_IMAGE_DIR "scene-loader/expected-result-2.png";
const std::string THIRD_IMAGE_FILE = TEST_IMAGE_DIR "scene-loader/expected-result-3.png";
const std::string FOURTH_IMAGE_FILE = TEST_IMAGE_DIR "scene-loader/expected-result-4.png";
const std::string FIFTH_IMAGE_FILE = TEST_IMAGE_DIR "scene-loader/expected-result-5.png";

enum TestStep
{
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

}  // namespace

/**
 * @brief This is to test the functionality which allows the uploading of textures to the GPU
 * without rendering while the application is paused, and thus, have them available immediately
 * for rendering on resume.
 */
class SceneLoaderTest: public VisualTest
{
 public:

  SceneLoaderTest( Application& application )
    : mApplication( application )
  {
  }

  void OnInit( Application& application )
  {
    Dali::Window window = mApplication.GetWindow();
    window.SetBackgroundColor(Color::WHITE);

    // Create a custom camera
    mSceneCamera = CameraActor::New();
    window.Add(mSceneCamera);

    auto renderTasks = window.GetRenderTaskList();
    mSceneRender = renderTasks.GetTask(0);
    mSceneRender.SetCameraActor(mSceneCamera);

    // Start the test
    gTestStep++;
    PerformTest();
  }

private:

  void ConfigureCamera(const CameraParameters& params, CameraActor camera)
  {
    if (params.isPerspective)
    {
      camera.SetProjectionMode(Camera::PERSPECTIVE_PROJECTION);
      camera.SetNearClippingPlane(params.zNear);
      camera.SetFarClippingPlane(params.zFar);
      camera.SetFieldOfView(Radian(Degree(params.yFov)));
    }
    else
    {
      camera.SetProjectionMode(Camera::ORTHOGRAPHIC_PROJECTION);
      camera.SetOrthographicProjection(params.orthographicSize.x,
        params.orthographicSize.y,
        params.orthographicSize.z,
        params.orthographicSize.w,
        params.zNear,
        params.zFar);
    }

    // model
    Vector3 camTranslation;
    Vector3 camScale;
    Quaternion camOrientation;
    params.CalculateTransformComponents(camTranslation, camOrientation, camScale);

    SetActorCentered(camera);

    // Scene loader needs camera configuration with inverted Y axis, which is different
    // with other visual tests that use the default camera. Therefore, the captured image
    // will be encoded as upside down.
    camera.SetInvertYAxis(true);

    camera.SetProperty(Actor::Property::POSITION, camTranslation);
    camera.SetProperty(Actor::Property::ORIENTATION, camOrientation);
    camera.SetProperty(Actor::Property::SCALE, camScale);

    camOrientation.Conjugate();
  }

  Actor LoadScene(std::string sceneName, CameraActor camera)
  {
    ResourceBundle::PathProvider pathProvider = [](ResourceType::Value type)
    {
      return RESOURCE_TYPE_DIRS[type];
    };

    auto path = pathProvider(ResourceType::Mesh) + sceneName;

    ResourceBundle resources;
    SceneDefinition scene;
    std::vector<AnimationGroupDefinition> animGroups;
    std::vector<CameraParameters> cameraParameters;
    std::vector<LightParameters> lights;
    std::vector<AnimationDefinition> animations;

    DliLoader loader;
    DliLoader::InputParams input
    {
      pathProvider(ResourceType::Mesh),
      nullptr,
      {},
      {},
      nullptr,
    };
    LoadResult output
    {
      resources,
      scene,
      animations,
      animGroups,
      cameraParameters,
      lights
    };
    DliLoader::LoadParams loadParams{ input, output };
    DALI_ASSERT_ALWAYS(loader.LoadScene(path, loadParams));

    if (cameraParameters.empty())
    {
      cameraParameters.push_back(CameraParameters());
      cameraParameters[0].matrix.SetTranslation(CAMERA_DEFAULT_POSITION);
    }
    ConfigureCamera(cameraParameters[0], camera);

    ViewProjection viewProjection = cameraParameters[0].GetViewProjection();
    Transforms xforms
    {
      MatrixStack{},
      viewProjection
    };
    NodeDefinition::CreateParams nodeParams
    {
      resources,
      xforms,
    };
    Customization::Choices choices;

    Actor sceneRoot = Actor::New();
    SetActorCentered(sceneRoot);

    for (auto root : scene.GetRoots())
    {
      auto resourceRefs = resources.CreateRefCounter();
      scene.CountResourceRefs(root, choices, resourceRefs);
      resources.CountEnvironmentReferences(resourceRefs);

      resources.LoadResources(resourceRefs, pathProvider);

      if (auto actor = scene.CreateNodes(root, choices, nodeParams))
      {
        scene.ConfigureSkeletonJoints(root, resources.mSkeletons, actor);
        scene.ConfigureSkinningShaders(resources, actor, std::move(nodeParams.mSkinnables));

        DALI_ASSERT_ALWAYS(scene.ConfigureBlendshapeShaders(resources, actor, std::move(nodeParams.mBlendshapeRequests)));

        scene.ApplyConstraints(actor, std::move(nodeParams.mConstrainables));

        sceneRoot.Add(actor);
      }
    }

    if (!animations.empty())
    {
      auto getActor = [&sceneRoot](const std::string& name)
      {
        return sceneRoot.FindChildByName(name);
      };

      mAnimation = animations[0].ReAnimate(getActor);
      mAnimation.SetLooping(false);
    }
    else
    {
      mAnimation.Reset();
    }

    return sceneRoot;
  }

  void WaitForNextTest( unsigned int milliSecond )
  {
    gTestStep++;

    if ( milliSecond == 0u )
    {
      PerformTest();
    }
    else
    {
      mTimer = Timer::New( milliSecond ); // ms
      mTimer.TickSignal().Connect( this, &SceneLoaderTest::OnTimer);
      mTimer.Start();
    }
  }

  bool OnTimer()
  {
    PerformTest();
    return false;
  }

  void PerformTest()
  {
    Window window = mApplication.GetWindow();

    unsigned int delay = 0u;
    if (mAnimation)
    {
      delay = mAnimation.GetDuration() * 1000 + DEFAULT_DELAY_TIME;
    }

    switch (gTestStep)
    {
      case LOAD_FIRST_SCENE_AND_CAPTURE:
      {
        mScene = LoadScene("exercise.dli", mSceneCamera);
        mSceneRender.SetSourceActor(mScene);
        window.Add(mScene);

        CaptureWindow(window, mSceneCamera);

        break;
      }
      case FIRST_SCENE_ANIMATION:
      {
        if (mAnimation)
        {
          mAnimation.Play();
        }

        WaitForNextTest(delay);

        break;
      }
      case FIRST_SCENE_SECOND_CAPTURE:
      {
        CaptureWindow(window, mSceneCamera);
        break;
      }
      case LOAD_SECOND_SCENE_AND_CAPTURE:
      {
        mScene = LoadScene("robot.dli", mSceneCamera);
        mSceneRender.SetSourceActor(mScene);
        window.Add(mScene);

        CaptureWindow(window, mSceneCamera);

        break;
      }
      case SECOND_SCENE_ANIMATION:
      {
        if (mAnimation)
        {
          mAnimation.Play();
        }

        WaitForNextTest(delay);

        break;
      }
      case SECOND_SCENE_SECOND_CAPTURE:
      {
        CaptureWindow(window, mSceneCamera);
        break;
      }
      case LOAD_THIRD_SCENE:
      {
        mScene = LoadScene("beer.dli", mSceneCamera);
        mSceneRender.SetSourceActor(mScene);
        window.Add(mScene);

        WaitForNextTest(DEFAULT_DELAY_TIME);

        break;
      }
      case THIRD_SCENE_CAPTURE:
      {
        CaptureWindow(window, mSceneCamera);
        break;
      }
      default:
        break;
    }
  }

  void PostRender()
  {
    if (gTestStep == LOAD_FIRST_SCENE_AND_CAPTURE)
    {
      DALI_ASSERT_ALWAYS(CheckImage(FIRST_IMAGE_FILE, 0.99f)); // verify the similarity

      WaitForNextTest(DEFAULT_DELAY_TIME);
    }
    else if (gTestStep == FIRST_SCENE_SECOND_CAPTURE)
    {
      DALI_ASSERT_ALWAYS(CheckImage(SECOND_IMAGE_FILE, 0.99f)); // verify the similarity

      WaitForNextTest(DEFAULT_DELAY_TIME);
    }
    else if ( gTestStep == LOAD_SECOND_SCENE_AND_CAPTURE )
    {
      DALI_ASSERT_ALWAYS(CheckImage(THIRD_IMAGE_FILE, 0.99f)); // verify the similarity

      WaitForNextTest(DEFAULT_DELAY_TIME);
    }
    else if ( gTestStep == SECOND_SCENE_SECOND_CAPTURE )
    {
      DALI_ASSERT_ALWAYS(CheckImage(FOURTH_IMAGE_FILE, 0.99f)); // verify the similarity

      WaitForNextTest(DEFAULT_DELAY_TIME);
    }
    else if ( gTestStep == THIRD_SCENE_CAPTURE )
    {
      DALI_ASSERT_ALWAYS(CheckImage(FIFTH_IMAGE_FILE, 0.99f)); // verify the similarity

      // The last check has been done, so we can quit the test
      mApplication.Quit();
    }
  }

private:
  Application&      mApplication;
  Timer             mTimer;
  CameraActor       mSceneCamera;
  Actor             mScene;
  RenderTask        mSceneRender;
  Animation         mAnimation;
};

DALI_VISUAL_TEST( SceneLoaderTest, OnInit )

