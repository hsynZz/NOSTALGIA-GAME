"""Execute inside UE 5.8.0: import real GLBs and build the authored quality scene.

No generated Unreal packages are checked in until an actual Editor has saved them.
Each source revision gets a separate namespace; authored prior maps are preserved.
"""
import hashlib
import json
from pathlib import Path

import unreal

ROOT = Path(unreal.Paths.project_dir()).resolve()
SOURCE = ROOT / "SourceArt/Quality02/scene.manifest.json"


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def set_property(obj, name, value):
    """Required settings fail loudly if this engine API differs; never silently skip."""
    obj.set_editor_property(name, value)


def main():
    version = unreal.SystemLibrary.get_engine_version()
    require(version.split("-")[0].split("+")[0] == "5.8.0", f"Expected UE 5.8.0, found {version}")
    data = json.loads(SOURCE.read_text(encoding="utf-8"))
    digest = hashlib.sha256(SOURCE.read_bytes()).hexdigest()
    namespace = "/Game/Quality02/B_" + digest[:12]
    map_path = namespace + "/Maps/EndlessSummer_Quality02"
    report = {"engine": version, "source_sha256": digest, "map": map_path,
              "prepared": False, "runtime_tested": False, "visually_accepted": False,
              "audio_recording_present": False, "imports": [], "warnings": []}
    output = ROOT / "Artifacts/Quality02"
    output.mkdir(parents=True, exist_ok=True)
    library = unreal.EditorAssetLibrary
    assets = unreal.AssetToolsHelpers.get_asset_tools()
    level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    try:
        # Validate every byte before importing anything or changing the current level.
        for item in data["assets"]:
            path = ROOT / item["path"]
            require(path.is_file(), f"Missing source: {path}")
            require(hashlib.sha256(path.read_bytes()).hexdigest() == item["sha256"], f"Source hash mismatch: {path}")
        # Unattended automation is safe only in a clean editor (no unsaved user work).
        dirty = list(unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages())
        dirty += list(unreal.EditorLoadingAndSavingUtils.get_dirty_content_packages())
        require(not dirty, "Unsaved Editor packages: save your work before running this importer.")
        if library.does_asset_exist(map_path):
            require(level.load_level(map_path), "Could not load preserved quality map")
            require(any(actor.actor_has_tag("NostalgiaQuality02Ready") for actor in actors.get_all_level_actors()),
                    "Existing map has no completed-import marker. Previous import was incomplete; inspect unreal-import.json. It has not been marked prepared.")
            directors = [actor for actor in actors.get_all_level_actors() if isinstance(actor, unreal.NostalgiaSliceDirector)]
            require(len(directors) == 1, "Preserved map must contain exactly one quality Director")
            report["audio_recording_present"] = bool(directors[0].get_editor_property("audio").get_editor_property("garden_cue"))
            report["prepared"] = True
            report["preserved_existing_map"] = True
            unreal.log("Existing quality map preserved: " + map_path)
            return

        meshes = {}
        for item in data["assets"]:
            destination = namespace + "/Meshes/" + item["name"]
            # Folder-per-mesh keeps texture/material import names from colliding.
            library.make_directory(destination)
            mesh_path = destination + "/" + item["name"]
            if library.does_asset_exist(mesh_path):
                mesh = library.load_asset(mesh_path)
            else:
                task = unreal.AssetImportTask()
                for key, value in {"filename": str(ROOT / item["path"]),
                                   "destination_path": destination, "destination_name": item["name"],
                                   "automated": True, "replace_existing": False, "save": True}.items():
                    set_property(task, key, value)
                assets.import_asset_tasks([task])
                imported = [o for o in task.get_objects() if isinstance(o, unreal.StaticMesh)]
                require(len(imported) == 1, f"Expected one combined StaticMesh from {item['name']}; got {len(imported)}")
                mesh = imported[0]
                if mesh.get_path_name().split(".")[0] != mesh_path:
                    require(library.rename_asset(mesh.get_path_name(), mesh_path), "Could not stabilize imported mesh name")
            require(isinstance(mesh, unreal.StaticMesh), f"Wrong imported asset class: {mesh_path}")
            # Catch glTF axis/unit import mistakes before a player is spawned inside bad geometry.
            bounds = mesh.get_bounding_box()
            extent = bounds.max - bounds.min
            wanted = [(b-a)*100 for a,b in item["bounds_m"]]
            actual = [extent.x, extent.y, extent.z]
            require(all(abs(a-b) <= max(2, .02*b) for a,b in zip(actual,wanted)),
                    f"Import axis/scale mismatch {item['name']}: {actual} cm, expected {wanted}; inspect glTF pipeline")
            require(len(mesh.get_editor_property("static_materials")) > 0, "Imported mesh has no material slots")
            if item["collision"] == "complex_static":
                body = mesh.get_editor_property("body_setup")
                require(body is not None, "Mesh has no BodySetup")
                set_property(body, "collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
            require(library.save_loaded_asset(mesh), "Could not save imported mesh")
            meshes[item["name"]] = mesh
            report["imports"].append({"source": item["path"], "mesh": mesh.get_path_name(), "extent_cm": actual})

        library.make_directory(namespace + "/Maps")
        require(level.new_level(map_path), "Quality map creation failed")
        for item in data["assets"]:
            if item["dynamic"]:
                continue
            actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector())
            require(actor is not None, "Static mesh actor creation failed")
            actor.set_actor_label(item["name"])
            component = actor.get_component_by_class(unreal.StaticMeshComponent)
            require(component.set_static_mesh(meshes[item["name"]]), "Could not assign static mesh")
            component.set_mobility(unreal.ComponentMobility.STATIC)
            component.set_collision_profile_name("BlockAll" if item["collision"] == "complex_static" else "NoCollision")
            component.set_cast_shadow(item["cast_shadow"])

        for index, location in enumerate(data["doors_cm"]):
            door = actors.spawn_actor_from_class(unreal.NostalgiaSliceDoor, unreal.Vector(*location))
            require(door is not None, "Build the native NOSTALGIAEditor target before importing")
            door.set_actor_label(f"Door_{index+1}_Authored")
            require(door.get_editor_property("leaf").set_static_mesh(meshes["SM_DoorLeaf"]), "Door visual assignment failed")

        start = actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(*data["spawn_cm"]), unreal.Rotator(0,-145,0))
        require(start is not None, "PlayerStart missing")
        director = actors.spawn_actor_from_class(unreal.NostalgiaSliceDirector, unreal.Vector())
        require(director is not None, "Quality Director class missing")
        audio = director.get_editor_property("audio")
        audio.set_editor_property("garden_origin", unreal.Vector(*data["garden_origin_cm"]))
        # Only explicitly approved/imported human recording is accepted; no synthetic substitute.
        audio_path = "/Game/Quality02/ApprovedAudio/A_GardenLaughter"
        if library.does_asset_exist(audio_path):
            recording = library.load_asset(audio_path)
            require(isinstance(recording, unreal.SoundWave), "Garden recording must be a SoundWave")
            require(recording.get_editor_property("num_channels") == 1, "Garden recording must be mono for point spatialization")
            require(not recording.get_editor_property("looping"), "Garden recording must not loop")
            require(0 < recording.get_editor_property("duration") <= 10, "Garden recording must be a short one-shot (at most 10 seconds)")
            audio.set_editor_property("garden_cue", recording)
            report["audio_recording_present"] = True
        else:
            report["warnings"].append("No approved garden laughter recording. The spatial one-shot is wired but silent.")

        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        world.get_world_settings().set_editor_property("default_game_mode", unreal.NostalgiaSliceGameMode.static_class())
        sun = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0,0,1000), unreal.Rotator(-50,90,0))
        light = sun.get_component_by_class(unreal.DirectionalLightComponent)
        light.set_mobility(unreal.ComponentMobility.MOVABLE)
        light.set_intensity(85000)
        set_property(light, "atmosphere_sun_light", True)
        set_property(light, "light_source_angle", .8)
        set_property(light, "use_temperature", True)
        set_property(light, "temperature", 6200)
        set_property(light, "cast_cloud_shadows", True)
        actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector())
        sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector())
        skylight = sky.get_component_by_class(unreal.SkyLightComponent)
        skylight.set_mobility(unreal.ComponentMobility.MOVABLE)
        set_property(skylight, "real_time_capture", True)
        skylight.set_intensity(1)
        cloud_material = library.load_asset("/Engine/EngineSky/VolumetricClouds/m_SimpleVolumetricCloud_Inst")
        require(cloud_material is not None, "Engine cloud material missing; install the standard Engine Content before completing scene preparation")
        clouds = actors.spawn_actor_from_class(unreal.VolumetricCloud, unreal.Vector())
        cloud = clouds.get_component_by_class(unreal.VolumetricCloudComponent)
        set_property(cloud, "material", cloud_material)
        set_property(cloud, "layer_bottom_altitude", 2)
        set_property(cloud, "layer_height", 1.6)

        volume = actors.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector())
        set_property(volume, "unbound", True)
        settings = volume.get_editor_property("settings")
        for field, value in {"auto_exposure_min_brightness": 8.0, "auto_exposure_max_brightness": 14.0,
                             "auto_exposure_speed_up": 1.0, "auto_exposure_speed_down": .6,
                             "auto_exposure_bias": .5, "bloom_intensity": .15,
                             "vignette_intensity": .12, "scene_fringe_intensity": 0.0,
                             "motion_blur_amount": 0.0, "film_grain_intensity": 0.0}.items():
            set_property(settings, "override_" + field, True)
            set_property(settings, field, value)
        set_property(volume, "settings", settings)
        director.set_editor_property("tags", [unreal.Name("NostalgiaQuality02Ready")])
        require(level.save_current_level(), "Map could not be saved")
        report["prepared"] = True
        report["warnings"].append("glTF sheen, glass transmission, foliage two-sided shading and normal maps require visual comparison in UE.")
        unreal.log("NOSTALGIA_QUALITY02_PREPARED " + map_path + " — PIE/visual acceptance still required")
    except Exception as error:
        report["error"] = str(error)
        raise
    finally:
        (output/"unreal-import.json").write_text(json.dumps(report, indent=2)+"\n", encoding="utf-8")


if __name__ == "__main__":
    main()
