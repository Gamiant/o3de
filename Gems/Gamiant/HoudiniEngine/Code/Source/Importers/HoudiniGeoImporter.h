/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/EntityId.h>

#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>

#include <HAPI/HAPI.h>

#include <HoudiniOutput.h>
#include <HoudiniPackageParams.h>
#include <HoudiniSettings.h>

namespace HoudiniEngine
{
    class HoudiniGeoImporter
    {
    public:

        void SetBakeRootFolder(const AZStd::string& folder) { m_bakeRootFolder = folder; };
        void SetOutputFilename(const AZStd::string& filename) { m_outputFilename = filename; };

        AZStd::vector<AZ::EntityId>& GetOutputObjects() { return OutputObjects; };

        // BEGIN: Static API
        // Open a BGEO file: create a file node in HAPI and cook it
        static bool OpenBGEOFile(const AZStd::string& InBGEOFile, HAPI_NodeId& OutNodeId, bool bInUseWorldComposition = false);

        // Cook the file node specified by the valid NodeId.
        static bool CookFileNode(const HAPI_NodeId& InNodeId);

        // Extract the outputs for a given node ID
        static bool BuildAllOutputsForNode(const HAPI_NodeId& InNodeId, AZ::EntityId InOuter, AZStd::vector<HoudiniOutput*>& InOldOutputs, AZStd::vector<HoudiniOutput*>& OutNewOutputs, bool bInAddOutputsToRootSet = false, bool bInUseOutputNodes = true);

        // Delete the HAPI node and remove InOutputs from the root set.
        static bool CloseBGEOFile(const HAPI_NodeId& InNodeId);
        // END: Static API

        // Import the BGEO file
        //bool ImportBGEOFile(const AZStd::string& InBGEOFile, AZ::EntityId InParent, const HoudiniPackageParams* InPackageParams = nullptr, const StaticMeshGenerationSettings* InStaticMeshGenerationProperties = nullptr, const MeshBuildSettings* InMeshBuildSettings = nullptr);

        // 1. Start a HE session if needed
        static bool AutoStartHoudiniEngineSessionIfNeeded();

        // 2. Update our file members fromn the input file path
        bool SetFilePath(const AZStd::string& InFilePath);

        // 3. Creates a new file node and loads the bgeo file in HAPI
        bool LoadBGEOFileInHAPI(HAPI_NodeId& NodeId);

        // 3.2 (alternative) Uses an object merge node to load the geo data in HAPI (used for node sync fetch)
        bool MergeGeoFromNode(const AZStd::string& InNodePath, HAPI_NodeId& OutNodeId);

        // 4. Extract the outputs for a given node ID
        bool BuildOutputsForNode(const HAPI_NodeId& InNodeId, AZStd::vector<HoudiniOutput*>& InOldOutputs, AZStd::vector<HoudiniOutput*>& OutNewOutputs, bool bInUseOutputNodes = true);

        // 5. Creates the static meshes object found in the output
        bool CreateStaticMeshes(AZStd::vector<HoudiniOutput*>& InOutputs, HoudiniPackageParams InPackageParams, const StaticMeshGenerationSettings& InStaticMeshGenerationProperties, const MeshBuildSettings& InMeshBuildSettings);

        // 6. Create the output curves
        bool CreateCurves(AZStd::vector<HoudiniOutput*>& InOutputs, HoudiniPackageParams InPackageParams);
        // 7. Create the output landscapes
        //bool CreateLandscapes(AZStd::vector<HoudiniOutput*>& InOutputs, HoudiniPackageParams InPackageParams);
        //// 8. Create the output landscape splines
        //bool CreateLandscapeSplines(AZStd::vector<HoudiniOutput*>& InOutputs, HoudiniPackageParams InPackageParams);
        //// 9. Create the output instancers
        //bool CreateInstancers(AZStd::vector<HoudiniOutput*>& InOutputs, HoudiniPackageParams InPackageParams);
        // 10. Clean up the created node
        static bool DeleteCreatedNode(const HAPI_NodeId& InNodeId);

        //static bool CreateInstancerOutputPartData(AZStd::vector<HoudiniOutput*>& InOutputs, AZStd::unordered_map<struct FHoudiniOutputObjectIdentifier, struct FHoudiniInstancedOutputPartData>& OutInstancedOutputPartData);

    private:

        // Path to the file we're currently loading
        AZStd::string SourceFilePath;

        // Absolute Path to the file
        AZStd::string AbsoluteFilePath;
        AZStd::string AbsoluteFileDirectory;

        // Input File Name / Extension
        AZStd::string FileName;
        AZStd::string FileExtension;

        // Output filename, if empty, will be set to the input filename
        AZStd::string m_outputFilename;

        // Root Folder for storing the created files
        AZStd::string m_bakeRootFolder;

        // Output Objects
        AZStd::vector<AZ::EntityId> OutputObjects;

    };
}
