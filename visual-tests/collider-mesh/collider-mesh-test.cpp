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
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/devel-api/actors/actor-devel.h>
#include <dali-scene3d/public-api/controls/scene-view/scene-view.h>
#include <dali-scene3d/public-api/controls/model/model.h>
#include <dali-scene3d/public-api/model-components/model-node.h>
#include <dali-scene3d/public-api/algorithm/navigation-mesh.h>
#include <dali-scene3d/public-api/loader/navigation-mesh-factory.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

// set macros to satisfy intellisense when outside of build system
#ifndef TEST_IMAGE_DIR
#define TEST_IMAGE_DIR
#endif

#ifndef TEST_RESOURCES_DIR
#define TEST_RESOURCES_DIR
#endif

namespace
{
const std::string IMAGE_FILE = TEST_IMAGE_DIR "collider-mesh/ColliderMeshTest0";



}  // namespace

/**
 * @brief This is to test the functionality which allows the uploading of textures to the GPU
 * without rendering while the application is paused, and thus, have them available immediately
 * for rendering on resume.
 */
class ColliderMeshTest : public VisualTest
{
 public:

  ColliderMeshTest( Application& application )
    : mApplication( application )
  {
  }

  void OnInit( Application& application )
  {
    Dali::Window window = mApplication.GetWindow();
    mWindow = window;
    window.SetBackgroundColor(Color::BLACK);

    const Vector2 windowSize = window.GetSize();

    // Create a SceneView
    Scene3D::SceneView sceneView = Handle::New<Scene3D::SceneView>({{Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER},
                                                  {Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER},
                                                  {Actor::Property::SIZE, windowSize}});

    mSceneView = sceneView;

    window.Add(sceneView);

    // Load the model and set IBL
    Scene3D::Model model = Scene3D::Model::New(TEST_RESOURCES_DIR "collider-mesh/floors2.gltf");
    model.SetProperties({
      {Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER},
      {Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER},
    });
    sceneView.Add(model);

    model.ResourceReadySignal().Connect(this, &ColliderMeshTest::LoadingReady );
    window.Add(sceneView);
  }

private:

  static bool OnModelHit(Scene3D::Model model, Scene3D::ModelNode node)
  {
    static const std::vector<std::string> NAMES = {"room0", "room1", "room2"};
    for(auto name : NAMES)
    {
      auto actor = model.FindChildModelNodeByName(name);
      actor.SetProperty(Actor::Property::COLOR, Color::WHITE);
    }
    node.SetProperty(Actor::Property::COLOR, Color::RED);
    return true;
  }


  void LoadingReady(Control control)
  {
    using Scene3D::Model;
    auto model = Model::DownCast(control);
    auto camera= model.GenerateCamera(0);

    mSceneView.AddCamera(camera);
    mSceneView.SelectCamera(mSceneView.GetCameraCount()-1);

    [[maybe_unused]] auto transform = Dali::DevelActor::GetWorldTransform(camera);

    // Disconnect signal to avoid calling it recursively!
    model.ResourceReadySignal().Disconnect(this, &ColliderMeshTest::LoadingReady );

    [[maybe_unused]] auto room0 = model.FindChildModelNodeByName("room0");
    [[maybe_unused]] auto room1 = model.FindChildModelNodeByName("room1");
    [[maybe_unused]] auto room2 = model.FindChildModelNodeByName("room2");

    auto collider0 = Scene3D::Loader::NavigationMeshFactory::CreateFromFile(TEST_RESOURCES_DIR "collider-mesh/room0.col");
    auto collider1 = Scene3D::Loader::NavigationMeshFactory::CreateFromFile(TEST_RESOURCES_DIR "collider-mesh/room1.col");
    auto collider2 = Scene3D::Loader::NavigationMeshFactory::CreateFromFile(TEST_RESOURCES_DIR "collider-mesh/room2.col");

    // unparent one node to simulate case of setting the collider mesh while node is not a part of model tree
    model.RemoveModelNode(room2);

    room0.SetColliderMesh(std::move(collider0));
    room1.SetColliderMesh(std::move(collider1));
    room2.SetColliderMesh(std::move(collider2));

    // put orphaned node back into the tree
    model.AddModelNode(room2);

    model.MeshHitSignal().Connect(this, &ColliderMeshTest::OnModelHit );
    PerformNextTest(1000);
  }

  bool OnUpdate()
  {
    if(mTestStep == 1) {
      Dali::TouchPoint p(0, Dali::PointState::DOWN, 400, 400);
      EmitTouch(p);
    }
    else if(mTestStep == 2)
    {
      Dali::TouchPoint p(0, Dali::PointState::DOWN, 798, 511);
      EmitTouch(p);
    }
    else if(mTestStep == 3)
    {
      Dali::TouchPoint p(0, Dali::PointState::DOWN, 568, 238);
      EmitTouch(p);
    }
    else if(mTestStep == 4)
    {
      mApplication.Quit();
    }
    CaptureWindowAfterFrameRendered(mWindow);
    mTestStep++;
    return false;
  }


  Dali::Timer mTimer;

  void PerformNextTest(uint32_t millisDelay = 0)
  {
    mTimer.Reset();
    // check previous step
    mTimer = Dali::Timer::New(1000);
    mTimer.TickSignal().Connect(this, &ColliderMeshTest::OnUpdate);
    mTimer.Start();
  }

  void PostRender(std::string outputFile, bool success) override
  {
    std::string fname(IMAGE_FILE);
    fname += std::to_string(mTestStep +1);
    fname += ".png";
    CompareImageFile(fname, outputFile, 0.98f);
    PerformNextTest(1000);
  }

private:

  Application& mApplication;
  Dali::Window mWindow;
  Scene3D::SceneView mSceneView;
  int mTestStep{-1};
};

DALI_VISUAL_TEST_WITH_WINDOW_SIZE( ColliderMeshTest, OnInit, 1280, 720 )
