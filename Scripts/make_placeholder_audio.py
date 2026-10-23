"""Generate original technical test tones. These are NOT recordings of children.

No downloaded samples or third-party libraries. Generated WAVs are imported into
Unreal by prepare_unreal.py, not passed off as production sound design.
"""
import math
import struct
import wave
from pathlib import Path

RATE = 24000


def generate(output: Path) -> list[Path]:
    output.mkdir(parents=True, exist_ok=True)
    written = []
    for name, duration in [("A_GardenPlaceholder", 6.0), ("A_KnockPlaceholder", 2.0)]:
        samples = []
        for index in range(int(RATE * duration)):
            time = index / RATE
            signal = 0.0
            if name == "A_GardenPlaceholder":
                for start, frequency in [(0.4, 420), (1.1, 540), (1.8, 460), (3.2, 500), (4.0, 380)]:
                    phase = time - start
                    if 0 <= phase <= 0.3:
                        envelope = math.sin(math.pi * phase / 0.3) ** 2
                        signal += 0.3 * envelope * math.sin(2 * math.pi * (frequency * phase + 180 * phase * phase))
            else:
                for start in (0.1, 0.65, 1.2):
                    phase = time - start
                    if 0 <= phase <= 0.12:
                        signal += 0.6 * math.exp(-45 * phase) * math.sin(2 * math.pi * 135 * phase)
            samples.append(struct.pack("<h", round(max(-0.95, min(0.95, signal)) * 32767)))
        path = output / f"{name}.wav"
        with wave.open(str(path), "wb") as handle:
            handle.setnchannels(1)
            handle.setsampwidth(2)
            handle.setframerate(RATE)
            handle.writeframes(b"".join(samples))
        written.append(path)
    return written


if __name__ == "__main__":
    for result in generate(Path(__file__).resolve().parents[1] / "Artifacts" / "Audio"):
        print(result)
