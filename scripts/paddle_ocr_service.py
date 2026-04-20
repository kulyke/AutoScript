import base64
import json
import os
import re
import sys

import cv2
import numpy as np


def build_ocr():
    from paddleocr._models import TextRecognition

    return TextRecognition(
        model_name="PP-OCRv5_mobile_rec",
    )


def normalize_digits(text: str) -> str:
    return "".join(re.findall(r"\d+", text or ""))


def collect_candidate_texts(value):
    texts = []

    if isinstance(value, str):
        texts.append(value)
        return texts

    if isinstance(value, dict):
        handled_keys = set()
        for key in ("rec_text", "text", "label"):
            candidate = value.get(key)
            if isinstance(candidate, str):
                texts.append(candidate)
                handled_keys.add(key)
        for key, nested_value in value.items():
            if key in handled_keys:
                continue
            texts.extend(collect_candidate_texts(nested_value))
        return texts

    if isinstance(value, (list, tuple)):
        for item in value:
            texts.extend(collect_candidate_texts(item))
        return texts

    return texts


def iter_variants(image: np.ndarray):
    yield image

    upscaled = cv2.resize(image, None, fx=3.0, fy=3.0, interpolation=cv2.INTER_CUBIC)
    yield upscaled

    gray = cv2.cvtColor(upscaled, cv2.COLOR_BGR2GRAY)
    yield cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)

    _, binary = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    yield cv2.cvtColor(binary, cv2.COLOR_GRAY2BGR)


def recognize_digits(ocr, image: np.ndarray) -> str:
    for variant in iter_variants(image):
        result = ocr.predict(variant)
        if not result:
            continue

        digits = normalize_digits("".join(collect_candidate_texts(result)))
        if digits:
            return digits

    return ""


def write_json(payload):
    sys.stdout.write(json.dumps(payload, ensure_ascii=True) + "\n")
    sys.stdout.flush()


def main():
    os.environ.setdefault("FLAGS_allocator_strategy", "auto_growth")
    os.environ.setdefault("PADDLE_PDX_DISABLE_MODEL_SOURCE_CHECK", "True")

    try:
        ocr = build_ocr()
    except Exception as exc:
        write_json({"ready": False, "error": f"init_failed: {exc}"})
        return 1

    write_json({"ready": True})

    for raw_line in sys.stdin:
        line = raw_line.strip()
        if not line:
            continue

        try:
            payload = json.loads(line)
        except json.JSONDecodeError as exc:
            write_json({"ok": False, "error": f"invalid_json: {exc}"})
            continue

        if payload.get("command") == "shutdown":
            write_json({"ok": True, "shutdown": True})
            return 0

        image_base64 = payload.get("image_base64", "")
        if not image_base64:
            write_json({"ok": False, "error": "missing_image"})
            continue

        try:
            image_bytes = base64.b64decode(image_base64)
            np_buffer = np.frombuffer(image_bytes, dtype=np.uint8)
            image = cv2.imdecode(np_buffer, cv2.IMREAD_COLOR)
            if image is None or image.size == 0:
                raise ValueError("decode_failed")
            digits = recognize_digits(ocr, image)
        except Exception as exc:
            write_json({"ok": False, "error": f"ocr_failed: {exc}"})
            continue

        if digits:
            write_json({"ok": True, "text": digits})
        else:
            write_json({"ok": False, "error": "no_digits"})

    return 0


if __name__ == "__main__":
    raise SystemExit(main())