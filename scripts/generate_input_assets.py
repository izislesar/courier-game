"""Generate Enhanced Input assets for Courier 404 (headless editor python).

Run via:
  UnrealEditor-Cmd Courier404.uproject -run=pythonscript -script=scripts/generate_input_assets.py
"""
import unreal

CONTENT_INPUT = "/Game/Input"


def ensure_action(name):
    path = f"{CONTENT_INPUT}/{name}"
    existing = unreal.EditorAssetLibrary.does_asset_exist(path)
    if existing:
        return unreal.load_asset(path)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, CONTENT_INPUT, unreal.InputAction, None)
    unreal.log(f"[Courier404] created {path}")
    return asset


def main():
    unreal.EditorAssetLibrary.make_directory(CONTENT_INPUT)

    move = ensure_action("IA_CourierMove")
    look = ensure_action("IA_CourierLook")
    interact = ensure_action("IA_CourierInteract")

    for action in (move, look, interact):
        action.value_type = unreal.InputActionValueType.AXIS2D if action in (move, look) else unreal.InputActionValueType.BOOLEAN

    imc_path = f"{CONTENT_INPUT}/IMC_CourierDefault"
    if not unreal.EditorAssetLibrary.does_asset_exist(imc_path):
        imc = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            "IMC_CourierDefault", CONTENT_INPUT, unreal.InputMappingContext, None)
        unreal.log("[Courier404] created IMC")
    else:
        imc = unreal.load_asset(imc_path)
        imc.unmap_all()

    key_map = {
        move: ["W", "S", "A", "D"],
        look: ["Mouse2D"],
        interact: ["E"],
    }
    for action, keys in key_map.items():
        for key_name in keys:
            imc.map_key(action, unreal.Key(unreal.Name(key_name)))

    unreal.log("[Courier404] input assets ready")


main()
