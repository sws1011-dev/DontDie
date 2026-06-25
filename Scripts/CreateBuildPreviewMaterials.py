import unreal


MATERIAL_DIR = "/Game/Build/Materials"


MATERIALS = {
    "M_BuildPreview_Valid": unreal.LinearColor(0.05, 1.0, 0.1, 0.45),
    "M_BuildPreview_Invalid": unreal.LinearColor(1.0, 0.05, 0.02, 0.45),
}


def get_or_create_material(asset_name):
    unreal.EditorAssetLibrary.make_directory(MATERIAL_DIR)
    asset_path = f"{MATERIAL_DIR}/{asset_name}"
    material = unreal.EditorAssetLibrary.load_asset(asset_path)
    if material is not None:
        return material

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    return asset_tools.create_asset(asset_name, MATERIAL_DIR, unreal.Material, unreal.MaterialFactoryNew())


def configure_preview_material(material, color):
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property("two_sided", True)

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

    emissive = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionConstant3Vector,
        -400,
        180,
    )
    emissive.constant = unreal.LinearColor(color.r * 0.6, color.g * 0.6, color.b * 0.6, 1.0)
    unreal.MaterialEditingLibrary.connect_material_property(
        emissive,
        "",
        unreal.MaterialProperty.MP_EMISSIVE_COLOR,
    )

    opacity = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionConstant,
        -400,
        360,
    )
    opacity.r = color.a
    unreal.MaterialEditingLibrary.connect_material_property(
        opacity,
        "",
        unreal.MaterialProperty.MP_OPACITY,
    )

    unreal.MaterialEditingLibrary.recompile_material(material)


def main():
    for asset_name, color in MATERIALS.items():
        material = get_or_create_material(asset_name)
        configure_preview_material(material, color)
        unreal.EditorAssetLibrary.save_asset(f"{MATERIAL_DIR}/{asset_name}", only_if_is_dirty=False)
        unreal.log(f"Created build preview material: {asset_name}")


main()
