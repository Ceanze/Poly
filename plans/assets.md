# Assets
New asset handling, including asset loading, managing, caching, components, types, and more.

## Asset Manager

See Poly/Resources/AssetHandler.h (temporay name while WIP)

Static class that loads, unloads, resolves and handles importers of assets.

Loading an asset can be done sync or async, and it returns an `AssetHandle<AssetType>`. The handle is just a wrapper around Handle, a generational ID handle.

## Asset Registry

Owns the storage of all the asset types, and handles the mapping between IDs -> AssetHandle, ID -> path, tracks current state of assets (loading, loaded, unloading, unloaded)

## Asset Manifest

Meta data for an asset, used together with composite objects such as models (Scene Assets) from for instance .gltf files. The manifest is combined with a sidecar .meta file next to the actual file to store the sub-assetIDs together with other data that might have been generated/set by the user.

Example of how a manifest can look like:
```md
AssetManifest
├── SchemaVersion          // .meta format version, for migrations
├── SourcePath              // sanity-check, redundant w/ filename but catches renames
├── SourceHash / SourceMTime // staleness detection, triggers reimport
├── ImporterID + ImporterVersion // which importer made this, version bump = reimport
├── RootAssetID + RootAssetType  // e.g. the SceneAsset itself
├── Children[]
│    ├── AssetID
│    ├── Type              // MeshAsset, MaterialAsset, ...
│    ├── Name / Index       // "mesh_0", or gltf node name, for editor display + stable resolution
│    └── (optional per-type extras: vertex/index count, material slot refs — nice-to-have for editor UI, not load-critical)
└── ImportSettings          // user-editable, type-specific
     ├── (model) ScaleFactor, GenerateTangents, AxisConvention, ImportMaterials(bool)
     ├── (texture) sRGB, GenerateMipmaps, CompressionFormat
     └── ...
```

## Asset Handle

Wrapper with a generational ID, used to represent a loaded (or soon to be) asset.

## Asset ID

Unique asset ID based on the vfs path. Sub assets like MeshAsset from a SceneAsset are generated using a special path like "assets/model/sponza.gltf#mesh_0" or similar.

## IImporter

Importer interface, actual implementations derive from this and implement the importing logic for the `GetSupportedExtensions()` and registers this to the Asset Manager.

## Example flow

### Folder structure

Example folder with assets
```md
assets/
├── Sponza.gltf
├── ceanze.png
```

### Asset manager setup

```c++
AssetManager::Init();
```

```c++
void AssetManager::Init() {
	m_Registry.Scan("/"); // "/" scans across all mounted backends, regardless of virtual root
}
```

### Initial loading + importing

```c++
AssetHandle<TextureAsset> asset = AssetManager::Load<TextureAsset>("assets/ceanze.png")
```

Load then checks asset registry if it is loaded

```c++
template<typename T>
AssetHandle<T> AssetManager::Load(std::string_view vfsPath)
{
	AssetID id(vfsPath);

	if (m_Registry.IsLoaded(id))
		return m_Registry.GetHandle<T>(id);

	IImporter* importer = GetImporter(vfsPath); // keyed by extension, not T
	if (!importer)
	{
		LOG_ERROR("No importer registered for '{}'", vfsPath);
		return AssetHandle<T>(); // invalid/default handle
	}

	if (!importer->Import(vfsPath, id, m_Registry)) // writes slot(s) into registry directly
	{
		LOG_ERROR("Import failed for '{}'", vfsPath);
		return AssetHandle<T>();
	}

	return m_Registry.GetHandle<T>(id); // now populated, same lookup as the cache-hit path
}

IImporter* AssetManager::GetImporter(std::string_view vfsPath)
{
	std::string ext = std::filesystem::path(vfsPath).extension().string();
	auto it = m_ImportersByExt.find(ext);
	return it != m_ImportersByExt.end() ? it->second : nullptr;
}
```

