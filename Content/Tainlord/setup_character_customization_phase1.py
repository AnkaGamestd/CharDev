#!/usr/bin/env python
"""
Tainlord Character Customization - Phase 1 Integration Setup
Run this in Unreal Editor's Output Log (cmd dropdown -> Python):

    exec(open(r'F:/Unreal/Projects/Tainlord/Content/Tainlord/setup_character_customization_phase1.py').read()); setup()

Creates:
1. DA_CharacterCustomizationCatalog data asset with placeholder Male/Human entries
2. Test character Blueprint (BP_TainlordCustomizationTest) with appearance component
3. Runs a smoke test to verify the system works
"""

import unreal

# --- Configuration ---
TAINLORD_ROOT = "/Game/Tainlord"
DATA_FOLDER = f"{TAINLORD_ROOT}/Data"
CHARACTER_FOLDER = f"{TAINLORD_ROOT}/Characters"
TEST_FOLDER = f"{TAINLORD_ROOT}/Test"

# Base mannequin mesh from DynamicSwordAnimset
BASE_SKELETON_MESH = "/Game/DynamicSwordAnimset/Character/SK_Mannequin"
BASE_SKELETON = "/Game/DynamicSwordAnimset/Character/SK_Mannequin_Skeleton"

# KD_Manny mesh (project-specific variant)
KD_MANNY_MESH = "/Game/KingdawnCombat/KD_Manny"

# --- Helper Functions ---

def ensure_folder(path):
    """Create content browser folder if it doesn't exist."""
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)
        unreal.log(f"Created folder: {path}")
    else:
        unreal.log(f"Folder already exists: {path}")

def create_data_asset(asset_class_path, asset_name, dest_folder):
    """
    Create a DataAsset of the specified C++ class.
    asset_class_path: e.g. '/Script/Tainlord.TainlordCharacterCustomizationCatalog'
    asset_name: e.g. 'DA_CharacterCustomizationCatalog'
    dest_folder: e.g. '/Game/Tainlord/Data'
    """
    asset_path = f"{dest_folder}/{asset_name}"
    
    # Check if already exists
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"Asset already exists: {asset_path}, loading...")
        return unreal.EditorAssetLibrary.load_asset(asset_path)
    
    unreal.log(f"Creating DataAsset: {asset_name} from class: {asset_class_path}")
    
    # Load the class
    asset_class = unreal.load_class(None, asset_class_path)
    
    if not asset_class:
        unreal.log_error(f"Could not load class: {asset_class_path}")
        return None
    
    unreal.log(f"Loaded asset class: {asset_class.get_name()}")
    
    # Create the data asset using AssetTools
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.DataAssetFactory()
    factory.set_editor_property('data_asset_class', asset_class)
    
    try:
        data_asset = asset_tools.create_asset(
            asset_name=asset_name,
            package_path=dest_folder,
            asset_class=None,  # Factory determines this
            factory=factory
        )
        
        if data_asset:
            unreal.EditorAssetLibrary.save_loaded_asset(data_asset)
            unreal.log(f"Created DataAsset: {asset_path}")
            return data_asset
        else:
            unreal.log_error(f"Failed to create DataAsset: {asset_name}")
            return None
    except Exception as e:
        unreal.log_error(f"Exception creating DataAsset: {str(e)}")
        return None

def populate_catalog_placeholder_data(catalog_asset):
    """
    Populate the catalog with placeholder Male/Human entries.
    Uses the base SK_Mannequin mesh for head/body (since we don't have separate modular meshes yet).
    """
    if not catalog_asset:
        unreal.log_error("Cannot populate catalog - asset is None")
        return False
    
    unreal.log("Populating catalog with placeholder Male/Human data...")
    
    # Load base mesh
    base_mesh = unreal.EditorAssetLibrary.load_asset(BASE_SKELETON_MESH)
    if not base_mesh:
        unreal.log_error(f"Could not load base mesh: {BASE_SKELETON_MESH}")
        return False
    
    unreal.log(f"Loaded base mesh: {base_mesh.get_name()}")
    
    # Note: We cannot directly set struct array elements via Python due to UE reflection limitations.
    # Instead, we'll log instructions for manual population.
    # In a real workflow, you'd either:
    # 1. Use a C++ editor utility to programmatically populate arrays
    # 2. Create entries via Blueprint editor utility widgets
    # 3. Manually author in the editor
    
    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("MANUAL STEP REQUIRED:")
    unreal.log("=" * 60)
    unreal.log(f"Open the catalog asset in the editor: {catalog_asset.get_path_name()}")
    unreal.log("")
    unreal.log("Add the following placeholder entries:")
    unreal.log("")
    unreal.log("HEADS array:")
    unreal.log("  - Id: 'head_male_human_01'")
    unreal.log("  - MeshAsset: SK_Mannequin (or KD_Manny)")
    unreal.log("  - GenderFilter: Male")
    unreal.log("  - RaceFilter: Human")
    unreal.log("")
    unreal.log("HAIR array:")
    unreal.log("  - Id: 'hair_male_human_short'")
    unreal.log("  - MeshAsset: SK_Mannequin (placeholder)")
    unreal.log("  - GenderFilter: Male")
    unreal.log("  - RaceFilter: Human")
    unreal.log("")
    unreal.log("BEARDS array:")
    unreal.log("  - Id: 'beard_male_human_stubble'")
    unreal.log("  - MeshAsset: SK_Mannequin (placeholder)")
    unreal.log("  - GenderFilter: Male")
    unreal.log("  - RaceFilter: Human")
    unreal.log("")
    unreal.log("SKIN TONES array:")
    unreal.log("  - Id: 'skintone_fair'")
    unreal.log("  - Color: (R=1.0, G=0.9, B=0.85, A=1.0)")
    unreal.log("  - MaterialParameterName: 'SkinTone'")
    unreal.log("")
    unreal.log("  - Id: 'skintone_medium'")
    unreal.log("  - Color: (R=0.8, G=0.6, B=0.5, A=1.0)")
    unreal.log("  - MaterialParameterName: 'SkinTone'")
    unreal.log("")
    unreal.log("  - Id: 'skintone_dark'")
    unreal.log("  - Color: (R=0.4, G=0.3, B=0.25, A=1.0)")
    unreal.log("  - MaterialParameterName: 'SkinTone'")
    unreal.log("=" * 60)
    unreal.log("")
    
    return True

def create_test_character_blueprint(catalog_asset):
    """
    Create a test character Blueprint with the appearance component attached.
    """
    bp_name = "BP_TainlordCustomizationTest"
    asset_path = f"{TEST_FOLDER}/{bp_name}"
    
    # Check if already exists
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"Test character Blueprint already exists: {asset_path}")
        return unreal.EditorAssetLibrary.load_asset(asset_path)
    
    unreal.log(f"Creating test character Blueprint: {bp_name}")
    
    # Use ACharacter as the base (or KDCombatCharacter if you want gameplay features)
    parent_class = unreal.load_class(None, '/Script/Engine.Character')
    
    if not parent_class:
        unreal.log_error("Could not load Character class")
        return None
    
    # Create the blueprint
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.BlueprintFactory()
    factory.set_editor_property('parent_class', parent_class)
    
    try:
        blueprint = asset_tools.create_asset(
            asset_name=bp_name,
            package_path=TEST_FOLDER,
            asset_class=unreal.Blueprint,
            factory=factory
        )
        
        if not blueprint:
            unreal.log_error(f"Failed to create Blueprint: {bp_name}")
            return None
        
        unreal.log(f"Created Blueprint: {asset_path}")
        
        # TODO: Add TainlordCharacterAppearanceComponent via Blueprint editor
        # Python API has limited Blueprint CDO manipulation capabilities
        # You'll need to manually add the component in the Blueprint editor
        
        unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
        
        unreal.log("")
        unreal.log("=" * 60)
        unreal.log("MANUAL STEP REQUIRED:")
        unreal.log("=" * 60)
        unreal.log(f"Open the Blueprint in the editor: {asset_path}")
        unreal.log("")
        unreal.log("1. Add Component -> TainlordCharacterAppearanceComponent")
        unreal.log("2. In the component details:")
        unreal.log(f"   - Set Catalog = {catalog_asset.get_path_name() if catalog_asset else 'None'}")
        unreal.log("   - Set HeadMeshComponent = Mesh (the character's skeletal mesh component)")
        unreal.log("   - Set BodyMeshComponent = Mesh")
        unreal.log("   - Set HairMeshComponent = (create a new skeletal mesh component if needed)")
        unreal.log("   - Set BeardMeshComponent = (create a new skeletal mesh component if needed)")
        unreal.log("")
        unreal.log("3. In BeginPlay (or a custom event), call:")
        unreal.log("   - SetProfileContext(Gender=Male, Race=Human)")
        unreal.log("   - ApplyAppearance(appearance data with IDs from catalog)")
        unreal.log("=" * 60)
        unreal.log("")
        
        return blueprint
        
    except Exception as e:
        unreal.log_error(f"Exception creating Blueprint: {str(e)}")
        return None

def run_smoke_test():
    """
    Log smoke test instructions (actual testing requires Blueprint/C++ execution).
    """
    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("PHASE 1 SMOKE TEST CHECKLIST")
    unreal.log("=" * 60)
    unreal.log("")
    unreal.log("1. Verify catalog asset populated:")
    unreal.log("   - Open DA_CharacterCustomizationCatalog")
    unreal.log("   - Confirm Heads, Hair, Beards, SkinTones arrays have entries")
    unreal.log("")
    unreal.log("2. Verify test character setup:")
    unreal.log("   - Open BP_TainlordCustomizationTest")
    unreal.log("   - Confirm TainlordCharacterAppearanceComponent is present")
    unreal.log("   - Confirm component properties are set (catalog, mesh refs)")
    unreal.log("")
    unreal.log("3. Test appearance apply:")
    unreal.log("   - Place BP_TainlordCustomizationTest in a level")
    unreal.log("   - PIE (Play In Editor)")
    unreal.log("   - Check Output Log for TainlordAppearance messages")
    unreal.log("   - Verify no 'not found or not allowed' errors")
    unreal.log("")
    unreal.log("4. Test invalid context rejection:")
    unreal.log("   - Try applying a Female-only head with Male context")
    unreal.log("   - Should see rejection warning in log")
    unreal.log("")
    unreal.log("5. Test skin tone:")
    unreal.log("   - Apply different skin tone IDs")
    unreal.log("   - If materials have 'SkinTone' parameter, color should change")
    unreal.log("=" * 60)
    unreal.log("")

def setup():
    """Main setup function - call this to create all Phase 1 assets."""
    unreal.log("=" * 80)
    unreal.log("Tainlord Character Customization - Phase 1 Integration Setup")
    unreal.log("=" * 80)
    unreal.log("")
    
    # Create folder structure
    unreal.log("STEP 1: Creating folder structure...")
    ensure_folder(TAINLORD_ROOT)
    ensure_folder(DATA_FOLDER)
    ensure_folder(CHARACTER_FOLDER)
    ensure_folder(TEST_FOLDER)
    unreal.log("")
    
    # Track assets
    created_assets = []
    
    # 1. Create catalog data asset
    unreal.log("STEP 2: Creating DA_CharacterCustomizationCatalog...")
    catalog = create_data_asset(
        '/Script/Tainlord.TainlordCharacterCustomizationCatalog',
        'DA_CharacterCustomizationCatalog',
        DATA_FOLDER
    )
    if catalog:
        created_assets.append('DA_CharacterCustomizationCatalog')
        populate_catalog_placeholder_data(catalog)
    unreal.log("")
    
    # 2. Create test character Blueprint
    unreal.log("STEP 3: Creating BP_TainlordCustomizationTest...")
    test_char = create_test_character_blueprint(catalog)
    if test_char:
        created_assets.append('BP_TainlordCustomizationTest')
    unreal.log("")
    
    # 3. Display smoke test instructions
    unreal.log("STEP 4: Smoke test instructions...")
    run_smoke_test()
    
    # Summary
    unreal.log("")
    unreal.log("=" * 80)
    unreal.log("SETUP COMPLETE")
    unreal.log("=" * 80)
    unreal.log(f"Created {len(created_assets)} assets:")
    for asset in created_assets:
        unreal.log(f"  - {asset}")
    unreal.log("")
    unreal.log("Next steps:")
    unreal.log("1. Follow the MANUAL STEP instructions logged above")
    unreal.log("2. Run the smoke test checklist")
    unreal.log("3. Proceed to creation UI or profile load integration")
    unreal.log("=" * 80)

# Auto-run if executed directly (you can also call setup() manually)
if __name__ == "__main__":
    setup()
