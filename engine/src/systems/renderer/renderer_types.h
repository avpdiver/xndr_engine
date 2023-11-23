#pragma once

#define DEFINE_RENDERER_HANDLE(T) using T##Handle = RefCountPtr<T>

using namespace Be::Framework::RHI;

namespace Be::System::Renderer
{

  class RenderQueue;
  class RenderContext;
  struct RenderableItemData;

  class Material;

  class Texture;
  class TextureManager;

  class Mesh;
  class MeshManager;

  class Model;
  class ModelManager;

  class ForwardPipeline;
  struct ForwardPipelineDesc;

}