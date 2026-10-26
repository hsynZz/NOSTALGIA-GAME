"""Repository consistency and preparation-tool tests; not an Unreal compiler."""
import ast
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import struct
import tempfile
import unittest
import wave

ROOT = Path(__file__).resolve().parents[1]


class RepositoryChecks(unittest.TestCase):
    def test_engine_pin_and_project_module(self):
        project = json.loads((ROOT / "NOSTALGIA.uproject").read_text())
        pin = json.loads((ROOT / "Config/EngineVersion.json").read_text())
        self.assertEqual((pin["major"], pin["minor"], pin["patch"]), (5, 8, 0))
        self.assertEqual(project["EngineAssociation"], pin["association"])
        self.assertEqual(project["Modules"], [{"Name": "Nostalgia", "Type": "Runtime", "LoadingPhase": "Default"}])
        plugins = {plugin["Name"]: plugin for plugin in project["Plugins"]}
        for name in ("PythonScriptPlugin", "EditorScriptingUtilities"):
            self.assertEqual(plugins[name]["TargetAllowList"], ["Editor"])

    def test_build_target_discovery(self):
        for target, kind in [("NOSTALGIA", "Game"), ("NOSTALGIAEditor", "Editor")]:
            text = (ROOT / f"Source/{target}.Target.cs").read_text()
            self.assertIn(f"class {target}Target", text)
            self.assertIn(f"TargetType.{kind}", text)
            self.assertIn('ExtraModuleNames.Add("Nostalgia")', text)
        self.assertIn("CppStandardVersion.Cpp20", (ROOT / "Source/Nostalgia/Nostalgia.Build.cs").read_text())

    def test_uht_generated_include_is_last(self):
        checked = 0
        for header in (ROOT / "Source/Nostalgia/Public").glob("*.h"):
            text = header.read_text()
            if re.search(r"U(CLASS|STRUCT|INTERFACE|ENUM)\(", text):
                includes = re.findall(r'^#include "([^"]+)"', text, re.MULTILINE)
                self.assertEqual(includes[-1], f"{header.stem}.generated.h", header.name)
                checked += 1
        self.assertGreaterEqual(checked, 10)

    def test_all_bound_input_names_have_config_mappings(self):
        source = (ROOT / "Source/Nostalgia/Private/NostalgiaCharacter.cpp").read_text()
        config = (ROOT / "Config/DefaultInput.ini").read_text()
        for kind, name in re.findall(r'Bind(Axis|Action)\(TEXT\("([^"]+)"\)', source):
            self.assertIn(f'{kind}Name="{name}"', config)

    def test_map_paths_match_generation_and_packaging(self):
        for relative in ["Config/DefaultEngine.ini", "Config/DefaultGame.ini", "Scripts/prepare_unreal.py", "Scripts/unreal.ps1"]:
            self.assertIn("/Game/Maps/EndlessSummer", (ROOT / relative).read_text())
        self.assertIn("/Script/Nostalgia.NostalgiaGameMode", (ROOT / "Config/DefaultEngine.ini").read_text())

    def test_python_scripts_parse_without_importing_unreal(self):
        for path in (ROOT / "Scripts").rglob("*.py"):
            ast.parse(path.read_text(), filename=str(path))

    def test_lfs_covers_unreal_packages_and_audio(self):
        attributes = (ROOT / ".gitattributes").read_text()
        for extension in ["umap", "uasset", "wav", "fbx", "png"]:
            self.assertIn(f"*.{extension} filter=lfs diff=lfs merge=lfs -text", attributes)

    def test_engine_test_runner_requires_actual_named_results(self):
        source = (ROOT / "Source/Nostalgia/Private/Tests/NostalgiaAutomationTests.cpp").read_text()
        names = re.findall(r'"(Nostalgia\.[^"]+)"', source)
        self.assertEqual(len(names), 4)
        runner = (ROOT / "Scripts/unreal.ps1").read_text()
        for name in names:
            self.assertIn(name, runner)
        self.assertIn('$Match[0].state -ne "Success"', runner)
        self.assertIn("yyyyMMdd_HHmmss_fff", runner)

    def test_placeholder_audio_is_mono_bounded_nonempty_and_repeatable(self):
        spec = importlib.util.spec_from_file_location("audio_generator", ROOT / "Scripts/make_placeholder_audio.py")
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        with tempfile.TemporaryDirectory() as directory:
            files = module.generate(Path(directory))
            before = [hashlib.sha256(p.read_bytes()).hexdigest() for p in files]
            self.assertEqual(len(files), 2)
            for path, duration in zip(files, (6, 2)):
                with wave.open(str(path)) as handle:
                    self.assertEqual(handle.getnchannels(), 1)
                    self.assertEqual(handle.getsampwidth(), 2)
                    self.assertEqual(handle.getnframes(), 24000 * duration)
                    values = struct.unpack(f"<{handle.getnframes()}h", handle.readframes(handle.getnframes()))
                    self.assertGreater(max(map(abs, values)), 1000)
                    self.assertLess(max(map(abs, values)), 32767)
            module.generate(Path(directory))
            self.assertEqual(before, [hashlib.sha256(p.read_bytes()).hexdigest() for p in files])


if __name__ == "__main__":
    unittest.main()
