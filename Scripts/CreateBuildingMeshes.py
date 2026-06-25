import os

import unreal


PROJECT_CONTENT_DIR = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_content_dir())
SOURCE_DIR = os.path.join(PROJECT_CONTENT_DIR, "Build", "MeshSources")
DEST_PATH = "/Game/Build/Meshes"


def box(name, center, size):
    cx, cy, cz = center
    sx, sy, sz = size[0] * 0.5, size[1] * 0.5, size[2] * 0.5
    verts = [
        (cx - sx, cy - sy, cz - sz),
        (cx + sx, cy - sy, cz - sz),
        (cx + sx, cy + sy, cz - sz),
        (cx - sx, cy + sy, cz - sz),
        (cx - sx, cy - sy, cz + sz),
        (cx + sx, cy - sy, cz + sz),
        (cx + sx, cy + sy, cz + sz),
        (cx - sx, cy + sy, cz + sz),
    ]
    faces = [
        (1, 4, 3, 2),
        (5, 6, 7, 8),
        (1, 2, 6, 5),
        (2, 3, 7, 6),
        (3, 4, 8, 7),
        (4, 1, 5, 8),
    ]
    return {"name": name, "verts": verts, "faces": faces}


def write_obj(asset_name, boxes):
    os.makedirs(SOURCE_DIR, exist_ok=True)
    path = os.path.join(SOURCE_DIR, f"{asset_name}.obj")
    vertex_offset = 0
    with open(path, "w", encoding="utf-8") as f:
        f.write(f"o {asset_name}\n")
        for part in boxes:
            f.write(f"g {part['name']}\n")
            for x, y, z in part["verts"]:
                f.write(f"v {x:.4f} {y:.4f} {z:.4f}\n")
            for face in part["faces"]:
                indices = [str(index + vertex_offset) for index in face]
                f.write("f " + " ".join(indices) + "\n")
            vertex_offset += len(part["verts"])
    return path


def workbench(size, tier):
    x, y, z = size
    bottom = -z * 0.5
    top_thickness = z * 0.14
    top_z = z * 0.35
    leg_h = z * 0.7
    leg = (x * 0.08, y * 0.08, leg_h)
    lx, ly = x * 0.38, y * 0.38
    parts = [
        box("top", (0, 0, top_z), (x, y, top_thickness)),
        box("leg_a", (lx, ly, bottom + leg_h * 0.5), leg),
        box("leg_b", (lx, -ly, bottom + leg_h * 0.5), leg),
        box("leg_c", (-lx, ly, bottom + leg_h * 0.5), leg),
        box("leg_d", (-lx, -ly, bottom + leg_h * 0.5), leg),
    ]
    if tier >= 2:
        parts += [
            box("back_board", (0, -y * 0.42, top_z + z * 0.22), (x * 0.8, y * 0.08, z * 0.42)),
            box("lower_shelf", (0, y * 0.2, bottom + z * 0.2), (x * 0.7, y * 0.12, z * 0.08)),
        ]
    if tier >= 3:
        parts += [
            box("side_extension", (x * 0.55, 0, top_z - z * 0.05), (x * 0.22, y * 0.7, z * 0.1)),
            box("tool_block", (-x * 0.18, -y * 0.44, top_z + z * 0.5), (x * 0.18, y * 0.08, z * 0.18)),
        ]
    return parts


def storage(size, tier):
    x, y, z = size
    parts = [
        box("crate_body", (0, 0, 0), (x, y, z * 0.75)),
        box("lid", (0, 0, z * 0.42), (x * 1.05, y * 1.05, z * 0.15)),
    ]
    if tier >= 2:
        parts += [
            box("strap_x_a", (x * 0.35, 0, 0), (x * 0.08, y * 1.1, z * 0.9)),
            box("strap_x_b", (-x * 0.35, 0, 0), (x * 0.08, y * 1.1, z * 0.9)),
        ]
    if tier >= 3:
        parts += [
            box("strap_y_a", (0, y * 0.52, 0), (x * 1.1, y * 0.08, z * 0.9)),
            box("strap_y_b", (0, -y * 0.52, 0), (x * 1.1, y * 0.08, z * 0.9)),
            box("front_lock", (0, y * 0.55, z * 0.18), (x * 0.25, y * 0.08, z * 0.16)),
        ]
    return parts


def bed(size, tier):
    x, y, z = size
    bottom = -z * 0.5
    parts = [
        box("frame", (0, 0, bottom + z * 0.18), (x, y, z * 0.2)),
        box("mattress", (0, 0, bottom + z * 0.36), (x * 0.92, y * 0.88, z * 0.16)),
        box("pillow", (x * 0.32, 0, bottom + z * 0.48), (x * 0.25, y * 0.75, z * 0.12)),
    ]
    if tier >= 2:
        parts += [
            box("headboard", (-x * 0.47, 0, bottom + z * 0.48), (x * 0.08, y, z * 0.55)),
            box("footboard", (x * 0.47, 0, bottom + z * 0.48), (x * 0.08, y, z * 0.35)),
        ]
    if tier >= 3:
        parts += [
            box("rail_a", (0, y * 0.48, bottom + z * 0.58), (x, y * 0.08, z * 0.28)),
            box("rail_b", (0, -y * 0.48, bottom + z * 0.58), (x, y * 0.08, z * 0.28)),
        ]
    return parts


def farm(size, tier):
    x, y, z = size
    bottom = -z * 0.5
    parts = [
        box("soil", (0, 0, bottom + z * 0.08), (x, y, z * 0.16)),
        box("furrow_a", (0, -y * 0.22, bottom + z * 0.2), (x * 0.9, y * 0.08, z * 0.08)),
        box("furrow_b", (0, y * 0.22, bottom + z * 0.2), (x * 0.9, y * 0.08, z * 0.08)),
    ]
    if tier >= 2:
        for idx, px in enumerate((-0.25, 0.25)):
            for idy, py in enumerate((-0.22, 0.22)):
                parts.append(box(f"crop_{idx}_{idy}", (x * px, y * py, bottom + z * 0.38), (x * 0.08, y * 0.08, z * 0.28)))
    if tier >= 3:
        parts += [
            box("fence_right", (x * 0.48, 0, bottom + z * 0.28), (x * 0.06, y, z * 0.28)),
            box("fence_left", (-x * 0.48, 0, bottom + z * 0.28), (x * 0.06, y, z * 0.28)),
            box("fence_top", (0, y * 0.48, bottom + z * 0.28), (x, y * 0.06, z * 0.28)),
            box("fence_bottom", (0, -y * 0.48, bottom + z * 0.28), (x, y * 0.06, z * 0.28)),
        ]
    return parts


def firepit(size, tier):
    x, y, z = size
    bottom = -z * 0.5
    parts = [
        box("fire_core", (0, 0, 0), (x * 0.75, y * 0.75, z * 0.35)),
        box("stone_a", (x * 0.34, 0, bottom + z * 0.35), (x * 0.12, y * 0.8, z * 0.28)),
        box("stone_b", (-x * 0.34, 0, bottom + z * 0.35), (x * 0.12, y * 0.8, z * 0.28)),
        box("stone_c", (0, y * 0.34, bottom + z * 0.35), (x * 0.8, y * 0.12, z * 0.28)),
        box("stone_d", (0, -y * 0.34, bottom + z * 0.35), (x * 0.8, y * 0.12, z * 0.28)),
    ]
    if tier >= 2:
        parts += [
            box("grill_plate", (0, 0, bottom + z * 0.62), (x * 0.55, y * 0.55, z * 0.18)),
            box("chimney", (0, 0, bottom + z * 0.82), (x * 0.3, y * 0.3, z * 0.25)),
        ]
    if tier >= 3:
        parts += [
            box("rim_a", (0, y * 0.42, bottom + z * 0.75), (x * 0.95, y * 0.08, z * 0.12)),
            box("rim_b", (0, -y * 0.42, bottom + z * 0.75), (x * 0.95, y * 0.08, z * 0.12)),
            box("rim_c", (x * 0.42, 0, bottom + z * 0.75), (x * 0.08, y * 0.95, z * 0.12)),
            box("rim_d", (-x * 0.42, 0, bottom + z * 0.75), (x * 0.08, y * 0.95, z * 0.12)),
        ]
    return parts


DEFINITIONS = [
    ("SM_Workbench_T1", workbench, (400, 400, 120), 1),
    ("SM_Workbench_T2", workbench, (440, 420, 150), 2),
    ("SM_Workbench_T3", workbench, (500, 460, 180), 3),
    ("SM_Storage_T1", storage, (260, 260, 180), 1),
    ("SM_Storage_T2", storage, (320, 300, 220), 2),
    ("SM_Storage_T3", storage, (380, 340, 260), 3),
    ("SM_Bed_T1", bed, (420, 220, 100), 1),
    ("SM_Bed_T2", bed, (460, 240, 130), 2),
    ("SM_Bed_T3", bed, (520, 260, 160), 3),
    ("SM_Farm_T1", farm, (500, 500, 80), 1),
    ("SM_Farm_T2", farm, (540, 540, 110), 2),
    ("SM_Farm_T3", farm, (600, 600, 140), 3),
    ("SM_Firepit_T1", firepit, (260, 260, 140), 1),
    ("SM_Firepit_T2", firepit, (320, 320, 220), 2),
    ("SM_Firepit_T3", firepit, (380, 380, 320), 3),
]


def import_meshes(obj_paths):
    unreal.EditorAssetLibrary.make_directory(DEST_PATH)
    tasks = []
    for asset_name, obj_path in obj_paths:
        task = unreal.AssetImportTask()
        task.filename = obj_path
        task.destination_path = DEST_PATH
        task.destination_name = asset_name
        task.automated = True
        task.replace_existing = True
        task.save = True

        options = unreal.FbxImportUI()
        options.import_mesh = True
        options.import_as_skeletal = False
        options.static_mesh_import_data.combine_meshes = True
        options.static_mesh_import_data.generate_lightmap_u_vs = True
        options.static_mesh_import_data.auto_generate_collision = True
        task.options = options
        tasks.append(task)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)


def main():
    target_name = os.environ.get("BUILD_MESH_NAME")
    obj_paths = []
    for asset_name, builder, size, tier in DEFINITIONS:
        if target_name and asset_name != target_name:
            continue
        obj_path = write_obj(asset_name, builder(size, tier))
        obj_paths.append((asset_name, obj_path))
    import_meshes(obj_paths)
    unreal.EditorAssetLibrary.save_directory(DEST_PATH, only_if_is_dirty=False, recursive=True)
    unreal.log(f"Created {len(obj_paths)} building static meshes in {DEST_PATH}")


main()
