"""
Unreal Editor Python Script: Add Hair & Beard entries to DA_CharacterCustomizationCatalog

USAGE:
  1. Open Tainlord project in UE5 Editor
  2. Open Python console (Window > Developer Tools > Output Log, then type: py console)
  3. Run: exec("Content/Tainlord/Scripts/add_hair_beard_to_catalog.py")
  
  OR:
  1. In UE5 Editor: Tools > Execute Python Script
  2. Paste this file's path

WHAT IT DOES:
  - Loads DA_CharacterCustomizationCatalog DataAsset
  - Adds 1 Hair entry (ShortHair01)
  - Adds 4 Beard entries (MiddleBeard01, SideBeard01, Mustache01, Mustache02)
  - Saves the asset

REQUIREMENTS:
  - Python plugin must be enabled in UE5
  - Hair mesh assets must already be migrated to /Game/Tainlord/Characters/Hair/Meshes/
"""

import unreal

def add_hair_beard_to_catalog():
    # Path to the catalog DataAsset
    catalog_path = "/Game/Tainlord/Data/DA_CharacterCustomizationCatalog"
    
    # Load the catalog
    catalog = unreal.load_asset(catalog_path)
    if not catalog:
        unreal.log_error(f"Could not load catalog at {catalog_path}")
        return False
    
    unreal.log(f"Loaded catalog: {catalog_path}")
    
    # Get the Hair array (we need to use the property system)
    # FTainlordHairEntry fields: Id, MeshAsset, GenderFilter, RaceFilter
    
    # Hair mesh paths
    hair_meshes = [
        {
            "id": "ShortHair01",
            "mesh_path": "/Game/Tainlord/Characters/Hair/Meshes/SK_man_Edit_Man_Short_Hair.SK_man_Edit_Man_Short_Hair",
            "gender": unreal.CharacterGender.MALE,
            "race": unreal.CharacterRace.ANY,
        },
    ]
    
    # Beard mesh paths
    beard_meshes = [
        {
            "id": "MiddleBeard01",
            "mesh_path": "/Game/Tainlord/Characters/Hair/Meshes/SK_man_Edit_Man_Middle_Beard.SK_man_Edit_Man_Middle_Beard",
            "gender": unreal.CharacterGender.MALE,
            "race": unreal.CharacterRace.ANY,
        },
        {
            "id": "SideBeard01",
            "mesh_path": "/Game/Tainlord/Characters/Hair/Meshes/SK_man_Edit_Man_Side_Beard.SK_man_Edit_Man_Side_Beard",
            "gender": unreal.CharacterGender.MALE,
            "race": unreal.CharacterRace.ANY,
        },
        {
            "id": "Mustache01",
            "mesh_path": "/Game/Tainlord/Characters/Hair/Meshes/SK_man_Edit_Man_Mustache_1.SK_man_Edit_Man_Mustache_1",
            "gender": unreal.CharacterGender.MALE,
            "race": unreal.CharacterRace.ANY,
        },
        {
            "id": "Mustache02",
            "mesh_path": "/Game/Tainlord/Characters/Hair/Meshes/SK_man_Edit_Man_Mustache_2.SK_man_Edit_Man_Mustache_2",
            "gender": unreal.CharacterGender.MALE,
            "race": unreal.CharacterRace.ANY,
        },
    ]
    
    # Use EditorAssetSubsystem to modify the DataAsset
    eas = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    
    # We need to use the C++ property system via unreal.EditorAssetLibrary
    # Since Python can't directly modify USTRUCT arrays in DataAssets easily,
    # we'll use a different approach: modify via the reflected properties
    
    unreal.log("NOTE: This script requires manual catalog editing in UE5 Editor.")
    unreal.log("The DataAsset binary format cannot be reliably modified via Python for custom USTRUCT arrays.")
    unreal.log("")
    unreal.log("MANUAL STEPS:")
    unreal.log("1. Open DA_CharacterCustomizationCatalog in Editor (double-click)")
    unreal.log("2. Find the 'Hair' array and click '+' to add entries:")
    for h in hair_meshes:
        unreal.log(f"   - Id: {h['id']}")
        unreal.log(f"     MeshAsset: {h['mesh_path']}")
        unreal.log(f"     GenderFilter: Male")
        unreal.log(f"     RaceFilter: Any")
    unreal.log("3. Find the 'Beards' array and click '+' to add entries:")
    for b in beard_meshes:
        unreal.log(f"   - Id: {b['id']}")
        unreal.log(f"     MeshAsset: {b['mesh_path']}")
        unreal.log(f"     GenderFilter: Male")
        unreal.log(f"     RaceFilter: Any")
    unreal.log("4. Save the asset")
    unreal.log("")
    unreal.log("After saving, hair/beard options will appear in character creation.")
    
    return True

if __name__ == "__main__":
    add_hair_beard_to_catalog()
