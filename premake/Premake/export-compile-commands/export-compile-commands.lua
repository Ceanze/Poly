local p = premake

p.modules.export_compile_commands = {}
local m = p.modules.export_compile_commands

local workspace = p.workspace
local project = p.project

function m.getToolset(cfg)
	return p.tools[cfg.toolset or "gcc"]
end

function m.getIncludeDirs(cfg)
	local flags = {}
	for _, dir in ipairs(cfg.includedirs) do
		table.insert(flags, "-I" .. p.quoted(dir))
	end
	for _, dir in ipairs(cfg.sysincludedir or {}) do
		table.insert(result, "-isystem " .. p.quoted(dir))
	end
	if cfg.pchheader ~= nil and cfg.pchheader ~= "" then
		table.insert(flags, "-I" .. p.quoted("D:/dev/Poly/Poly/src/" .. cfg.pchheader))
	end
	return flags
end

function m.getCppStandard(cfg)
	if cfg.language ~= "C++" or cfg.cppdialect == nil or cfg.cppdialect == "" then
		return ""
	end

	-- From premake-core/modules/d/actions/vcxproj.lua
	local standards = {
		["Default"] = "c++17", -- the default C++ dialect for the toolset
		["C++latest"] = "c++20", -- the latest C++ dialect for the toolset or action where available, otherwise the latest C++ dialect supported by Premake
		["C++98"] = "c++98", -- ISO C++98
		["C++0x"] = "c++11", -- ISO C++11 Draft
		["C++11"] = "c++11", -- ISO C++11
		["C++1y"] = "c++14", -- ISO C++14 Draft
		["C++14"] = "c++14", -- ISO C++14
		["C++1z"] = "c++17", -- ISO C++17 Draft
		["C++17"] = "c++17", -- ISO C++17
		["C++2a"] = "c++20", -- ISO C++20 Draft
		["C++20"] = "c++20", -- ISO C++20
		["gnu++98"] = "c++98", -- GNU dialect of ISO C++98
		["gnu++0x"] = "c++11", -- GNU dialect of ISO C++11 Draft
		["gnu++11"] = "c++11", -- GNU dialect of ISO C++11
		["gnu++1y"] = "c++14", -- GNU dialect of ISO C++14 Draft
		["gnu++14"] = "c++14", -- GNU dialect of ISO C++14
		["gnu++1z"] = "c++17", -- GNU dialect of ISO C++17 Draft
		["gnu++17"] = "c++17", -- GNU dialect of ISO C++17
		["gnu++2a"] = "c++20", -- GNU dialect of ISO C++20 Draft
		["gnu++20"] = "c++20", -- GNU dialect of ISO C++20
	}

	if standards[cfg.cppdialect] ~= nil then
		return "-std=" .. standards[cfg.cppdialect]
	else
		return ""
	end
end

function m.getCommonFlags(cfg)
	local toolset = m.getToolset(cfg)
	local flags = toolset.getcppflags(cfg)
	flags = table.join(flags, toolset.getdefines(cfg.defines))
	flags = table.join(flags, toolset.getundefines(cfg.undefines))
	-- can't use toolset.getincludedirs because some tools that consume
	-- compile_commands.json have problems with relative include paths
	flags = table.join(flags, m.getIncludeDirs(cfg))
	flags = table.join(flags, toolset.getcflags(cfg))
	flags = table.join(flags, m.getCppStandard(cfg))
	return table.join(flags, cfg.buildoptions)
end

function m.getObjectPath(prj, cfg, node)
	return path.join(cfg.objdir, path.appendExtension(node.objname, ".o"))
end

function m.getDependenciesPath(prj, cfg, node)
	return path.join(cfg.objdir, path.appendExtension(node.objname, ".d"))
end

function m.getFileFlags(prj, cfg, node)
	return table.join(m.getCommonFlags(cfg), {
		"-o",
		m.getObjectPath(prj, cfg, node),
		"-MF",
		m.getDependenciesPath(prj, cfg, node),
		"-c",
		node.abspath,
	})
end

function m.generateCompileCommand(prj, cfg, node)
	return {
		directory = prj.location,
		file = node.abspath,
		command = "cc " .. table.concat(m.getFileFlags(prj, cfg, node), " "),
	}
end

function m.includeFile(prj, node, depth)
	return path.iscppfile(node.abspath)
end

function m.getConfig(prj)
	if _OPTIONS["export-compile-commands-config"] then
		return project.getconfig(
			prj,
			_OPTIONS["export-compile-commands-config"],
			_OPTIONS["export-compile-commands-platform"]
		)
	end
	for cfg in project.eachconfig(prj) do
		-- just use the first configuration which is usually "Debug"
		return cfg
	end
end

function m.getProjectCommands(prj, cfg)
	local tr = project.getsourcetree(prj)
	local cmds = {}
	p.tree.traverse(tr, {
		onleaf = function(node, depth)
			if not m.includeFile(prj, node, depth) then
				return
			end
			table.insert(cmds, m.generateCompileCommand(prj, cfg, node))
		end,
	})
	return cmds
end

local function execute()
	for wks in p.global.eachWorkspace() do
		local cfgCmds = {}
		for prj in workspace.eachproject(wks) do
			for cfg in project.eachconfig(prj) do
				local cfgKey = string.format("%s", cfg.shortname)
				if not cfgCmds[cfgKey] then
					cfgCmds[cfgKey] = {}
				end
				cfgCmds[cfgKey] = table.join(cfgCmds[cfgKey], m.getProjectCommands(prj, cfg))
			end
		end
		for cfgKey, cmds in pairs(cfgCmds) do
			local outfile = string.format("compile_commands/%s.json", cfgKey)
			p.generate(wks, outfile, function(wks)
				p.w("[")
				for i = 1, #cmds do
					local item = cmds[i]
					local command = string.format(
						[[
          {
            "directory": "%s",
            "file": "%s",
            "command": "%s"
          }]],
						item.directory,
						item.file,
						item.command:gsub("\\", "\\\\"):gsub('"', '\\"')
					)
					if i > 1 then
						p.w(",")
					end
					p.w(command)
				end
				p.w("]")
			end)
		end
	end
end

newaction({
	trigger = "export-compile-commands",
	description = "Export compiler commands in JSON Compilation Database Format",
	execute = execute,
})

return m
