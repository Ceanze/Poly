#pragma once

#include "Platform/API/Buffer.h"
#include "Platform/API/PipelineLayout.h"
#include "Platform/API/Sampler.h"
#include "Platform/API/Texture.h"
#include "Platform/API/TextureView.h"
#include "Poly/Core/Core.h"
#include "Poly/Core/Handle.h"

#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace Poly
{
	class DescriptorSet;
	class CommandPool;
	class CommandBuffer;

	using TextureHandle = Handle<struct TextureHandleTag>;
	using BufferHandle  = Handle<struct BufferHandleTag>;
	using SamplerHandle = Handle<struct SamplerHandleTag>;

	class ResourceManager
	{
	public:
		CLASS_STATIC(ResourceManager);

		static constexpr uint32 MAX_TEXTURES     = 4096;
		static constexpr uint32 MAX_SAMPLERS     = 256;
		static constexpr uint32 FRAMES_IN_FLIGHT = 2;

		// Bit layout of the packed uint32 used in a pass's textureIndices[] push-constant slot: texture
		// index in the low TEXTURE_INDEX_BITS bits, sampler index directly above it (mirrors bindless.glsl).
		static constexpr uint32 TEXTURE_INDEX_BITS  = 12; // log2(MAX_TEXTURES)
		static constexpr uint32 SAMPLER_INDEX_SHIFT = TEXTURE_INDEX_BITS;

		struct TextureInfo
		{
			TextureHandle Handle;
			uint32        Width  = 0;
			uint32        Height = 0;
			EFormat       Format = EFormat::UNDEFINED;
			std::string   DebugName;
		};

		struct BufferInfo
		{
			BufferHandle Handle;
			uint64       Size = 0;
			std::string  DebugName;
		};

		static void Init();
		static void Release();

		/*
		 * Creates a 2D texture
		 * @param width - Width of the texture
		 * @param height - Height of the texture
		 * @param format - Format of the texture
		 * @param usage - Usage of the texture
		 * @param debugName - Debug name of the texture
		 * @return TextureHandle - Handle to the created texture
		 */
		static TextureHandle CreateTexture2D(uint32 width, uint32 height, EFormat format, FTextureUsage usage, std::string debugName = "");

		/*
		 * Creates a specified GPU buffer
		 * @param size - Size of the buffer
		 * @param usage - Usage of the buffer
		 * @param memUsage - Memory usage of the buffer
		 * @param debugName - Debug name of the buffer
		 * @return BufferHandle - Handle to the created buffer
		 */
		static BufferHandle CreateBuffer(uint64 size, FBufferUsage usage, EMemoryUsage memUsage, std::string debugName = "");

		/*
		 * Creates a uniform buffer
		 * @param size - Size of the buffer
		 * @param debugName - Debug name of the buffer
		 * @return BufferHandle - Handle to the created buffer
		 */
		static BufferHandle CreateUniformBuffer(uint64 size, std::string debugName = "");

		/*
		 * Creates a vertex buffer
		 * @param size - Size of the buffer
		 * @param memUsage - Memory usage of the buffer
		 * @param debugName - Debug name of the buffer
		 * @return BufferHandle - Handle to the created buffer
		 */
		static BufferHandle CreateVertexBuffer(uint64 size, EMemoryUsage memUsage = EMemoryUsage::GPU_ONLY, std::string debugName = "");

		/*
		 * Creates an index buffer
		 * @param size - Size of the buffer
		 * @param memUsage - Memory usage of the buffer
		 * @param debugName - Debug name of the buffer
		 * @return BufferHandle - Handle to the created buffer
		 */
		static BufferHandle CreateIndexBuffer(uint64 size, EMemoryUsage memUsage = EMemoryUsage::GPU_ONLY, std::string debugName = "");

		/*
		 * Creates a storage buffer
		 * @param size - Size of the buffer
		 * @param memUsage - Memory usage of the buffer
		 * @param debugName - Debug name of the buffer
		 * @return BufferHandle - Handle to the created buffer
		 */
		static BufferHandle CreateStorageBuffer(uint64 size, EMemoryUsage memUsage = EMemoryUsage::GPU_ONLY, std::string debugName = "");

		/*
		 * Creates a custom texture view - should only be used for special cases where the default view is not sufficient.
		 * Note: If a non-default view is needed to be created, consider creating a new method for it instead
		 * @param texture - Handle to the texture
		 * @param desc - Description of the view
		 * @return Ref<TextureView> - Reference to the created view
		 */
		static Ref<TextureView> CreateCustomView(TextureHandle texture, TextureViewDesc desc);

		/*
		 * Gets or creates a sampler
		 * @param desc - Description of the sampler
		 * @return SamplerHandle - Handle to the created or existing sampler
		 */
		static SamplerHandle GetOrCreateSampler(const SamplerDesc& desc);

		static SamplerHandle GetDefaultLinearSampler();
		static SamplerHandle GetDefaultNearestSampler();

		// TEMP: Used for call sites which have not migrated to the new system with handles yet. Will be removed once all call sites have been updated.
		static uint32 RegisterExternalTextureAndSampler(const TextureView* pTextureView, ETextureLayout layout, Sampler* pSampler);

		/*
		 * Resolves a handle to the underlying Texture object.
		 * @param handle - Handle to the texture
		 * @return Texture* - Pointer to the underlying Texture object - nullptr if the handle is invalid
		 */
		static Texture* Resolve(TextureHandle handle);

		/*
		 * Resolves a handle to the underlying TextureView object.
		 * @param handle - Handle to the texture
		 * @return TextureView* - Pointer to the underlying TextureView object - nullptr if the handle is invalid
		 */
		static TextureView* ResolveView(TextureHandle handle);

		/*
		 * Resolves a handle to the underlying Buffer object.
		 * @param handle - Handle to the buffer
		 * @return Buffer* - Pointer to the underlying Buffer object - nullptr if the handle is invalid
		 */
		static Buffer* Resolve(BufferHandle handle);

		/*
		 * Resolves a handle to the underlying Sampler object.
		 * @param handle - Handle to the sampler
		 * @return Sampler* - Pointer to the underlying Sampler object - nullptr if the handle is invalid
		 */
		static Sampler* Resolve(SamplerHandle handle);

		/*
		 * Queues destruction of a texture.
		 * @param handle - Handle to the texture
		 */
		static void Destroy(TextureHandle handle);

		/*
		 * Queues destruction of a buffer.
		 * @param handle - Handle to the buffer
		 */
		static void Destroy(BufferHandle handle);

		/*
		 * Uploads data to a texture.
		 * @param handle - Handle to the texture
		 * @param pData - Pointer to the data to upload
		 * @param width - Width of the texture
		 * @param height - Height of the texture
		 */
		static void UploadTextureData(TextureHandle handle, const void* pData, uint32 width, uint32 height);

		/*
		 * Uploads data to a buffer. If GPU_ONLY the transfer is queued for next Execute of the render instance. If CPU_VISIBLE the transfer is immediate.
		 * @param handle - Handle to the buffer
		 * @param pData - Pointer to the data to upload
		 * @param size - Size of the data to upload
		 * @param offset - Offset in the buffer to upload to
		 */
		static void UploadBufferData(BufferHandle handle, const void* pData, uint64 size, uint64 offset = 0);

		/*
		 * Updates resource manager state, handling any pending uploads and deferred destruction of resources. Should be called once per frame.
		 * NOTE: Should only be called from the Renderer::Render() function
		 */
		static void Update();

		/*
		 * Gets all textures registered in the resource manager.
		 * @return std::vector<TextureInfo> - Vector of texture information
		 */
		static std::vector<TextureInfo> GetAllTextures();

		/*
		 * Gets all buffers registered in the resource manager.
		 * @return std::vector<BufferInfo> - Vector of buffer information
		 */
		static std::vector<BufferInfo> GetAllBuffers();

		/*
		 * Gets the descriptor set layout for the resource manager.
		 * Mainly for internal engine use
		 * @return const DescriptorSetLayout& - Reference to the descriptor set layout
		 */
		static const DescriptorSetLayout& GetSetLayoutDesc();

		/*
		 * Gets the descriptor set for the resource manager.
		 * Mainly for internal engine use
		 * @return DescriptorSet* - Pointer to the descriptor set
		 */
		static DescriptorSet* GetDescriptorSet();

	private:
		struct TextureSlot
		{
			Ref<Texture>     pTexture;     // null for externally-registered (RegisterExternalTextureAndSampler) slots
			Ref<TextureView> pDefaultView; // null for externally-registered slots
			uint32           Generation = 0;
			uint32           Width = 0, Height = 0;
			EFormat          Format = EFormat::UNDEFINED;
			std::string      DebugName;
			bool             Alive = false;
		};

		struct BufferSlot
		{
			Ref<Buffer> pBuffer;
			uint32      Generation = 0;
			std::string DebugName;
			bool        Alive = false;
		};

		struct SamplerDescLess
		{
			bool operator()(const SamplerDesc& a, const SamplerDesc& b) const;
		};

		struct PendingTextureUpload
		{
			TextureHandle     Handle;
			std::vector<byte> Data;
			uint32            Width, Height;
		};

		struct PendingBufferUpload
		{
			BufferHandle      Handle;
			std::vector<byte> Data;
			uint64            Offset;
		};

		static uint32 AllocTextureSlot(); // pops the free-list or grows m_Textures - caller holds s_Mutex
		static uint32 AllocBufferSlot();
		static uint32 RegisterSamplerIndex(Sampler* pSampler, Ref<Sampler> pOwnedRef); // caller holds s_Mutex
		static void   FlushUploads();                                                  // caller holds s_Mutex

		inline static std::recursive_mutex s_Mutex;

		inline static std::vector<TextureSlot> s_Textures;
		inline static std::vector<uint32>      s_FreeTextureIndices;

		inline static std::vector<BufferSlot> s_Buffers;
		inline static std::vector<uint32>     s_FreeBufferIndices;

		inline static std::vector<Ref<Sampler>>                             s_Samplers; // null entries = externally-owned sampler
		inline static std::map<SamplerDesc, SamplerHandle, SamplerDescLess> s_SamplerCache;
		inline static SamplerHandle                                         s_DefaultLinearSampler;
		inline static SamplerHandle                                         s_DefaultNearestSampler;

		inline static std::vector<PendingTextureUpload> s_PendingTextureUploads;
		inline static std::vector<PendingBufferUpload>  s_PendingBufferUploads;

		// {slot index, frame it was destroyed on} - swept in Update() once FRAMES_IN_FLIGHT frames have passed.
		inline static std::vector<std::pair<uint32, uint64>> s_PendingTextureDestroys;
		inline static std::vector<std::pair<uint32, uint64>> s_PendingBufferDestroys;
		inline static uint64                                 s_CurrentFrame = 0;

		inline static Ref<PipelineLayout> s_pHeapPipelineLayout; // only used to own the set-0 VkDescriptorSetLayout
		inline static Ref<DescriptorSet>  s_pHeapSet;
		inline static DescriptorSetLayout s_SetLayoutDesc;

		// Persistent transfer/graphics command buffers reused every FlushUploads() call, mirroring the
		// pattern AssetLoader::Init() already uses for the same purpose.
		inline static Ref<CommandPool> s_pTransferCommandPool;
		inline static CommandBuffer*   s_pTransferCommandBuffer = nullptr;
		inline static Ref<CommandPool> s_pGraphicsCommandPool;
		inline static CommandBuffer*   s_pGraphicsCommandBuffer = nullptr;
	};
} // namespace Poly
