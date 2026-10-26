"""Run INSIDE Unreal Editor 5.8.0 after compiling the native editor target.

Creates genuine packages through Unreal APIs. Existing authored packages are
preserved; the script never fabricates .umap/.uasset binary files outside UE.
"""
import json
import sys
from pathlib import Path

import unreal

ROOT = Path(unreal.Paths.project_dir()).resolve()
sys.path.insert(0, str(ROOT / "Scripts"))
from make_placeholder_audio import generate  # noqa: E402


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def main():
    pin = json.loads((ROOT / "Config/EngineVersion.json").read_text(encoding="utf-8"))
    version = unreal.SystemLibrary.get_engine_version()
    prefix = f"{pin['major']}.{pin['minor']}.{pin['patch']}"
    require(version.split("-")[0].split("+")[0] == prefix, f"Expected UE {prefix}; found {version}")
    library = unreal.EditorAssetLibrary
    assets = unreal.AssetToolsHelpers.get_asset_tools()
    for directory in ("/Game/Maps", "/Game/Audio", "/Game/Data", "/Game/Materials"):
        library.make_directory(directory)

    for wav in generate(ROOT / "Artifacts/Audio"):
        target = f"/Game/Audio/{wav.stem}"
        if not library.does_asset_exist(target):
            task = unreal.AssetImportTask()
            task.set_editor_property("filename", str(wav))
            task.set_editor_property("destination_path", "/Game/Audio")
            task.set_editor_property("destination_name", wav.stem)
            task.set_editor_property("automated", True)
            task.set_editor_property("replace_existing", False)
            task.set_editor_property("save", True)
            assets.import_asset_tasks([task])
        require(isinstance(library.load_asset(target), unreal.SoundWave), f"Missing/wrong audio asset: {target}")

    material_path = "/Game/Materials/M_Silhouette"
    if not library.does_asset_exist(material_path):
        material = assets.create_asset("M_Silhouette", "/Game/Materials", unreal.Material, unreal.MaterialFactoryNew())
        require(material is not None, "Could not create silhouette material")
        color = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant3Vector, -200, 0)
        color.set_editor_property("constant", unreal.LinearColor(0.002, 0.002, 0.003, 1.0))
        require(unreal.MaterialEditingLibrary.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR), "Material connection failed")
        unreal.MaterialEditingLibrary.recompile_material(material)
        require(library.save_asset(material_path), "Material save failed")
    require(isinstance(library.load_asset(material_path), unreal.Material), "Wrong silhouette material class")

    catalog_path = "/Game/Data/DA_EndlessSummerEvents"
    if not library.does_asset_exist(catalog_path):
        factory = unreal.DataAssetFactory()
        factory.set_editor_property("data_asset_class", unreal.NostalgiaEventCatalog.static_class())
        catalog = assets.create_asset("DA_EndlessSummerEvents", "/Game/Data", unreal.NostalgiaEventCatalog, factory)
        require(catalog is not None, "Event catalog creation failed")
        require(library.save_asset(catalog_path), "Catalog save failed")
    require(isinstance(library.load_asset(catalog_path), unreal.NostalgiaEventCatalog), "Wrong event catalog class")

    level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_system = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    map_path = "/Game/Maps/EndlessSummer"
    if not library.does_asset_exist(map_path):
        require(level.new_level(map_path), "Map creation failed")
        blockout = actor_system.spawn_actor_from_class(unreal.NostalgiaBlockout, unreal.Vector(0, 0, 0))
        require(blockout is not None, "Native blockout class not loaded; build NOSTALGIAEditor first")
        blockout.set_actor_label("EndlessSummer_RuntimeBlockout")
        start = actor_system.spawn_actor_from_class(unreal.PlayerStart, unreal.NostalgiaGameMode.prototype_spawn_location())
        require(start is not None, "PlayerStart creation failed")
        require(level.save_current_level(), "Map save failed")
    else:
        require(level.load_level(map_path), "Existing map could not be loaded")
    actors = actor_system.get_all_level_actors()
    require(sum(isinstance(actor, unreal.NostalgiaBlockout) for actor in actors) == 1, "Map needs exactly one native blockout actor")
    require(any(isinstance(actor, unreal.PlayerStart) for actor in actors), "Map needs PlayerStart")
    report = {"engine": version, "map": map_path, "prepared": True, "runtime_tested": False,
              "assets": [material_path, catalog_path, "/Game/Audio/A_GardenPlaceholder", "/Game/Audio/A_KnockPlaceholder"]}
    output = ROOT / "Artifacts/Unreal"
    output.mkdir(parents=True, exist_ok=True)
    (output / "prepare.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    unreal.log("NOSTALGIA_PREPARE_SUCCESS: genuine Unreal packages saved; runtime acceptance still pending")


if __name__ == "__main__":
    main()
