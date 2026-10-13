#include "SlangCompiler.h"

#include "Poly/Resources/Shader/SlangReflector.h"
#include "Poly/Resources/VFS/VirtualFileSystem.h"
#include "polypch.h"

#include <atomic>
#include <slang/slang-com-ptr.h>
#include <slang/slang.h>

namespace
{
	constexpr const char* SHADER_SEARCH_PATH = "assets/shaders";

	class SlangVFSBlob final : public ISlangBlob
	{
	public:
		explicit SlangVFSBlob(std::vector<byte> data)
		    : m_Data(std::move(data))
		{}

		SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** ppOutObject) override
		{
			if (uuid == ISlangUnknown::getTypeGuid() || uuid == ISlangBlob::getTypeGuid())
			{
				addRef();
				*ppOutObject = static_cast<ISlangBlob*>(this);
				return SLANG_OK;
			}

			return SLANG_E_NO_INTERFACE;
		}

		SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return ++m_RefCount; }

		SLANG_NO_THROW uint32_t SLANG_MCALL release() override
		{
			const uint32_t refCount = --m_RefCount;
			if (refCount == 0)
				delete this;
			return refCount;
		}

		SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() override { return m_Data.data(); }
		SLANG_NO_THROW size_t SLANG_MCALL      getBufferSize() override { return m_Data.size(); }

	private:
		std::atomic<uint32_t> m_RefCount = 1;
		std::vector<byte>     m_Data;
	};

	SlangStage ConvertShaderStageSlang(Poly::FShaderStage shaderStage)
	{
		switch (shaderStage)
		{
		case Poly::FShaderStage::VERTEX:
			return SLANG_STAGE_VERTEX;
		case Poly::FShaderStage::FRAGMENT:
			return SLANG_STAGE_FRAGMENT;
		case Poly::FShaderStage::COMPUTE:
			return SLANG_STAGE_COMPUTE;
		case Poly::FShaderStage::TESSELLATION_CONTROL:
			return SLANG_STAGE_HULL;
		case Poly::FShaderStage::TESSELLATION_EVALUATION:
			return SLANG_STAGE_DOMAIN;
		case Poly::FShaderStage::GEOMETRY:
			return SLANG_STAGE_GEOMETRY;
		case Poly::FShaderStage::RAYGEN:
			return SLANG_STAGE_RAY_GENERATION;
		case Poly::FShaderStage::ANY_HIT:
			return SLANG_STAGE_ANY_HIT;
		case Poly::FShaderStage::CLOSEST_HIT:
			return SLANG_STAGE_CLOSEST_HIT;
		case Poly::FShaderStage::MISS:
			return SLANG_STAGE_MISS;
		case Poly::FShaderStage::INTERSECTION:
			return SLANG_STAGE_INTERSECTION;
		case Poly::FShaderStage::CALLABLE:
			return SLANG_STAGE_CALLABLE;
		case Poly::FShaderStage::TASK:
			return SLANG_STAGE_AMPLIFICATION;
		case Poly::FShaderStage::MESH:
			return SLANG_STAGE_MESH;
		default:
			return SLANG_STAGE_NONE;
		}
	}

	// Slang reports warnings through the same blob as errors, so it is logged even on success
	void LogDiagnostics(std::string_view path, slang::IBlob* pDiagnostics, bool failed)
	{
		if (!pDiagnostics || pDiagnostics->getBufferSize() == 0)
			return;

		const std::string_view message(static_cast<const char*>(pDiagnostics->getBufferPointer()), pDiagnostics->getBufferSize());
		if (failed)
			POLY_CORE_ERROR("Slang compilation failed for: {0} \n {1}", path, message);
		else
			POLY_CORE_WARN("Slang diagnostics for: {0} \n {1}", path, message);
	}
} // namespace

namespace Poly
{
	// Lets slang resolve import/#include through the VFS. Owned by the SlangCompiler, so the ref count is unused
	class SlangFileSystem final : public ISlangFileSystem
	{
	public:
		SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** ppOutObject) override
		{
			*ppOutObject = castAs(uuid);
			return *ppOutObject ? SLANG_OK : SLANG_E_NO_INTERFACE;
		}

		SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return 1; }
		SLANG_NO_THROW uint32_t SLANG_MCALL release() override { return 1; }

		SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& uuid) override
		{
			if (uuid == ISlangUnknown::getTypeGuid() || uuid == ISlangCastable::getTypeGuid() || uuid == ISlangFileSystem::getTypeGuid())
				return static_cast<ISlangFileSystem*>(this);

			return nullptr;
		}

		SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(char const* pPath, ISlangBlob** ppOutBlob) override
		{
			if (!VirtualFileSystem::Exists(pPath) || VirtualFileSystem::IsDirectory(pPath))
				return SLANG_E_NOT_FOUND;

			*ppOutBlob = new SlangVFSBlob(VirtualFileSystem::Read(pPath));
			return SLANG_OK;
		}
	};

	SlangCompiler::SlangCompiler()
	{
		SlangResult result = slang::createGlobalSession(&m_pGlobalSession);
		POLY_VALIDATE(SLANG_SUCCEEDED(result), "Slang global session could not be created - result was {}", static_cast<int>(result));
		if (!m_pGlobalSession)
			return;

		POLY_CORE_INFO("Slang initialized, version {}", m_pGlobalSession->getBuildTagString());

		m_pFileSystem = CreateUnique<SlangFileSystem>();

		slang::TargetDesc targetDesc = {};
		targetDesc.format            = SLANG_SPIRV;
		targetDesc.profile           = m_pGlobalSession->findProfile("spirv_1_5");

		slang::SessionDesc sessionDesc = {};
		sessionDesc.targets            = &targetDesc;
		sessionDesc.targetCount        = 1;
		sessionDesc.searchPaths        = &SHADER_SEARCH_PATH;
		sessionDesc.searchPathCount    = 1;
		sessionDesc.fileSystem         = m_pFileSystem.get();
		// Matches glm and the GLSL shaders
		sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

		// 41012: "entry point uses additional capabilities that are not part of the specified profile", reported for
		// every fragment shader that samples a texture
		slang::CompilerOptionEntry disableProfileWarning = {};
		disableProfileWarning.name                       = slang::CompilerOptionName::DisableWarning;
		disableProfileWarning.value.kind                 = slang::CompilerOptionValueKind::String;
		disableProfileWarning.value.stringValue0         = "41012";
		sessionDesc.compilerOptionEntries                = &disableProfileWarning;
		sessionDesc.compilerOptionEntryCount             = 1;

		result = m_pGlobalSession->createSession(sessionDesc, &m_pSession);
		POLY_VALIDATE(SLANG_SUCCEEDED(result), "Slang session could not be created - result was {}", static_cast<int>(result));
	}

	SlangCompiler::~SlangCompiler()
	{
		m_Modules.clear();

		if (m_pSession)
			m_pSession->release();

		if (m_pGlobalSession)
			m_pGlobalSession->release();

		slang::shutdown();
	}

	CompiledShader SlangCompiler::Compile(std::string_view path, FShaderStage shaderStage)
	{
		slang::IModule* pModule = LoadModule(path);
		if (!pModule)
			return {};

		const SlangStage                  stage = ConvertShaderStageSlang(shaderStage);
		Slang::ComPtr<slang::IEntryPoint> pEntryPoint;
		for (SlangInt32 i = 0; i < pModule->getDefinedEntryPointCount(); i++)
		{
			Slang::ComPtr<slang::IEntryPoint> pCandidate;
			pModule->getDefinedEntryPoint(i, pCandidate.writeRef());

			slang::ProgramLayout* pLayout = pCandidate->getLayout();
			if (!pLayout || pLayout->getEntryPointCount() == 0 || pLayout->getEntryPointByIndex(0)->getStage() != stage)
				continue;

			if (pEntryPoint)
			{
				POLY_CORE_ERROR("Slang module {} defines more than one entry point for the requested stage", path);
				return {};
			}

			pEntryPoint = pCandidate;
		}

		if (!pEntryPoint)
		{
			POLY_CORE_ERROR("Slang module {} defines no entry point for the requested stage, is it missing a [shader(\"...\")] attribute?", path);
			return {};
		}

		slang::IComponentType*               components[] = {pModule, pEntryPoint.get()};
		Slang::ComPtr<slang::IComponentType> pComposite;
		Slang::ComPtr<slang::IBlob>          pDiagnostics;
		SlangResult                          result = m_pSession->createCompositeComponentType(components, 2, pComposite.writeRef(), pDiagnostics.writeRef());
		LogDiagnostics(path, pDiagnostics, SLANG_FAILED(result));
		if (SLANG_FAILED(result))
			return {};

		Slang::ComPtr<slang::IComponentType> pLinked;
		pDiagnostics.setNull();
		result = pComposite->link(pLinked.writeRef(), pDiagnostics.writeRef());
		LogDiagnostics(path, pDiagnostics, SLANG_FAILED(result));
		if (SLANG_FAILED(result))
			return {};

		Slang::ComPtr<slang::IBlob> pCode;
		pDiagnostics.setNull();
		result = pLinked->getEntryPointCode(0, 0, pCode.writeRef(), pDiagnostics.writeRef());
		LogDiagnostics(path, pDiagnostics, SLANG_FAILED(result));
		if (SLANG_FAILED(result))
			return {};

		const byte*    pData = static_cast<const byte*>(pCode->getBufferPointer());
		CompiledShader compiled;
		compiled.Code       = std::vector<byte>(pData, pData + pCode->getBufferSize());
		compiled.Reflection = SlangReflector::Reflect(pLinked->getLayout());
		return compiled;
	}

	slang::IModule* SlangCompiler::LoadModule(std::string_view path)
	{
		if (!m_pSession)
		{
			POLY_CORE_ERROR("Slang module {} cannot be loaded, the slang session was not created", path);
			return nullptr;
		}

		const std::string key(path);
		if (auto it = m_Modules.find(key); it != m_Modules.end())
			return it->second;

		if (!VirtualFileSystem::Exists(path))
		{
			POLY_CORE_ERROR("Slang module {} does not exist", path);
			return nullptr;
		}

		// Module names share one namespace in the session, the path keeps equally named files in different directories apart
		std::string moduleName = key;
		for (char& character : moduleName)
		{
			if (!std::isalnum(static_cast<unsigned char>(character)))
				character = '_';
		}

		const std::string           source = VirtualFileSystem::ReadText(path);
		Slang::ComPtr<slang::IBlob> pDiagnostics;
		slang::IModule*             pModule = m_pSession->loadModuleFromSourceString(moduleName.c_str(), key.c_str(), source.c_str(), pDiagnostics.writeRef());
		LogDiagnostics(path, pDiagnostics, pModule == nullptr);
		if (!pModule)
			return nullptr;

		m_Modules[key] = pModule;
		return pModule;
	}
} // namespace Poly
