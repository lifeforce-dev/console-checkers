local config = require "common_paths"
local package_info = require "package_info"

newoption {
    trigger = "sln_name",
    value = "NAME",
    description = "The name of the solution"
}

if not _OPTIONS["sln_name"] then
    print("Error: You must provide a solution name.")
    os.exit(1)
end

print("Working directory: " .. os.getcwd())

local function recursiveAddFiles(dir)
    files { path.join(dir, "**.cpp"), path.join(dir, "**.h") }
    vpaths { ["*"] = { dir .. "/**" } }
    includedirs { dir }
end

-- Folders in here will be treated as projects that should be built as static libs.
-- The folder structure is the same, where _static/<project_name> just as source/<project_name>
-- Currently, all projects will depend on any static lib made in here. Its meant for having something
-- like a "common" lib. Perhaps in the future I'll add more control, for now static libs are global.
local static_folder_name = "_static"
local source_dir = path.join(config.sln_dir, "source")
local static_dir = path.join(source_dir, "_static")
local static_lib_dirs = os.matchdirs(path.join(static_dir, "*")) -- Get all directories in 'static'

workspace (_OPTIONS["sln_name"])
architecture "x64"
configurations { "Debug", "Release" }
cppdialect "C++20"

group "contrib"
for name, pkg in pairs(package_info.packages) do
    local premake_script_path = path.join(config.sln_dir,
        "premake/supported-packages", name, pkg.version, "premake.lua")
    include(premake_script_path)
end
group ""

-- Collect directories from contrib packages
local contrib_includes = {}
for pkg_name, pkg in pairs(package_info.packages) do
    if config.project_includes[pkg_name] then
        table.insert(contrib_includes, config.project_includes[pkg_name])
        -- Print the directory being added to the include path
        print("Adding include directory for package '" .. pkg_name .. "': " .. config.project_includes[pkg_name])
    end
end

-- Set up static libraries
for _, lib_dir in ipairs(static_lib_dirs) do
    local lib_name = path.getname(lib_dir)
    project(lib_name)
    kind "StaticLib"
    language "C++"
    location(path.join(config.sln_dir, "build", "projects", "packages"))
    targetdir(config.lib_dir)
    objdir(path.join(config.obj_dir, lib_name))
    includedirs(static_dir)  -- Include directory for all static libs
    includedirs(contrib_includes)
    recursiveAddFiles(lib_dir)
end

-- Set up other projects excluding static libraries
local projects = os.matchdirs(path.join(source_dir, "*"))
for _, project_dir in ipairs(projects) do
    local project_name = path.getname(project_dir)

     -- Skip static lib directories
    if project_name ~= "_static" then
        project(project_name)
        kind "ConsoleApp"
        language "C++"
        location(path.join(config.sln_dir, "build", "projects", "packages"))
        targetdir(config.bin_dir)
        objdir(path.join(config.obj_dir, project_name))

          -- Include directory for all static libs
        includedirs(static_dir)
        includedirs(contrib_includes)
        recursiveAddFiles(project_dir)
        -- Link all static libraries
        for _, lib_dir in ipairs(static_lib_dirs) do
            links { path.getname(lib_dir) }
        end
    end
end


filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"
    runtime "Debug"

filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "On"
    runtime "Release"