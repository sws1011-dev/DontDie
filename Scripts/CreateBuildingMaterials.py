import unreal


MATERIAL_DIR = "/Game/Build/Materials"
MESH_DIR = "/Game/Build/Meshes"


MATERIALS = {
    "M_Build_Workbench": unreal.LinearColor(0.45, 0.25, 0.12, 1.0),
    "M_Build_Storage": unreal.LinearColor(0.38, 0.40, 0.42, 1.0),
    "M_Build_Bed": unreal.LinearColor(0.22, 0.32, 0.58, 1.0),
    "M_Build_Farm": unreal.LinearColor(0.27, 0.17, 0.08, 1.0),
    "M_Build_Firepit": unreal.LinearColor(0.42, 0.32, 0.26, 1.0),
}


MESH_MATERIALS = {
    "SM_Workbench_T1": "M_Build_Workbench",
    "SM_Workbench_T2": "M_Build_Workbench",
    "SM_Workbench_T3": "M_Build_Workbench",
    "SM_Storage_T1": "M_Build_Storage",
    "SM_Storage_T2": "M_Build_Storage",
    "SM_Storage_T3": "M_Build_Storage",
    "SM_Bed_T1": "M_Build_Bed",
    "SM_Bed_T2": "M_Build_Bed",
    "SM_Bed_T3": "M_Build_Bed",
    "SM_Farm_T1": "M_Build_Farm",
    "SM_Farm_T2": "M_Build_Farm",
    "SM_Farm_T3": "M_Build_Farm",
    "SM_Firepit_T1": "M_Build_Firepit",
    "SM_Firepit_T2": "M_Build_Firepit",
    "SM_Firepit_T3": "M_Build_Firepit",
}


def create_material(asset_name, color):
    unreal.EditorAssetLibrary.make_directory(MATERIAL_DIR)
    asset_path = f"{MATERIAL_DIR}/{asset_name}"
    existing = unreal.EditorAssetLibrary.load_asset(asset_path)
    if existing:
        return existing

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = asset_tools.create_asset(asset_name, MATERIAL_DIR, unreal.Material, unreal.MaterialFactoryNew())

    base_color = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionConstant3Vector,
        -400,
        0,
    )
    base_color.constant = color
    unreal.MaterialEditingLibrary.connect_material_property(
        base_color,
        "",
        unreal.MaterialProperty.MP_BASE_COLOR,
    )

    roughness = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionConstant,
        -400,
        160,
    )
    roughness.r = 0.75
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness,
        "",
        unreal.MaterialProperty.MP_ROUGHNESS,
    )

    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)
    return material


def assign_materials(material_assets):
    for mesh_name, material_name in MESH_MATERIALS.items():
        mesh = unreal.EditorAssetLibrary.load_asset(f"{MESH_DIR}/{mesh_name}")
        material = material_assets.get(material_name)
        if mesh is None:
            unreal.log_warning(f"Missing static mesh: {mesh_name}")
            continue
        if material is None:
            unreal.log_warning(f"Missing material: {material_name}")
            continue

        mesh.set_material(0, material)
        unreal.EditorAssetLibrary.save_asset(f"{MESH_DIR}/{mesh_name}", only_if_is_dirty=False)
        unreal.log(f"Assigned {material_name} to {mesh_name}")


def main():
    material_assets = {}
    for asset_name, color in MATERIALS.items():
        material_assets[asset_name] = create_material(asset_name, color)
    assign_materials(material_assets)
    unreal.EditorAssetLibrary.save_directory(MATERIAL_DIR, only_if_is_dirty=False, recursive=True)
    unreal.EditorAssetLibrary.save_directory(MESH_DIR, only_if_is_dirty=False, recursive=True)
    unreal.log("Building materials created and assigned.")


main()
