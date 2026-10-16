"""Compare actual JUCE captures with the three supplied references (Pillow).

This does not synthesize or substitute GUI screenshots. Missing captures fail.
"""
import argparse
import json
from pathlib import Path
from PIL import Image, ImageChops, ImageDraw, ImageStat


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--references", type=Path, required=True)
    parser.add_argument("--captures", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    files = {
        "4SX": "Screenshot 2026-10-16 142938.png",
        "3DR": "Screenshot 2026-10-16 142946.png",
        "40G": "Screenshot 2026-10-16 142958.png",
    }
    # Validate all inputs before creating any comparison artefacts.
    for skin, filename in files.items():
        for file in (args.references / filename, args.captures / f"{skin}-1x.png"):
            if not file.is_file():
                raise FileNotFoundError(f"Actual reference/capture missing: {file}")
    args.output.mkdir(parents=True, exist_ok=True)
    results = {}
    for skin, filename in files.items():
        with Image.open(args.references / filename) as source:
            reference = source.convert("RGB").crop((0, 22, source.width, source.height))
        with Image.open(args.captures / f"{skin}-1x.png") as source:
            actual = source.convert("RGB")
        if actual.size != reference.size:
            raise ValueError(f"{skin}: capture {actual.size} != reference {reference.size}")
        difference = ImageChops.difference(reference, actual)
        page = Image.new("RGB", (reference.width * 3, reference.height + 28), "#202326")
        draw = ImageDraw.Draw(page)
        for index, (label, image) in enumerate((("Reference (host title removed)", reference),
                                                ("Actual JUCE capture", actual),
                                                ("Absolute pixel difference", difference))):
            draw.text((index * reference.width + 8, 8), label, fill="white")
            page.paste(image, (index * reference.width, 28))
        page.save(args.output / f"{skin}-comparison.png")
        results[skin] = {"size": list(actual.size), "mean_absolute_RGB_error": ImageStat.Stat(difference).mean,
                         "note": "Diagnostic only: typography, signature and mode switch intentionally differ; inspect visually."}
    (args.output / "comparison.json").write_text(json.dumps(results, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()
